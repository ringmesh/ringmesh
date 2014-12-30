/*[
* Association Scientifique pour la Geologie et ses Applications (ASGA)
* Copyright (c) 1993-2013 ASGA. All Rights Reserved.
*
* This program is a Trade Secret of the ASGA and it is not to be:
* - reproduced, published, or disclosed to other,
* - distributed or displayed,
* - used for purposes or on Sites other than described
*   in the GOCAD Advancement Agreement,
* without the prior written authorization of the ASGA. Licencee
* agrees to attach or embed this Notice on all copies of the program,
* including partial copies or modified versions thereof.
]*/

#include <grgmesh/tetra_gen.h>
#include <grgmesh/boundary_model_element.h>

#include <geogram/mesh/mesh_private.h>

#include <iomanip>
#include <stack>

namespace GRGMesh {

    TetraGen_var TetraGen::instantiate(
        const TetraMethod& method,
        GEO::Mesh& tetmesh,
        const BoundaryModelElement* region,
        bool add_steiner_points,
        const std::vector< vec3 >& internal_vertices,
        const std::vector< std::vector< Edge > >& well_vertices,
        GEO::Mesh* background )
    {
        switch( method ) {
            case TetGen:
                return new TetraGen_TetGen( tetmesh, region, add_steiner_points,
                    internal_vertices, well_vertices, background ) ;
#ifdef USE_MG_TETRA
            case MG_Tetra:
                return new TetraGen_MG_Tetra( tetmesh, region, add_steiner_points,
                    internal_vertices, well_vertices, background ) ;
#endif
            default:
                return nil ;
        }
    }

    TetraGen::TetraGen(
        GEO::Mesh& tetmesh,
        const BoundaryModelElement* region,
        const std::vector< vec3 >& internal_vertices,
        const std::vector< std::vector< Edge > >& well_edges,
        GEO::Mesh* background )
        :
            tetmesh_( tetmesh ),
            internal_points_( internal_vertices ),
            resolution_( 0 ),
            background_( background ),
            tetmesh_builder_( &tetmesh ),
            region_( region )
    {
        tetmesh_builder_.begin_mesh() ;
        if( !well_edges.empty() ) {
            index_t nb_well_edges = 0 ;
            for( index_t w = 0; w < well_edges.size(); w++ ) {
                nb_well_edges += well_edges[w].size() ;
            }
            well_edges_.reserve( nb_well_edges ) ;
            well_ptr_.reserve( nb_well_edges + 1 ) ;
            well_ptr_.push_back( 0 ) ;
            for( index_t w = 0; w < well_edges.size(); w++ ) {
                for( index_t e = 0; e < well_edges[w].size(); e++ ) {
                    well_edges_.push_back( well_edges[w][e] ) ;
                }
                well_ptr_.push_back( well_ptr_[w] + well_edges[w].size() ) ;
            }
        }

        index_t first_index = 1 ;
        index_t nb_surfaces = region->nb_boundaries() ;
        std::vector< const BoundaryModelElement* > unique_surfaces ;
        unique_surfaces.reserve( nb_surfaces ) ;
        surface_id_.reserve( nb_surfaces ) ;
        surface_ptr_.reserve( nb_surfaces + 1 ) ;
        surface_ptr_.push_back( 0 ) ;
        signed_index_t nb_points = 0, nb_facets = 0 ;
        for( index_t s = 0; s < nb_surfaces; s++ ) {
            const BoundaryModelElement* surface = region->boundary( s ) ;
            if( Utils::contains( surface_id_,
                static_cast< signed_index_t >( surface->id() ) ) ) continue ;
            nb_points += surface->nb_vertices() ;
            nb_facets += surface->nb_simplices() ;

            surface_id_.push_back( surface->id() ) ;
            unique_surfaces.push_back( surface ) ;
        }

        signed_index_t nb_points_without_well = nb_points ;
        nb_points += well_edges.size() ;
        points_.reserve( nb_points ) ;
        triangles_.reserve( 3*nb_facets ) ;

        MakeUnique uniqueID( unique_surfaces, true ) ;
        uniqueID.add_edges( well_edges_ ) ;
        uniqueID.unique() ;

        const std::vector< vec3 >& unique_points = uniqueID.points() ;
        const std::vector< signed_index_t >& unique_indices = uniqueID.indices() ;
        signed_index_t offset = 0, cur_id = 0 ;
        for( index_t p = 0; p < unique_indices.size(); p++ ) {
            if( cur_id == unique_indices[p] ) {
                cur_id++ ;
                points_.push_back( unique_points[unique_indices[p] + offset] ) ;
            } else {
                offset++ ;
            }
        }

        well_indices_.reserve( well_edges_.size()*2 ) ;
        for( index_t i = nb_points_without_well; i < unique_indices.size(); i++ ) {
            well_indices_.push_back( unique_indices[i]+first_index ) ;
        }

        offset = 0 ;
        for( index_t s = 0; s < unique_surfaces.size(); s++ ) {
            double area = 0 ;
            const SurfacePart& surface = dynamic_cast< const SurfacePart& >( *unique_surfaces[s] ) ;
            for( index_t t = 0; t < surface.nb_simplices(); t++ ) {
                area += surface.simplex_size( t ) ;
                if( surface.is_triangle( t ) ) {
                    for( index_t v = 0; v < 3; v++ ) {
                        triangles_.push_back(
                            unique_indices[offset + surface.point_index( t, v )]+first_index  ) ;
                    }
                } else {
                    double diag0 = length(
                        surface.point( t, 0 ) - surface.point( t, 2 ) ) ;
                    double diag1 = length(
                        surface.point( t, 1 ) - surface.point( t, 3 ) ) ;
                    if( diag0 < diag1 ) {
                        for( index_t v = 0; v < 3; v++ ) {
                            triangles_.push_back(
                                unique_indices[offset + surface.point_index( t, v )]+first_index ) ;
                        }

                        triangles_.push_back(
                            unique_indices[offset + surface.point_index( t, 0 )]+first_index ) ;
                        for( index_t v = 2; v < 4; v++ ) {
                            triangles_.push_back(
                                unique_indices[offset + surface.point_index( t, v )]+first_index ) ;
                        }
                    } else {
                        for( index_t v = 1; v < 4; v++ ) {
                            triangles_.push_back(
                                unique_indices[offset + surface.point_index( t, v )]+first_index ) ;
                        }

                        for( index_t v = 0; v < 2; v++ ) {
                            triangles_.push_back(
                                unique_indices[offset + surface.point_index( t, v )]+first_index ) ;
                        }
                        triangles_.push_back(
                            unique_indices[offset + surface.point_index( t, 3 )]+first_index ) ;
                    }

                }
            }
            offset += surface.nb_vertices() ;
            surface_ptr_.push_back( nb_triangles() ) ;
            area /= static_cast< double >( surface.nb_simplices() ) ;
            double r = sqrt( (double)(4 * area / sqrt( (double)3 )) ) ;
            resolution_ = std::max( r, resolution_ ) ;
        }
    }

    TetraGen::~TetraGen()
    {
    }

    void TetraGen::initialize_storage(
        index_t nb_points,
        index_t nb_tets,
        index_t nb_triangles,
        index_t nb_lines)
    {
        tetmesh_.reserve_vertices( nb_points ) ;
        tetmesh_.reserve_facets_and_corners( nb_triangles,
            nb_tets * 4 + nb_triangles * 3 + nb_lines * 2 ) ;
        GEO::MeshMutator::set_attributes( tetmesh_, GEO::MESH_FACET_REGION ) ;
        GEO::MeshMutator::facet_regions( tetmesh_ ).resize( nb_triangles ) ;
        GEO::MeshMutator::tet_vertices( tetmesh_ ).reserve( nb_tets * 4 ) ;
        GEO::MeshMutator::set_nb_vertices( tetmesh_, nb_points ) ;
//        GEO::MeshMutator::set_nb_facets( tetmesh_, nb_triangles ) ;
//        GEO::MeshMutator::set_nb_cells( tetmesh_, nb_tets ) ;

        /*
        tetmesh_.tetra_adjacents_.resize( nb_tets * 4, -1 ) ;
        tetmesh_.triangle_surface_id_.resize( 4 * nb_tets, -1 ) ;
        */
    }

    void TetraGen::set_point( index_t index, double* point )
    {
        tetmesh_builder_.add_vertex_by_ptr( point ) ;
    }

    void TetraGen::set_tetra( index_t index, int* tet, index_t nb_lines, index_t nb_triangles )
    {
        tetmesh_.add_tet( tet[0] - 1, tet[1] - 1, tet[2] - 1, tet[3] - 1 ) ;
    }

    void TetraGen::set_tetra_adjacent(
        index_t index,
        index_t face,
        signed_index_t adj )
    {
//        grgmesh_assert_not_reached ;
       //tetmesh_.tetra_adjacents_[4 * index + face] = adj ;
    }

    void TetraGen::set_triangle(
        index_t index,
        int * triangle,
        index_t nb_lines )
    {
        tetmesh_builder_.begin_facet() ;
        for( uint8 i = 0; i < 3; i++ ) {
            tetmesh_builder_.add_vertex_to_facet( triangle[i] - 1 ) ;
        }
        tetmesh_builder_.end_facet() ;
    }

    void TetraGen::set_line(
        index_t index,
        int * line )
    {
        grgmesh_assert_not_reached ;
        /*
        tetmesh_builder_.add_line( index, 2 * index ) ;
        for( index_t i = 0; i < 3; i++ ) {
            tetmesh_builder_.add_vertex_index( 2 * index + i, line[i] - 1 ) ;
        }
        */
    }

    void TetraGen::set_face_marker(
        index_t tri,
        index_t marker )
    {
        tetmesh_.set_facet_region( tri, marker ) ;
    }

    void TetraGen::set_tetra_face_marker(
        index_t tet,
        index_t adj,
        index_t marker )
    {
        grgmesh_assert_not_reached ;
        /*
    	TetraAttribute< intArrayTmpl < 4 > > surface_id_tet( &tetmesh_, "surface_id" ) ;
    	surface_id_tet[tet].value(adj) = marker ;
        grgmesh_assert_not_reached
        //tetmesh_.triangle_surface_id_[4 * tet + adj] = marker ;
         *
         */

    }

    void TetraGen::store_edge_attrib() const
    {
        grgmesh_assert_not_reached ;
        /*
        tetmesh_.edges_on_well_.resize( 6 * tetmesh_.nb_tetra(), -1 ) ;
        if( well_edges_.empty() ) return ;
        ColocaterANN ann( well_edges_ ) ;

        const GEO::Mesh::CellDescriptor* tet_descr =
            GEO::Mesh::nb_v_to_cell_descriptor[4] ;
#pragma omp parallel for
        for( index_t t = 0; t < tetmesh_.nb_tetra(); t++ ) {
            for( index_t e = 0; e < 6; e++ ) {
                vec3 v0 = tetmesh_.vertex( t, tet_descr->edge[e][0] ) ;
                vec3 v1 = tetmesh_.vertex( t, tet_descr->edge[e][1] ) ;
                vec3 barycenter( ( v0 + v1 ) / 2.0 ) ;
                std::vector< index_t > result ;
                ann.get_colocated( barycenter, result ) ;
                for( index_t f = 0; f < result.size(); f++ ){
                    tetmesh_.edges_on_well_[6*t+e] = well_id( result[f] ) ;
                }
            }
        }
        */
    }

    TetraGen_TetGen::TetraGen_TetGen(
        GEO::Mesh& tetmesh,
        const BoundaryModelElement* region,
        bool add_steiner_points,
        const std::vector< vec3 >& internal_vertices,
        const std::vector< std::vector< Edge > >& well_edges,
        GEO::Mesh* background )
        :
            TetraGen( tetmesh, region, internal_vertices, well_edges, background )
    {
        tetgen_input_.initialize() ;
        tetgen_input_.firstnumber = 1 ;
        tetgen_input_.numberofpoints = nb_total_points() ;
        tetgen_input_.pointlist = new double[tetgen_input_.numberofpoints*3] ;

#pragma omp parallel for
        for( index_t p = 0; p < nb_points(); p++ ) {
            tetgen_input_.pointlist[3*p] = points_[p].x ;
            tetgen_input_.pointlist[3*p+1] = points_[p].y ;
            tetgen_input_.pointlist[3*p+2] = points_[p].z ;
        }

        tetgen_input_.numberoffacets = nb_triangles() ;
        tetgen_input_.facetlist = new tetgenio::facet[tetgen_input_.numberoffacets] ;
        tetgen_input_.facetmarkerlist = new int[tetgen_input_.numberoffacets] ;

#pragma omp parallel for
        for( index_t f = 0; f < nb_triangles(); f++ ) {
            tetgenio::facet* F = &( tetgen_input_.facetlist[f] ) ;
            tetgenio::init( F ) ;
            F->numberofpolygons = 1 ;
            F->polygonlist = new tetgenio::polygon[F->numberofpolygons] ;
            tetgenio::polygon* P = F->polygonlist ;
tetgenio::init( P ) ;
            P->numberofvertices = 3 ;
            P->vertexlist = new int[P->numberofvertices] ;
            for( index_t v = 0; v < 3; v++ ) {
                P->vertexlist[v] = point_index( f, v ) ;
            }
            F->numberofholes = 0 ;
            F->holelist = nil ;
            tetgen_input_.facetmarkerlist[f] = surface_id( f ) + 1 ; // tetgen starts at 0 and not -1
        }

        tetgen_input_.numberofedges = well_edges.size() ;
        tetgen_input_.edgelist = new int[tetgen_input_.numberofedges*2] ;
        for( unsigned e = 0; e < well_edges.size(); e++ ) {
            tetgen_input_.edgelist[2*e]   = well_indices_[2*e] ;
            tetgen_input_.edgelist[2*e+1] = well_indices_[2*e+1] ;
        }

#pragma omp parallel for
        for( index_t p = 0; p < nb_internal_points(); p++ ) {
            tetgen_input_.pointlist[3*(p+nb_points())] = internal_points_[p].x ;
            tetgen_input_.pointlist[3*(p+nb_points())+1] = internal_points_[p].y ;
            tetgen_input_.pointlist[3*(p+nb_points())+2] = internal_points_[p].z ;
        }

        //todo
        bool use_background_mesh = false ;
        /*
        bool use_background_mesh = background_ && background_->is_resolution_set() ;
        if( use_background_mesh ) {
            tetgen_background_.initialize() ;
            tetgen_background_.firstnumber = 0 ;
            tetgen_background_.numberofpoints = background_->nb_points() ;
            tetgen_background_.pointlist =
                new double[tetgen_background_.numberofpoints * 3] ;
            tetgen_background_.numberofpointmtrs = 1 ;
            tetgen_background_.pointmtrlist =
                new double[tetgen_background_.numberofpoints * tetgen_background_.numberofpointmtrs] ;
#pragma omp parallel for
            for( index_t p = 0; p < tetgen_background_.numberofpoints; p++ ) {
                tetgen_background_.pointlist[3*p] = background_->vertex( p ).x ;
                tetgen_background_.pointlist[3*p+1] = background_->vertex( p ).y ;
                tetgen_background_.pointlist[3*p+2] = background_->vertex( p ).z ;
                tetgen_background_.pointmtrlist[p] = background_->resolution( p ) ;
            }

            tetgen_background_.numberofcorners = 4 ;
            tetgen_background_.numberoftetrahedra = background_->nb_tetra() ;
            tetgen_background_.tetrahedronlist =
                new int[tetgen_background_.numberoftetrahedra * tetgen_background_.numberofcorners] ;
#pragma omp parallel for
            for( index_t t = 0; t < tetgen_background_.numberoftetrahedra; t++ ) {
                for( index_t p = 0; p < tetgen_background_.numberofcorners; p++ ) {
                    tetgen_background_.tetrahedronlist[4*t+p] = background_->vertex_index( t, p ) ;
                }
            }
        }
        */


        std::ostringstream cmd_line ;
        cmd_line << "pYfnn" ;
        if( add_steiner_points ) {
            cmd_line << "q" ;
            if( use_background_mesh ) {
                cmd_line << "m" ;
            } else {
                cmd_line << std::fixed ;
                cmd_line << "a" << sqrt( (double)2) * resolution_ * resolution_ * resolution_ / static_cast< double >( 12 ) ;

            }
        }
        tetgen_args_.parse_commandline( const_cast< char* >( cmd_line.str().c_str() ) ) ;
    }

    bool TetraGen_TetGen::tetrahedralize()
    {
        tetgen_output_.deinitialize() ;
        try {
            ::tetrahedralize( &tetgen_args_, &tetgen_input_, &tetgen_output_,
                nil, &tetgen_background_ ) ;
        } catch( ... ) {
            std::cerr << "Encountered a problem..."
                << std::endl ;
            return false ;
        }

        if( tetgen_output_.numberofpoints == 0 ) return false ;
        index_t nb_triangles = 0;
        index_t nb_lines = 0;

//#pragma omp parallel for
        for( index_t f = 0; f < tetgen_output_.numberoftrifaces; f++ ) {
            signed_index_t face_marker = tetgen_output_.trifacemarkerlist[f] - 1 ;
            if( face_marker == -1 ) continue ;
            nb_triangles++ ;
        }
//#pragma omp parallel for
//        for( index_t l = 0; l < tetgen_output_.numberofedges; l++ ) {
//            signed_index_t line_marker = tetgen_output_.edgemarkerlist[l] - 1 ;
//            if( line_marker == -1 ) continue ;
//            nb_lines++ ;
//        }
        initialize_storage( tetgen_output_.numberofpoints,
            tetgen_output_.numberoftetrahedra, nb_triangles, nb_lines ) ;
//#pragma omp parallel for
        for( index_t p = 0; p < tetgen_output_.numberofpoints; p++ ) {
            set_point( p, &tetgen_output_.pointlist[3 * p] ) ;
        }
//#pragma omp parallel for
        for( index_t p = 0; p < tetgen_output_.numberoftetrahedra; p++ ) {
            set_tetra( p, &tetgen_output_.tetrahedronlist[4 * p], nb_lines, nb_triangles ) ;
        }
//#pragma omp parallel for
        for( index_t p = 0; p < tetgen_output_.numberoftetrahedra; p++ ) {
            for( index_t f = 0; f < 4; f++ ) {
                signed_index_t adj = std::max( tetgen_output_.neighborlist[4 * p + f]-1 , -1 ) ;
                set_tetra_adjacent( p, f, adj ) ;
            }
        }
        index_t cur_index_triangle = 0 ;
        std::vector< index_t > temp ;
        temp.reserve( 8 ) ;
        std::vector< std::vector< index_t > > star( tetgen_output_.numberofpoints, temp ) ;

//#pragma omp parallel for
        for( index_t f = 0; f < tetgen_output_.numberoftrifaces; f++ ) {
            signed_index_t face_marker = tetgen_output_.trifacemarkerlist[f] - 1 ;
            if( face_marker == -1 ) continue ;
            /*
            signed_index_t tet1 = tetgen_output_.adjtetlist[2 * f] - 1 ;
            signed_index_t tet2 = tetgen_output_.adjtetlist[2 * f + 1] - 1 ;
            if( tet1 < 0 ){
                bool found = false ;
                for( index_t ff = 0; ff < 4; ff++ ) {
                    if( Utils::triple_equal(
                        tetmesh_.tetra_vertex_index( tet2, ff, 0 ),
                        tetmesh_.tetra_vertex_index( tet2, ff, 1 ),
                        tetmesh_.tetra_vertex_index( tet2, ff, 2 ),
                        tetgen_output_.trifacelist[3 * f] - 1,
                        tetgen_output_.trifacelist[3 * f + 1] - 1,
                        tetgen_output_.trifacelist[3 * f + 2] - 1 ) ) {
                    	set_triangle(cur_index_triangle, &tetgen_output_.trifacelist[3 *f ], nb_lines) ;
                        for( index_t i = 0; i < 3; i++ ) {
                            star[tetgen_output_.trifacelist[3 *f ] - 1].push_back( cur_index_triangle ) ;
                        }
                        // set_tetra_face_marker( tet2, ff, face_marker ) ;
                        set_face_marker(cur_index_triangle, face_marker) ;
                        cur_index_triangle ++ ;
                        found = true ;
                        break ;
                    }
                }
                grgmesh_debug_assert( found ) ;
            } else if( tet2 < 0 ) {
                bool found = false ;
                for( index_t ff = 0; ff < 4; ff++ ) {
                    if( Utils::triple_equal(
                        tetmesh_.tetra_vertex_index( tet1, ff, 0 ),
                        tetmesh_.tetra_vertex_index( tet1, ff, 1 ),
                        tetmesh_.tetra_vertex_index( tet1, ff, 2 ),
                        tetgen_output_.trifacelist[3 * f] - 1,
                        tetgen_output_.trifacelist[3 * f + 1] - 1,
                        tetgen_output_.trifacelist[3 * f + 2] - 1 ) ) {
                    	set_triangle( cur_index_triangle, &tetgen_output_.trifacelist[3 *f ], nb_lines ) ;
                        for( index_t i = 0; i < 3; i++ ) {
                            star[tetgen_output_.trifacelist[3 *f ] - 1].push_back( cur_index_triangle ) ;
                        }
                        // set_tetra_face_marker( tet1, ff, face_marker ) ;
                        set_face_marker(cur_index_triangle, face_marker) ;
                        cur_index_triangle ++ ;
                        found = true ;
                        break ;
                    }
                }
                grgmesh_debug_assert( found ) ;
            } else {
            	set_triangle( cur_index_triangle, &tetgen_output_.trifacelist[3 *f ], nb_lines ) ;
                for( index_t i = 0; i < 3; i++ ) {
                    star[tetgen_output_.trifacelist[3 *f ] - 1].push_back( cur_index_triangle ) ;
                }
                set_face_marker( tet1, face_marker ) ;
                set_face_marker( tet2, face_marker ) ;
                cur_index_triangle ++ ;
            }
            */

            set_triangle( cur_index_triangle, &tetgen_output_.trifacelist[3 *f ], nb_lines ) ;
            for( index_t i = 0; i < 3; i++ ) {
                star[tetgen_output_.trifacelist[3 *f ] - 1].push_back( cur_index_triangle ) ;
            }
            set_face_marker( cur_index_triangle, face_marker ) ;
            cur_index_triangle ++ ;
        }


        tetmesh_builder_.end_mesh() ;
        Utils::check_and_repair_mesh_consistency( *region_, tetmesh_ ) ;

        /*
        std::vector< bool > is_triangle_visited ( false, nb_triangles ) ;
        std::vector< index_t > surfaces_id ;

        for( index_t i = 0; i < nb_triangles; i++ ) {
            index_t cur_suface_id = tetmesh_.facet_region( i ) ;
            if( Utils::contains( surfaces_id, cur_suface_id ) ) {
                surfaces_id.push_back( cur_suface_id ) ;
                std::stack< index_t > S ;
                S.push( i ) ;
                do {
                    index_t cur_triangle = S.top() ;
                    is_triangle_visited[cur_triangle] = true ;
                    S.pop() ;
                    std::vector< index_t > vertices_on_cur_triangle( 4, -1 ) ;
                    for( index_t j = tetmesh_.facet_begin( cur_triangle );
                        j < tetmesh_.facet_end( cur_triangle ); j++ ) {
                        vertices_on_cur_triangle[j] = tetmesh_.corner_vertex_index(
                            j ) ;
                    }
                    vertices_on_cur_triangle[3] = tetmesh_.corner_vertex_index(
                        tetmesh_.facet_begin( cur_triangle ) ) ;

                    for( uint8 j = 0; j < 3; j++ ) {
                        std::vector< index_t > intersect = Utils::intersect(
                            star[vertices_on_cur_triangle[j]],
                            star[vertices_on_cur_triangle[j + 1]] ) ;
                        for( uint8 k = 0; k < intersect.size(); k++ ) {
                            index_t intersect_triangle = intersect[k] ;
                            if( tetmesh_.facet_region( intersect_triangle )
                                == cur_suface_id
                                && !is_triangle_visited[intersect_triangle] ) {
                                S.push( intersect_triangle ) ;
                                std::vector< index_t > vertices_on_neighbor_triangle(
                                    4, -1 ) ;
                                for( index_t l = tetmesh_.facet_begin(
                                    intersect_triangle );
                                    l < tetmesh_.facet_end( intersect_triangle );
                                    l++ ) {
                                    vertices_on_neighbor_triangle[l] =
                                        tetmesh_.corner_vertex_index( l ) ;
                                }
                                vertices_on_neighbor_triangle[3] =
                                    tetmesh_.corner_vertex_index(
                                        tetmesh_.facet_begin(
                                            intersect_triangle ) ) ;

                                if( Utils::check_order(
                                    vertices_on_neighbor_triangle,
                                    vertices_on_cur_triangle[j],
                                    vertices_on_cur_triangle[j + 1] ) ) {
                                    GEO::MeshMutator::flip_facet( tetmesh_,
                                        intersect_triangle ) ;
                                }
                            }
                        }
                    }
                } while( !S.empty() ) ;
        	}
        }

        /*
        index_t cur_index_line = 0 ;
#pragma omp parallel for
        for( index_t l = 0; l < tetgen_output_.numberofedges; l++ ) {
            signed_index_t line_marker = tetgen_output_.edgemarkerlist[l] - 1 ;
            if( line_marker == -1 ) continue ;
        	set_line(cur_index_line, &tetgen_output_.edgelist[2 *l ]) ;
        	cur_index_line ++ ;
        }
        */

        // store_edge_attrib() ;
        return true ;
    }

#ifdef USE_MG_TETRA

    TetraGen_MG_Tetra::TetraGen_MG_Tetra(
        GEO::Mesh& tetmesh,
        const BoundaryModelElement* region,
        bool add_steiner_points,
        const std::vector< vec3 >& internal_vertices,
        const std::vector< std::vector< Edge > >& well_vertices,
        GEO::Mesh* background )
        :
            TetraGen( tetmesh, region, internal_vertices, well_vertices, background ),
            mesh_output_( nil ),
            add_steiner_points_( add_steiner_points ),
            mesh_background_( nil ),
            sizemap_( nil )
    {
        context_ = context_new() ;
        mesh_input_ = mesh_new_in_memory( context_ ) ;
        status_t ret = context_set_message_callback(context_, my_message_cb, 0);
        grgmesh_debug_assert( ret == STATUS_OK ) ;

        ret = mesh_set_vertex_count( mesh_input_, nb_total_points() ) ;
        grgmesh_debug_assert( ret == STATUS_OK ) ;
        for( index_t p = 0; p < nb_points(); p++ ) {
            ret = mesh_set_vertex_coordinates( mesh_input_, p + 1,
                points_[p].data() ) ;
            grgmesh_debug_assert( ret == STATUS_OK ) ;
        }

        ret = mesh_set_edge_count( mesh_input_, well_edges_.size() ) ;
        grgmesh_debug_assert( ret == STATUS_OK ) ;
        for( index_t e = 0; e < well_edges_.size(); e++ ) {
            ret = mesh_set_edge_vertices( mesh_input_, e + 1,
                &well_indices_[2 * e] ) ;
            grgmesh_debug_assert( ret == STATUS_OK ) ;
        }

        for( index_t p = 0; p < nb_internal_points(); p++ ) {
            ret = mesh_set_vertex_coordinates( mesh_input_, nb_points() + p + 1,
                internal_points_[p].data() ) ;
            grgmesh_debug_assert( ret == STATUS_OK ) ;
        }

        ret = mesh_set_triangle_count( mesh_input_, nb_triangles() ) ;
        grgmesh_debug_assert( ret == STATUS_OK ) ;
        for( index_t t = 0; t < nb_triangles(); t++ ) {
            ret = mesh_set_triangle_vertices( mesh_input_, t + 1,
                &triangles_[3 * t] ) ;
            grgmesh_debug_assert( ret == STATUS_OK ) ;
            mesh_set_triangle_tag( mesh_input_, t+1, surface_id_ptr( t ) ) ;
        }

        if( background_ ) {
            grgmesh_assert_not_reached ;
            /*
            mesh_background_ = mesh_new_in_memory( context_ ) ;
            ret = mesh_set_vertex_count( mesh_background_,
                background_->nb_points() ) ;
            grgmesh_debug_assert( ret == STATUS_OK ) ;
            for( index_t p = 0; p < background_->nb_points(); p++ ) {
                vec3 point = background_->vertex( p ) ;
                ret = mesh_set_vertex_coordinates( mesh_background_, p + 1,
                    background_->ref_vertex( p ).data() ) ;
                grgmesh_debug_assert( ret == STATUS_OK ) ;
            }

            ret = mesh_set_tetrahedron_count( mesh_background_, background_->nb_tetra() ) ;
            grgmesh_debug_assert( ret == STATUS_OK ) ;
            for( index_t t = 0; t < background_->nb_tetra(); t++ ) {
                ret = mesh_set_tetrahedron_vertices( mesh_background_, t + 1,
                    background_->vertex_index_ptr( t ) ) ;
                grgmesh_debug_assert( ret == STATUS_OK ) ;
            }

            sizemap_ = meshgems_sizemap_new( mesh_background_,
                meshgems_sizemap_type_iso_mesh_vertex,
                reinterpret_cast< void* >( get_size_value ), this ) ;
                */
        }

        tms_ = tetra_session_new( context_ ) ;
        ret = tetra_set_surface_mesh( tms_, mesh_input_ ) ;
        grgmesh_debug_assert( ret == STATUS_OK ) ;
        ret = tetra_set_param( tms_, "verbose", "4" ) ;
        grgmesh_debug_assert( ret == STATUS_OK ) ;
        ret = tetra_set_param( tms_, "components", "all" ) ;
        grgmesh_debug_assert( ret == STATUS_OK ) ;
        ret = tetra_set_param( tms_, "optimisation_level", "standard" ) ;
        grgmesh_debug_assert( ret == STATUS_OK ) ;
        ret = tetra_set_param( tms_, "gradation", "1.1" ) ;
        grgmesh_debug_assert( ret == STATUS_OK ) ;
        ret = tetra_set_param( tms_, "pthreads_mode", "aggressive" ) ;
        grgmesh_debug_assert( ret == STATUS_OK ) ;
        ret = tetra_set_param( tms_, "max_number_of_threads", "8" ) ;
        grgmesh_debug_assert( ret == STATUS_OK ) ;
        ret = tetra_set_param( tms_, "max_error_count", "5" ) ;
        grgmesh_debug_assert( ret == STATUS_OK ) ;
    }

    TetraGen_MG_Tetra::~TetraGen_MG_Tetra()
    {
        tetra_regain_mesh( tms_, mesh_output_ ) ;
        tetra_session_delete( tms_ ) ;
        mesh_delete( mesh_input_ ) ;
        context_delete( context_ ) ;
    }

    bool TetraGen_MG_Tetra::tetrahedralize()
    {
        status_t ret = tetra_mesh_boundary( tms_ ) ;
        if( ret != STATUS_OK ) {
            std::cout << "Encountered a problem while meshing boundary..."
                << std::endl ;
            return false ;
        }
        if( add_steiner_points_ ) {
            ret = tetra_insert_volume_vertices( tms_ ) ;
            if( ret != STATUS_OK ) {
                std::cout << "Encountered a problem while meshing inside..."
                    << std::endl ;
                return false ;
            }
            ret = tetra_optimise_volume_regular( tms_ ) ;
            if( ret != STATUS_OK ) {
                std::cout << "Encountered a problem while meshing inside..."
                    << std::endl ;
                return false ;
            }
        }
        ret = tetra_get_mesh( tms_, &mesh_output_ ) ;
        grgmesh_debug_assert( ret == STATUS_OK ) ;
        signed_index_t nb_points = 0 ;
        ret = mesh_get_vertex_count( mesh_output_, &nb_points ) ;
        grgmesh_debug_assert( ret == STATUS_OK ) ;
        signed_index_t nb_tets = 0 ;
        ret = mesh_get_tetrahedron_count( mesh_output_, &nb_tets ) ;
        grgmesh_debug_assert( ret == STATUS_OK ) ;
        signed_index_t nb_triangles = 0 ;
        ret = mesh_get_triangle_count( mesh_output_, &nb_triangles ) ;
        grgmesh_debug_assert( ret == STATUS_OK ) ;
        signed_index_t nb_lines = 0 ;
        ret = mesh_get_edge_count( mesh_output_, &nb_lines ) ;
        grgmesh_debug_assert( ret == STATUS_OK ) ;

        initialize_storage( nb_points, nb_tets, nb_triangles, nb_lines ) ;
        std::vector< index_t > temp ;
        temp.reserve( 15 ) ;
        std::vector< std::vector< index_t > > star( nb_points, temp ) ;
        for( index_t t = 0; t < nb_tets; t++ ) {
            signed_index_t tet[4] ;
            ret = mesh_get_tetrahedron_vertices( mesh_output_, t+1, tet ) ;
            grgmesh_debug_assert( ret == STATUS_OK ) ;
            set_tetra( t, tet, nb_lines, nb_triangles ) ;
            for( index_t i = 0; i < 4; i++ ) {
                star[tet[i] - 1].push_back( t ) ;
            }
        }

#pragma omp parallel for
        for( index_t p = 0; p < nb_points; p++ ) {
            double point[3] ;
            ret = mesh_get_vertex_coordinates( mesh_output_, p+1, point ) ;
            grgmesh_debug_assert( ret == STATUS_OK ) ;
            set_point( p, point ) ;
            std::sort( star[p].begin(), star[p].end() ) ;
        }
        signed_index_t cur_index_triangle = 0 ;
#pragma omp parallel for
        for( index_t t = 0; t < nb_triangles; t++ ) {
            signed_index_t tag = -1 ;
            ret = mesh_get_triangle_tag( mesh_output_, t+1, &tag ) ;
            grgmesh_debug_assert( ret == STATUS_OK ) ;
            if ( tag != -1 ) {
                signed_index_t vertices[3] ;
                ret = mesh_get_triangle_vertices( mesh_output_, t+1, vertices ) ;
                grgmesh_debug_assert( ret == STATUS_OK ) ;
                set_triangle(cur_index_triangle, vertices, nb_lines) ;
                cur_index_triangle ++ ;
            }

            /*
            const std::vector< index_t >& tetra0 = star[ vertices[0]-1 ] ;
            const std::vector< index_t >& tetra1 = star[ vertices[1]-1 ] ;
            const std::vector< index_t >& tetra2 = star[ vertices[2]-1 ] ;
            std::vector< index_t >::const_iterator cur_tetra0 = tetra0.begin() ;
            std::vector< index_t >::const_iterator cur_tetra1 = tetra1.begin() ;
            std::vector< index_t >::const_iterator cur_tetra2 = tetra2.begin() ;
            std::vector< index_t >::const_iterator end_tetra0 = tetra0.end() ;
            std::vector< index_t >::const_iterator end_tetra1 = tetra1.end() ;
            std::vector< index_t >::const_iterator end_tetra2 = tetra2.end() ;
            index_t results[2] ;
            index_t count = 0 ;
            //intersection
            while( cur_tetra0 != end_tetra0 && cur_tetra1 != end_tetra1
                && cur_tetra2 != end_tetra2 && count < 2 ) {
                if( *cur_tetra0 < *cur_tetra1 || *cur_tetra0 < *cur_tetra2 ) {
                    ++cur_tetra0 ;
                } else if( *cur_tetra1 < *cur_tetra0 || *cur_tetra1 < *cur_tetra2 ) {
                    ++cur_tetra1 ;
                } else if( *cur_tetra2 < *cur_tetra0 || *cur_tetra2 < *cur_tetra1 ) {
                    ++cur_tetra2 ;
                } else {
                    results[count++] = *cur_tetra0 ;
                    ++cur_tetra0 ;
                    ++cur_tetra1 ;
                    ++cur_tetra2 ;
                }
            }

            grgmesh_debug_assert( count != 0 ) ;
            if( count == 1 ) {
                index_t tet = results[0] ;
                bool found = false ;
                for( index_t ff = 0; ff < 4; ff++ ) {
                    if( Utils::triple_equal(
                        tetmesh_.tetra_vertex_index( tet, ff, 0 ),
                        tetmesh_.tetra_vertex_index( tet, ff, 1 ),
                        tetmesh_.tetra_vertex_index( tet, ff, 2 ),
                        vertices[0] - 1,
                        vertices[1] - 1,
                        vertices[2] - 1 ) ) {
                        signed_index_t tri[3] ;
                        ret = mesh_get_triangle_vertices( mesh_output_, t+1, tri ) ;
                        grgmesh_debug_assert( ret == STATUS_OK ) ;
                        if ( tag != -1 ) {
                        	set_triangle(cur_index_triangle, tri, nb_lines) ;
                        	cur_index_triangle ++ ;
                        }
                        set_tetra_face_marker( tet, ff, tag ) ;
                        found = true ;
                        break ;
                    }
                }
                grgmesh_debug_assert( found ) ;
            } else if( count == 2 ) {
                index_t tet1 = results[0] ;
                index_t tet2 = results[1] ;
                set_face_marker( tet1, tag ) ;
                set_face_marker( tet2, tag ) ;
            }
            */
        }


        tetmesh_builder_.end_mesh() ;
        Utils::check_and_repair_mesh_consistency( *region_, tetmesh_ ) ;

            /*
        index_t cur_index_line = 0 ;
#pragma omp parallel for
        for( index_t l = 0; l < well_edges_.size(); l++ ) {
            signed_index_t tag = -1 ;
            ret = mesh_get_edge_tag( mesh_output_, l+1, &tag ) ;
            signed_index_t lin[2] ;
            ret = mesh_get_edge_vertices( mesh_output_, l+1, lin ) ;
            if( tag != -1 ) {
				set_line(cur_index_line, lin) ;
				cur_index_line ++ ;
            }
        }


        store_edge_attrib() ;
        */

        return true ;
    }

    status_t TetraGen_MG_Tetra::my_message_cb( message_t * msg, void *user_data )
    {
        char *desc ;
        integer e, ibuff[6] ;
        real rbuff[3] ;

        message_get_description( msg, &desc ) ;
        message_get_number( msg, &e ) ;

        if( e == 0 ) {
            std::cerr << desc << std::endl ;
        } else if( e == MESHGEMS_TETRA_CODE( -5110 ) ) {
            message_get_integer_data( msg, 1, 4, ibuff ) ;
            std::cerr << "two surface edges are intersecting : " << ibuff[0] << " "
                << ibuff[1] << " intersects " << ibuff[2] << " " << ibuff[3]
                << std::endl ;
        } else if( e == MESHGEMS_TETRA_CODE( -5120 ) ) {
            message_get_integer_data( msg, 1, 5, ibuff ) ;
            std::cerr << "surface edge intersects a surface face : " << ibuff[0]
                << " " << ibuff[1] << " intersects " << ibuff[2] << " " << ibuff[3]
                << " " << ibuff[4] << std::endl ;
        } else if( e == MESHGEMS_TETRA_CODE( -5150 ) ) {
            message_get_integer_data( msg, 1, 4, ibuff ) ;
            std::cerr << "boundary point inside a surface face : " << ibuff[0]
                << " in " << ibuff[1] << " " << ibuff[2] << " " << ibuff[3]
                << std::endl ;
        } else if( e == MESHGEMS_TETRA_CODE( 5200 ) ) {
            message_get_integer_data( msg, 1, 3, ibuff ) ;
            std::cerr << "duplicated face : " << ibuff[0] << " " << ibuff[1] << " "
                << ibuff[2] << " " << ibuff[3] << std::endl ;
        } else if( e == MESHGEMS_TETRA_CODE( -5621 ) ) {
            message_get_integer_data( msg, 1, 4, ibuff ) ;
            message_get_real_data( msg, 1, 1, rbuff ) ;
            std::cerr << "degenerated face : face " << ibuff[0] << " (" << ibuff[1]
                << ", " << ibuff[2] << ", " << ibuff[3] << ") with small inradius = "
                << rbuff[0] << std::endl ;
        } else if( e == MESHGEMS_TETRA_CODE( -5820 ) ) {
            message_get_integer_data( msg, 1, 2, ibuff ) ;
            std::cerr << "edge bounding a hole : " << ibuff[0] << " " << ibuff[1]
                << std::endl ;
        } else if( e == MESHGEMS_TETRA_CODE( 8423 ) ) {
            message_get_integer_data( msg, 1, 3, ibuff ) ;
            std::cerr << "constrained face cannot be enforced : " << ibuff[0] << " "
                << ibuff[1] << " " << ibuff[2] << std::endl ;
        } else if( e == MESHGEMS_TETRA_CODE( 8441 ) ) {
            message_get_integer_data( msg, 1, 2, ibuff ) ;
            std::cerr << "constrained edge cannot be enforced : " << ibuff[0] << " "
                << ibuff[1] << std::endl ;
        } else {
            std::cerr << "Error message not directly handle" << std::endl ;
            std::cerr << "Error(" << e << ") : " << desc << std::endl ;
        }
        return STATUS_OK ;
    }
    status_t TetraGen_MG_Tetra::get_size_value(
        meshgems_integer i,
        meshgems_real* size,
        void *user_data ) {
        *size = static_cast< TetraGen_MG_Tetra* >( user_data )->get_resolution_value( i ) ;
        return STATUS_OK ;
    }
    double TetraGen_MG_Tetra::get_resolution_value( signed_index_t i )
    {
        grgmesh_assert_not_reached ;
        return 0 ; //background_->resolution( i ) ;
    }
#endif

}
