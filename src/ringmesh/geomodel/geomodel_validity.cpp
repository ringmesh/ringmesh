/*
 * Copyright (c) 2012-2017, Association Scientifique pour la Geologie et ses Applications (ASGA)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of ASGA nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL ASGA BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *     http://www.ring-team.org
 *
 *     RING Project
 *     Ecole Nationale Superieure de Geologie - GeoRessources
 *     2 Rue du Doyen Marcel Roubault - TSA 70605
 *     54518 VANDOEUVRE-LES-NANCY
 *     FRANCE
 */

#include <ringmesh/geomodel/geomodel_validity.h>

#include <geogram/mesh/triangle_intersection.h>

#include <ringmesh/geomodel/geomodel.h>
#include <ringmesh/geomodel/geomodel_mesh_entity.h>
#include <ringmesh/geomodel/geomodel_geological_entity.h>

#include <ringmesh/mesh/geogram_mesh.h>
#include <ringmesh/mesh/geogram_mesh_builder.h>

/*!
 * @file ringmesh/geomodel/geomodel_validity.cpp
 * @brief Implementation of functions to check the validity of GeoModels
 * @author Jeanne Pellerin
 * @todo Refactor the functions - reorganize to have a cleaner code.
 */

namespace {
    using namespace RINGMesh ;

    bool triangles_intersect(
        const GeoModel& geomodel,
        const GeoModelMeshFacets& facets,
        index_t triangle1,
        index_t triangle2 )
    {
        ringmesh_assert( facets.nb_vertices( triangle1 ) == 3 ) ;
        ringmesh_assert( facets.nb_vertices( triangle2 ) == 3 ) ;
        const GeoModelMeshVertices& vertices = geomodel.mesh.vertices ;
        const vec3& p1 = vertices.vertex( facets.vertex( triangle1, 0 ) ) ;
        const vec3& p2 = vertices.vertex( facets.vertex( triangle1, 1 ) ) ;
        const vec3& p3 = vertices.vertex( facets.vertex( triangle1, 2 ) ) ;

        const vec3& q1 = vertices.vertex( facets.vertex( triangle2, 0 ) ) ;
        const vec3& q2 = vertices.vertex( facets.vertex( triangle2, 1 ) ) ;
        const vec3& q3 = vertices.vertex( facets.vertex( triangle2, 2 ) ) ;
        GEO::vector< GEO::TriangleIsect > sym ;
        return triangles_intersections( p1, p2, p3, q1, q2, q3, sym ) ;
    }

    bool triangle_quad_intersect(
        const GeoModel& geomodel,
        const GeoModelMeshFacets& facets,
        index_t triangle,
        index_t quad )
    {
        ringmesh_assert( facets.nb_vertices( triangle ) == 3 ) ;
        ringmesh_assert( facets.nb_vertices( quad ) == 4 ) ;
        const GeoModelMeshVertices& vertices = geomodel.mesh.vertices ;
        const vec3& p1 = vertices.vertex( facets.vertex( triangle, 0 ) ) ;
        const vec3& p2 = vertices.vertex( facets.vertex( triangle, 1 ) ) ;
        const vec3& p3 = vertices.vertex( facets.vertex( triangle, 2 ) ) ;

        const vec3& q1 = vertices.vertex( facets.vertex( quad, 0 ) ) ;
        const vec3& q2 = vertices.vertex( facets.vertex( quad, 1 ) ) ;
        const vec3& q3 = vertices.vertex( facets.vertex( quad, 2 ) ) ;
        const vec3& q4 = vertices.vertex( facets.vertex( quad, 3 ) ) ;
        GEO::vector< GEO::TriangleIsect > sym ;
        if( triangles_intersections( p1, p2, p3, q1, q2, q3, sym ) ) {
            return true ;
        }
        if( triangles_intersections( p1, p2, p3, q1, q3, q4, sym ) ) {
            return true ;
        }
        return false ;
    }

    bool quad_quad_intersect(
        const GeoModel& geomodel,
        const GeoModelMeshFacets& facets,
        index_t quad1,
        index_t quad2 )
    {
        ringmesh_assert( facets.nb_vertices( quad1 ) == 4 ) ;
        ringmesh_assert( facets.nb_vertices( quad2 ) == 4 ) ;
        const GeoModelMeshVertices& vertices = geomodel.mesh.vertices ;
        const vec3& p1 = vertices.vertex( facets.vertex( quad1, 0 ) ) ;
        const vec3& p2 = vertices.vertex( facets.vertex( quad1, 1 ) ) ;
        const vec3& p3 = vertices.vertex( facets.vertex( quad1, 2 ) ) ;
        const vec3& p4 = vertices.vertex( facets.vertex( quad1, 3 ) ) ;

        const vec3& q1 = vertices.vertex( facets.vertex( quad2, 0 ) ) ;
        const vec3& q2 = vertices.vertex( facets.vertex( quad2, 1 ) ) ;
        const vec3& q3 = vertices.vertex( facets.vertex( quad2, 2 ) ) ;
        const vec3& q4 = vertices.vertex( facets.vertex( quad2, 3 ) ) ;
        GEO::vector< GEO::TriangleIsect > sym ;
        if( triangles_intersections( p1, p2, p3, q1, q2, q3, sym ) ) {
            return true ;
        }
        if( triangles_intersections( p1, p2, p3, q1, q3, q4, sym ) ) {
            return true ;
        }
        if( triangles_intersections( p1, p3, p4, q1, q2, q3, sym ) ) {
            return true ;
        }
        if( triangles_intersections( p1, p3, p4, q1, q3, q4, sym ) ) {
            return true ;
        }
        return false ;
    }

    bool is_edge_on_line( const Line& line, index_t v0, index_t v1 )
    {
        if( v0 > v1 ) {
            std::swap( v0, v1 ) ;
        }
        index_t delta_i = v1 - v0 ;

        if( delta_i == 1 ) {
            // There is an edge if their indices in the Line are i and i+1
            return true ;
        } else if( line.is_closed() && delta_i == line.nb_vertices() - 2 ) {
            // If the Line is closed we can also have 0; n-2 or n-1; 1
            return true ;
        } else {
            // The two points are on the same line but
            // do not define an edge
            return false ;
        }
    }

    /*!
     * @brief Returns the Line identification if the given points define
     *       an edge of one of the Line of the geomodel
     * @param[in] geomodel The GeoModel to consider
     * @param[in] v0 Index in the geomodel of the edge first point
     * @param[in] v1 Index in the geomodel of the edge second point
     */
    bool is_edge_on_line( const GeoModel& geomodel, index_t v0, index_t v1 )
    {
        std::vector< GMEVertex > v0_line_bme ;
        geomodel.mesh.vertices.gme_type_vertices( Line::type_name_static(), v0,
            v0_line_bme ) ;
        if( v0_line_bme.empty() ) {
            return false ;
        }
        std::vector< GMEVertex > v1_line_bme ;
        geomodel.mesh.vertices.gme_type_vertices( Line::type_name_static(), v1,
            v1_line_bme ) ;
        if( v1_line_bme.empty() ) {
            return false ;
        }

        bool found_line = false ;
        for( const GMEVertex& vertex0 : v0_line_bme ) {
            index_t line0_id = vertex0.gmme.index() ;
            for( const GMEVertex& vertex1 : v1_line_bme ) {
                if( line0_id == vertex1.gmme.index() ) {
                    if( !is_edge_on_line( geomodel.line( line0_id ), vertex0.v_index,
                        vertex1.v_index ) ) {
                        return false ;
                    }
                    found_line = true ;
                    break ;
                }
            }
        }
        return found_line ;
    }

    /*!
     * @brief Returns true if the facets @param f1 and @param f2
     *        of the mesh @param facets share an edge
     *        that is on one Line of the boundary geomodel @param geomodel
     *
     */
    bool facets_share_line_edge(
        const GeoModel& geomodel,
        const GeoModelMeshFacets& facets,
        index_t f1,
        index_t f2 )
    {
        // Only test the edges on boundary
        for( index_t v1 = 0; v1 < facets.nb_vertices( f1 ); v1++ ) {
            if( facets.adjacent( f1, v1 ) != NO_ID ) {
                continue ;
            }
            index_t v10 = facets.vertex( f1, v1 ) ;
            index_t v11 = facets.vertex( f1,
                ( v1 + 1 ) % facets.nb_vertices( f1 ) ) ;
            for( index_t v2 = 0; v2 < facets.nb_vertices( f2 ); v2++ ) {
                if( facets.adjacent( f2, v2 ) != NO_ID ) {
                    continue ;
                }
                index_t v20 = facets.vertex( f2, v2 ) ;
                index_t v21 = facets.vertex( f2,
                    ( v2 + 1 ) % facets.nb_vertices( f2 ) ) ;

                if( ( v10 == v20 && v11 == v21 ) || ( v10 == v21 && v11 == v20 ) ) {
                    if( is_edge_on_line( geomodel, v20, v21 ) ) {
                        return true ;
                    }
                }
            }
        }

        return false ;
    }

    bool facets_are_adjacent(
        const GeoModelMeshFacets& facets,
        index_t f1,
        index_t f2 )
    {
        if( f1 == f2 ) {
            return true ;
        }
        for( index_t v = 0; v < facets.nb_vertices( f1 ); v++ ) {
            if( facets.adjacent( f1, v ) == f2 ) {
                return true ;
            }
        }
        return false ;
    }

    /*!
     * @brief Action class for storing intersections when traversing
     *  a AABBTree.
     */
    class StoreIntersections {
    public:
        /*!
         * @brief Constructs the StoreIntersections
         * @param[in] geomodel the geomodel
         * @param[out] has_isect the flag that indicates for each facet
         *  whether it has intersections
         */
        StoreIntersections(
            const GeoModel& geomodel,
            std::vector< bool >& has_isect )
            :
                geomodel_( geomodel ),
                facets_( geomodel.mesh.facets ),
                has_intersection_( has_isect )
        {
            has_intersection_.assign( facets_.nb(), 0 ) ;
        }

        /*!
         * @brief Determines the intersections between two facets
         * @details It is a callback for AABBTree traversal
         * @param[in] f1 index of the first facet
         * @param[in] f2 index of the second facet
         */
        void operator()( index_t f1, index_t f2 )
        {
            if( f1 == f2 || facets_are_adjacent( facets_, f1, f2 )
                || facets_share_line_edge( geomodel_, facets_, f1, f2 ) ) {
                return ;
            }

            if( is_triangle( f1 ) ) {
                if( is_triangle( f2 ) ) {
                    if( triangles_intersect( geomodel_, facets_, f1, f2 ) ) {
                        has_intersection_[f1] = 1 ;
                        has_intersection_[f2] = 1 ;
                    }
                } else if( is_quad( f2 ) ) {
                    if( triangle_quad_intersect( geomodel_, facets_, f1, f2 ) ) {
                        has_intersection_[f1] = 1 ;
                        has_intersection_[f2] = 1 ;
                    }
                } else {
                    ringmesh_assert_not_reached ;
                }
            } else if( is_quad( f1 ) ) {
                if( is_triangle( f2 ) ) {
                    if( triangle_quad_intersect( geomodel_, facets_, f2, f1 ) ) {
                        has_intersection_[f1] = 1 ;
                        has_intersection_[f2] = 1 ;
                    }
                } else if( is_quad( f2 ) ) {
                    if( quad_quad_intersect( geomodel_, facets_, f1, f2 ) ) {
                        has_intersection_[f1] = 1 ;
                        has_intersection_[f2] = 1 ;
                    }
                } else {
                    ringmesh_assert_not_reached ;
                }
            } else {
                ringmesh_assert_not_reached ;
            }
        }

        bool is_triangle( index_t f ) const
        {
            index_t index ;
            return facets_.type( f, index ) == GeoModelMeshFacets::TRIANGLE ;
        }
        bool is_quad( index_t f ) const
        {
            index_t index ;
            return facets_.type( f, index ) == GeoModelMeshFacets::QUAD ;
        }

    private:
        const GeoModel& geomodel_ ;
        const GeoModelMeshFacets& facets_ ;
        std::vector< bool >& has_intersection_ ;
    } ;

    void save_mesh_locating_geomodel_inconsistencies(
        const GEO::Mesh& mesh,
        const std::ostringstream& file )
    {
        if( GEO::CmdLine::get_arg_bool( "validity_save" ) ) {
            GEO::mesh_save( mesh, file.str() ) ;
        }
    }

    /***************************************************************************/

    /*!
     * @brief Check if entity @param is of the @param geomodel is in the
     *        in_boundary vector of entity @param in.
     */
    bool is_in_in_boundary(
        const GeoModel& geomodel,
        const gmme_id& is,
        const gmme_id& in )
    {
        const GeoModelMeshEntity& E = geomodel.mesh_entity( in ) ;
        for( index_t i = 0; i < E.nb_in_boundary(); ++i ) {
            if( E.in_boundary_gmme( i ) == is ) {
                return true ;
            }
        }
        return false ;
    }

    void save_invalid_points(
        const std::ostringstream& file,
        const GeoModel& geomodel,
        const std::vector< bool >& valid )
    {
        GEO::Mesh point_mesh ;
        for( index_t i = 0; i < valid.size(); ++i ) {
            if( !valid[i] ) {
                const vec3& V = geomodel.mesh.vertices.vertex( i ) ;
                point_mesh.vertices.create_vertex( V.data() ) ;
            }
        }
        save_mesh_locating_geomodel_inconsistencies( point_mesh, file ) ;
    }

    /*!
     * @brief Check the geometrical-topological consistency of the geomodel
     * @details Verification is based on the information stored by the unique
     *          vertices of the geomodel which validity must be checked beforehand
     * @todo Check that the geomodel vertices are consistent with the geomodel_vertex_ids
     *       stored at by the GMME
     * @todo Implementation for regions
     * @todo Split in smaller functions
     */
    bool check_model_points_validity( const GeoModel& geomodel )
    {
        // For all the vertices of the geomodel 
        // We check that the entities in which they are are consistent 
        // to have a valid B-Rep geomodel
        std::vector< bool > valid( geomodel.mesh.vertices.nb(), true ) ;
        for( index_t i = 0; i < geomodel.mesh.vertices.nb(); ++i ) {
            bool valid_vertex = true ;

            // Get the mesh entities in which this vertex is            
            index_t corner = NO_ID ;
            std::vector< index_t > lines ;
            std::vector< index_t > surfaces ;
            std::vector< index_t > regions ;

            std::vector< GMEVertex > bmes ;
            geomodel.mesh.vertices.gme_vertices( i, bmes ) ;

            for( const GMEVertex& vertex : bmes ) {
                const MeshEntityType& T = vertex.gmme.type() ;
                index_t id = vertex.gmme.index() ;
                if( T == Region::type_name_static() ) {
                    regions.push_back( id ) ;
                } else if( T == Surface::type_name_static() ) {
                    surfaces.push_back( id ) ;
                } else if( T == Line::type_name_static() ) {
                    lines.push_back( id ) ;
                } else if( T == Corner::type_name_static() ) {
                    if( corner != NO_ID ) {
                        Logger::warn( "GeoModel", " Vertex ", i,
                            " is in at least 2 Corners" ) ;
                        valid_vertex = false ;
                    } else {
                        corner = id ;
                    }
                } else {
                    Logger::warn( "GeoModel", " Vertex ", i,
                        " is in no Entity of the Model" ) ;
                    valid_vertex = false ;
                    break ;
                }
            }

            if( valid_vertex ) {
                if( surfaces.empty() ) {
                    if( regions.size() != 1 ) {
                        std::ostringstream oss ;
                        oss << " Vertex " << i << " is in " << regions.size()
                            << " Regions: " ;
                        for( index_t region : regions ) {
                            oss << region << " ; " ;
                        }
                        Logger::warn( "GeoModel", oss.str() ) ;
                        valid_vertex = false ;
                    } /// @todo Implement the other conditions for Region point validity
                } else if( corner == NO_ID && lines.empty() ) {
                    // This is a point on one SURFACE and only one
                    if( surfaces.size() != 1 ) {
                        std::ostringstream oss ;
                        oss << " Vertex " << i << " is in " << surfaces.size()
                            << " Surfaces: " ;
                        for( index_t surface : surfaces ) {
                            oss << surface << " ; " ;
                        }
                        Logger::warn( "GeoModel", oss.str() ) ;
                        valid_vertex = false ;
                    }
                } else if( corner == NO_ID && !lines.empty() ) {
                    // This is a point on one LINE 
                    if( lines.size() != 1 ) {
                        std::ostringstream oss ;
                        oss << " Vertex " << i << " is in " << lines.size()
                            << " Lines " ;
                        for( index_t line : lines ) {
                            oss << line << " ; " ;
                        }
                        Logger::warn( "GeoModel", oss.str() ) ;
                        valid_vertex = false ;
                    } else {
                        // This point must also be in at least one SURFACE
                        if( surfaces.empty() ) {
                            Logger::warn( "GeoModel", " Vertex ", i,
                                " is in a Line but in no Surface " ) ;
                            valid_vertex = false ;
                        }
                        // Check that one point is no more than twice in a SURFACE
                        for( index_t surface : surfaces ) {
                            index_t nb = static_cast< index_t >( std::count(
                                surfaces.begin(), surfaces.end(), surface ) ) ;
                            if( nb > 2 ) {
                                Logger::warn( "GeoModel", " Vertex ", i, " is ", nb,
                                    " times in Surface ",
                                    geomodel.surface( surface ).gmme() ) ;
                                valid_vertex = false ;
                            } else if( nb == 2 ) {
                                // If a point is twice in a SURFACE, it must be
                                // on an internal boundary Line.
                                bool internal_boundary = false ;
                                for( index_t line : lines ) {
                                    if( geomodel.line( line ).is_inside_border(
                                        geomodel.surface( surface ) ) ) {
                                        internal_boundary = true ;
                                        break ;
                                    }
                                }
                                if( !internal_boundary ) {
                                    Logger::warn( "GeoModel", " Vertex ", i,
                                        " appears ", nb, " times in Surface ",
                                        geomodel.surface( surface ).gmme() ) ;
                                    valid_vertex = false ;
                                }
                            }
                        }
                        // Check that all the surfaces are in in_boundary of all
                        // the lines 
                        for( index_t surface : surfaces ) {
                            for( index_t line : lines ) {
                                gmme_id s_id( Surface::type_name_static(),
                                    surface ) ;
                                gmme_id l_id( Line::type_name_static(), line ) ;
                                if( !is_in_in_boundary( geomodel, s_id, l_id ) ) {
                                    Logger::warn( "GeoModel",
                                        " Inconsistent Line-Surface connectivity ",
                                        " Vertex ", i, " shows that ", s_id,
                                        " must be in the boundary of ", l_id ) ;
                                    valid_vertex = false ;
                                }
                            }
                        }
                    }
                } else if( corner != NO_ID ) {
                    // This is one point at a CORNER
                    // It must be in at least one LINE
                    if( lines.empty() ) {
                        Logger::warn( "GeoModel", " Vertex ", i,
                            " is at a Corner but in no Line " ) ;
                        valid_vertex = false ;
                    } else {
                        if( lines.size() < 2 ) {
                            Logger::warn( "GeoModel", " Vertex ", i,
                                " is in at a Corner but in one Line only: ",
                                lines.front() ) ;
                            valid_vertex = false ;
                        }
                        // Check that a point is no more than twice in a LINE
                        for( index_t line : lines ) {
                            index_t nb = static_cast< index_t >( std::count(
                                lines.begin(), lines.end(), line ) ) ;
                            if( nb == 2 ) {
                                // The line must be closed
                                if( !geomodel.line( line ).is_closed() ) {
                                    Logger::warn( "GeoModel", " Vertex ", i,
                                        " is twice in Line ", line ) ;
                                    valid_vertex = false ;
                                }
                            }
                            if( nb > 2 ) {
                                Logger::warn( "GeoModel", " Vertex ", i, " appears ",
                                    nb, " times in Line ", line ) ;
                                valid_vertex = false ;
                                break ;
                            }
                        }
                        // Check that all the lines are in in_boundary of this corner
                        for( index_t line : lines ) {
                            gmme_id l_id( Line::type_name_static(), line ) ;
                            gmme_id c_id( Corner::type_name_static(), corner ) ;
                            if( !is_in_in_boundary( geomodel, l_id, c_id ) ) {
                                Logger::warn( "GeoModel",
                                    " Inconsistent Line-Corner connectivity ",
                                    " vertex ", i, " shows that ", l_id,
                                    " must be in the boundary of ", c_id ) ;
                                valid_vertex = false ;
                            }
                        }
                    }
                    // It must also be in a least one surface ? perhaps 2
                    if( surfaces.empty() ) {
                        Logger::warn( "GeoModel", " Vertex ", i,
                            " is at a Corner but in no Surface " ) ;
                        valid_vertex = false ;
                    }
                }
            }
            valid[i] = valid_vertex ;
        }
        index_t nb_invalid = static_cast< index_t >( std::count( valid.begin(),
            valid.end(), false ) ) ;

        if( nb_invalid > 0 ) {
            std::ostringstream file ;
            file << validity_errors_directory << "/invalid_global_vertices.geogram" ;
            save_invalid_points( file, geomodel, valid ) ;

            if( GEO::CmdLine::get_arg_bool( "validity_save" ) ) {
                Logger::warn( "GeoModel", nb_invalid, " invalid vertices" ) ;
                Logger::warn( "GeoModel", "Saved in file: ", file.str() ) ;
            }

            return false ;
        } else {
            return true ;
        }
    }

    void save_edges(
        const std::ostringstream& file,
        const GeoModel& geomodel,
        const std::vector< index_t >& e )
    {
        GEO::Mesh edge_mesh ;
        index_t previous_vertex_id = NO_ID ;
        for( index_t i = 0; i < e.size(); ++i ) {
            index_t cur_vertex_id = edge_mesh.vertices.create_vertex(
                geomodel.mesh.vertices.vertex( e[i] ).data() ) ;
            if( i % 2 == 0 ) {
                ringmesh_assert( previous_vertex_id == NO_ID ) ;
                previous_vertex_id = cur_vertex_id ;
                continue ;
            }
            ringmesh_assert( previous_vertex_id != NO_ID ) ;
            edge_mesh.edges.create_edge( previous_vertex_id, cur_vertex_id ) ;
            previous_vertex_id = NO_ID ;
        }
        save_mesh_locating_geomodel_inconsistencies( edge_mesh, file ) ;
    }

    void save_facets(
        const std::string& file,
        const Surface& surface,
        const std::vector< index_t >& facets )
    {
        GEO::Mesh mesh ;
        for( index_t cur_facet : facets ) {
            index_t nb_vertices_in_facet = surface.nb_mesh_element_vertices(
                cur_facet ) ;
            GEO::vector< index_t > vertices ;
            vertices.reserve( nb_vertices_in_facet ) ;
            for( index_t v = 0; v < nb_vertices_in_facet; v++ ) {
                index_t new_vertex = mesh.vertices.create_vertex(
                    surface.mesh_element_vertex( cur_facet, v ).data() ) ;
                vertices.push_back( new_vertex ) ;
            }
            mesh.facets.create_polygon( vertices ) ;
        }
        GEO::mesh_save( mesh, file ) ;
    }

    /*!
     * @brief Check boundary of a surface
     * @details All the edges on the boundary of a surface must be in a Line
     *          of the associated geomodel
     *          The Line boundaries must form a closed manifold line.
     */
    bool surface_boundary_valid( const Surface& surface )
    {
        const GeoModelMeshVertices& geomodel_vertices =
            surface.geomodel().mesh.vertices ;
        std::vector< index_t > invalid_corners ;
        for( index_t f = 0; f < surface.nb_mesh_elements(); ++f ) {
            for( index_t v = 0; v < surface.nb_mesh_element_vertices( f ); ++v ) {
                if( surface.facet_adjacent_index( f, v ) == NO_ID
                    && !is_edge_on_line( surface.geomodel(),
                        geomodel_vertices.geomodel_vertex_id( surface.gmme(), f, v ),
                        geomodel_vertices.geomodel_vertex_id( surface.gmme(), f,
                            surface.next_facet_vertex_index( f, v ) ) ) ) {
                    invalid_corners.push_back(
                        geomodel_vertices.geomodel_vertex_id( surface.gmme(), f,
                            v ) ) ;
                    invalid_corners.push_back(
                        geomodel_vertices.geomodel_vertex_id( surface.gmme(), f,
                            surface.next_facet_vertex_index( f, v ) ) ) ;
                }
            }
        }
        if( !invalid_corners.empty() ) {
            std::ostringstream file ;
            file << validity_errors_directory << "/invalid_boundary_surface_"
                << surface.index() << ".geogram" ;
            save_edges( file, surface.geomodel(), invalid_corners ) ;

            if( GEO::CmdLine::get_arg_bool( "validity_save" ) ) {
                Logger::warn( "GeoModel", " Invalid surface boundary: ",
                    invalid_corners.size() / 2, " boundary edges of ",
                    surface.gmme(), "  are in no line of the geomodel " ) ;
                Logger::warn( "GeoModel", " Saved in file: ", file.str() ) ;
            }

            return false ;
        } else {
            return true ;
        }
    }

    /*!
     * @brief Save in a .lin file the
     */
    void debug_save_non_manifold_edges(
        const GeoModel& geomodel,
        const std::vector< index_t >& edge_indices,
        const std::vector< index_t >& non_manifold_edges )
    {
        GeogramMesh1D mesh ;
        GeogramMesh1DBuilder builder ;
        builder.set_mesh( mesh ) ;
        index_t nb_edges = static_cast< index_t >( non_manifold_edges.size() ) ;
        builder.create_vertices( 2 * nb_edges ) ;
        builder.create_edges( nb_edges ) ;
        const GeoModelMeshVertices& vertices = geomodel.mesh.vertices ;
        for( index_t e = 0; e < non_manifold_edges.size(); e++ ) {
            index_t edge_id = non_manifold_edges[e] ;
            const vec3& v0 = vertices.vertex( edge_indices[edge_id] ) ;
            const vec3& v1 = vertices.vertex( edge_indices[edge_id + 1] ) ;
            builder.set_vertex( 2 * e, v0 ) ;
            builder.set_vertex( 2 * e + 1, v1 ) ;
            builder.set_edge_vertex( e, 0, 2 * e ) ;
            builder.set_edge_vertex( e, 1, 2 * e + 1 ) ;
        }
        mesh.save_mesh( validity_errors_directory + "/non_manifold_edges.geogram" ) ;
    }

    bool is_surface_conformal_to_volume(
        const Surface& surface,
        const NNSearch& cell_facet_barycenter_nn_search )
    {
        std::vector< index_t > unconformal_facets ;
        for( index_t f = 0; f < surface.nb_mesh_elements(); f++ ) {
            vec3 center = surface.mesh_element_barycenter( f ) ;
            std::vector< index_t > result =
                cell_facet_barycenter_nn_search.get_neighbors( center,
                    surface.geomodel().epsilon() ) ;
            if( result.empty() ) {
                unconformal_facets.push_back( f ) ;
            }
        }
        if( !unconformal_facets.empty() ) {
            std::ostringstream file ;
            file << validity_errors_directory << "/unconformal_surface_"
                << surface.index() << ".geogram" ;
            save_facets( file.str(), surface, unconformal_facets ) ;

            if( GEO::CmdLine::get_arg_bool( "validity_save" ) ) {
                Logger::warn( "GeoModel", " Unconformal surface: ",
                    unconformal_facets.size(), " facets of ", surface.gmme(),
                    " are unconformal with the geomodel cells " ) ;
                Logger::warn( "GeoModel", " Saved in file: ", file.str() ) ;
            }

            return false ;
        } else {
            return true ;
        }
    }

    void compute_border_edges(
        const GeoModel& geomodel,
        std::vector< index_t >& edge_indices )
    {
        const GeoModelMeshFacets& facets = geomodel.mesh.facets ;
        for( index_t s = 0; s < geomodel.nb_surfaces(); s++ ) {
            for( index_t f = 0; f < facets.nb_facets( s ); f++ ) {
                index_t facet_id = facets.facet( s, f ) ;
                for( index_t v = 0; v < facets.nb_vertices( facet_id ); v++ ) {
                    index_t adj = facets.adjacent( facet_id, v ) ;
                    if( adj == NO_ID ) {
                        edge_indices.push_back( facets.vertex( facet_id, v ) ) ;
                        index_t next_v = ( v + 1 ) % facets.nb_vertices( facet_id ) ;
                        edge_indices.push_back( facets.vertex( facet_id, next_v ) ) ;
                    }
                }
            }
        }
    }

    void compute_border_edge_barycenters(
        const GeoModel& geomodel,
        const std::vector< index_t >& edge_indices,
        std::vector< vec3 >& edge_barycenters )
    {
        const GeoModelMeshVertices& vertices = geomodel.mesh.vertices ;
        index_t nb_edges = static_cast< index_t >( edge_indices.size() / 2 ) ;
        edge_barycenters.reserve( nb_edges ) ;
        for( index_t e = 0; e < edge_indices.size(); e += 2 ) {
            const vec3& v0 = vertices.vertex( edge_indices[e] ) ;
            const vec3& v1 = vertices.vertex( edge_indices[e + 1] ) ;
            edge_barycenters.push_back( ( v0 + v1 ) * 0.5 ) ;
        }
    }

    void compute_edge_on_lines(
        const GeoModel& geomodel,
        const std::vector< vec3 >& edge_barycenters,
        std::vector< bool >& edge_on_lines )
    {
        edge_on_lines.resize( edge_barycenters.size(), false ) ;
        NNSearch nn( edge_barycenters ) ;
        for( index_t l = 0; l < geomodel.nb_lines(); l++ ) {
            const Line& line = geomodel.line( l ) ;
            for( index_t e = 0; e < line.nb_mesh_elements(); e++ ) {
                const vec3 query = line.mesh_element_barycenter( e ) ;
                std::vector< index_t > results = nn.get_neighbors( query,
                    geomodel.epsilon() ) ;
                for( index_t edge : results ) {
                    edge_on_lines[edge] = true ;
                }
            }
        }
    }

    void compute_non_manifold_edges(
        const std::vector< bool >& edge_on_lines,
        std::vector< index_t >& non_manifold_edges )
    {
        for( index_t e = 0; e < edge_on_lines.size(); e++ ) {
            if( !edge_on_lines[e] ) {
                non_manifold_edges.push_back( e ) ;
            }
        }
    }

    /*!
     * @brief Implementation class for validity checks on a GeoModel
     */
    class GeoModelValidityCheck {
    public:
        GeoModelValidityCheck(
            const GeoModel& geomodel,
            const ValidityCheckMode validity_check_mode )
            : geomodel_( geomodel ), valid_( true ), mode_( validity_check_mode )
        {
            if( mode_ == UNDEFINED ) {
                // If not defined, reset the check mode to the largest possible
                mode_ = ALL ;
            }
            // Ensure that the geomodel vertices are computed and up-to-date
            // Without that we cannot do anything        
            geomodel_.mesh.vertices.test_and_initialize() ;
            geomodel_.mesh.facets.test_and_initialize() ;
        }

        /*!
         * @brief Run the geomodel validity check with the specified mode. If no mode
         * is given, the object mode is used by default.
         */
        bool is_geomodel_valid( const ValidityCheckMode specified_mode = UNDEFINED )
        {
            if( specified_mode != UNDEFINED ) {
                do_check_validity( specified_mode ) ;
            } else {
                do_check_validity( mode_ ) ;
            }
            return valid_ ;
        }

    private:
        void do_check_validity( ValidityCheckMode mode )
        {
            GEO::ThreadGroup threads ;
            if( mode == GEOMETRY || mode == ALL ) {
                threads.push_back( new TestFacetIntersections( *this ) ) ;
            }
            if( mode != TOPOLOGY ) {
                // Add geometrical validity check
                threads.push_back( new TestGeomodelMeshEntitiesValidity( *this ) ) ;
                threads.push_back(
                    new TestGeometryConnectivityConsistency3D( *this ) ) ;
                threads.push_back( new TestNonManifoldEdges( *this ) ) ;

            }
            if( mode != GEOMETRY ) {
                // Add topological validity check
                threads.push_back( new TestGeomodelConnectivityValidity( *this ) ) ;
                threads.push_back( new TestFiniteExtension( *this ) ) ;
                threads.push_back(
                    new TestGeometryConnectivityConsistency( *this ) ) ;
            }

            // Geological validity must always be checked
            threads.push_back( new TestGeomodelGeologicalValidity( *this ) ) ;

            RINGMESH_PARALLEL_LOOP_DYNAMIC
            for( index_t i = 0; i < threads.size(); i++ ) {
                threads[i]->run() ;
            }
        }

        /*! 
         * @brief Verify the validity of all GeoModelMeshEntities
         */
        class TestGeomodelMeshEntitiesValidity final : public GEO::Thread {
        public:
            TestGeomodelMeshEntitiesValidity( GeoModelValidityCheck& validity )
                : validity_( validity )
            {
            }
            virtual void run() final
            {
                if( !are_geomodel_mesh_entities_valid( validity_.geomodel_ ) ) {
                    validity_.set_invalid_model() ;
                }
            }
        private:
            GeoModelValidityCheck& validity_ ;
        } ;

        /*!
         * @brief Verify the validity of all GeoModelEntities
         */
        class TestGeomodelConnectivityValidity final : public GEO::Thread {
        public:
            TestGeomodelConnectivityValidity( GeoModelValidityCheck& validity )
                : validity_( validity )
            {
            }
            virtual void run() final
            {
                if( !are_geomodel_mesh_entities_connectivity_valid(
                    validity_.geomodel_ ) ) {
                    validity_.set_invalid_model() ;
                }
            }
        private:
            GeoModelValidityCheck& validity_ ;
        } ;

        /*!
         * @brief Verify the validity of all GeoModelGeologicalEntities
         */
        class TestGeomodelGeologicalValidity final : public GEO::Thread {
        public:
            TestGeomodelGeologicalValidity( GeoModelValidityCheck& validity )
                : validity_( validity )
            {
            }
            virtual void run() final
            {
                if( !are_geomodel_geological_entities_valid(
                    validity_.geomodel_ ) ) {
                    validity_.set_invalid_model() ;
                }
            }
        private:
            GeoModelValidityCheck& validity_ ;
        } ;

        /*!
         * @brief Check that the geomodel has a finite extension
         * @details The boundary of the universe region is a one connected component
         * manifold closed surface.
         * @todo Implement this check
         */
        class TestFiniteExtension final : public GEO::Thread {
        public:
            TestFiniteExtension( GeoModelValidityCheck& validity )
                : validity_( validity )
            {
            }
            virtual void run() final
            {
                if( !validity_.geomodel_.universe().is_valid() ) {
                    validity_.set_invalid_model() ;
                }
            }
        private:
            GeoModelValidityCheck& validity_ ;
        } ;

        /*!
         * Check geometrical-connectivity consistency
         * @todo Check that all Line segments correspond to a Surface
         *  edge that is on the boundary.
         */
        class TestGeometryConnectivityConsistency final : public GEO::Thread {
        public:
            TestGeometryConnectivityConsistency( GeoModelValidityCheck& validity )
                : validity_( validity )
            {
            }
            virtual void run() final
            {
                // Check relationships between GeoModelEntities
                // sharing the same point of the geomodel
                if( !check_model_points_validity( validity_.geomodel_ ) ) {
                    validity_.set_invalid_model() ;
                }
                // Check on that Surface edges are in a Line
                for( index_t i = 0; i < validity_.geomodel_.nb_surfaces(); ++i ) {
                    if( !surface_boundary_valid(
                        validity_.geomodel_.surface( i ) ) ) {
                        validity_.set_invalid_model() ;
                    }
                }
            }
        private:
            GeoModelValidityCheck& validity_ ;
        } ;

        class TestGeometryConnectivityConsistency3D final : public GEO::Thread {
        public:
            TestGeometryConnectivityConsistency3D( GeoModelValidityCheck& validity )
                : validity_( validity )
            {
            }
            virtual void run() final
            {
                if( validity_.geomodel_.mesh.cells.nb() > 0 ) {
                    // Check the consistency between Surface facets and Region cell facets
                    const NNSearch& nn_search =
                        validity_.geomodel_.mesh.cells.cell_facet_nn_search() ;
                    for( index_t i = 0; i < validity_.geomodel_.nb_surfaces();
                        ++i ) {
                        if( !is_surface_conformal_to_volume(
                            validity_.geomodel_.surface( i ), nn_search ) ) {
                            validity_.set_invalid_model() ;
                        }
                    }
                }
            }
        private:
            GeoModelValidityCheck& validity_ ;
        } ;

        /*!
         * @brief Returns true if there are non-manifold edges that are
         *        not in any Line of the geomodel
         * @note Connect the facets of the global mesh
         * @note This is a quite expensive test.
         */
        class TestNonManifoldEdges final : public GEO::Thread {
        public:
            TestNonManifoldEdges( GeoModelValidityCheck& validity )
                : validity_( validity )
            {
            }
            virtual void run() final
            {
                std::vector< index_t > edge_indices ;
                compute_border_edges( validity_.geomodel_, edge_indices ) ;
                std::vector< vec3 > edge_barycenters ;
                compute_border_edge_barycenters( validity_.geomodel_, edge_indices,
                    edge_barycenters ) ;
                std::vector< bool > edge_on_lines ;
                compute_edge_on_lines( validity_.geomodel_, edge_barycenters,
                    edge_on_lines ) ;
                std::vector< index_t > non_manifold_edges ;
                compute_non_manifold_edges( edge_on_lines, non_manifold_edges ) ;

                if( !non_manifold_edges.empty() ) {
                    Logger::warn( "GeoModel", non_manifold_edges.size(),
                        " non-manifold edges " ) ;
                    debug_save_non_manifold_edges( validity_.geomodel_, edge_indices,
                        non_manifold_edges ) ;

                    validity_.set_invalid_model() ;
                }
            }
        private:
            GeoModelValidityCheck& validity_ ;
        } ;

        /*!
         * @brief Returns true if there are intersections between facets
         * @details Operates on the global mesh
         * @note This is a very expensive test.
         */
        class TestFacetIntersections final : public GEO::Thread {
        public:
            TestFacetIntersections( GeoModelValidityCheck& validity )
                : validity_( validity )
            {
            }
            virtual void run() final
            {
                if( validity_.geomodel_.mesh.facets.nb()
                    == validity_.geomodel_.mesh.facets.nb_triangle()
                        + validity_.geomodel_.mesh.facets.nb_quad() ) {
                    std::vector< bool > has_intersection ;
                    StoreIntersections action( validity_.geomodel_,
                        has_intersection ) ;
                    const AABBTree2D& AABB = validity_.geomodel_.mesh.facets.aabb() ;
                    AABB.compute_self_element_bbox_intersections( action ) ;

                    index_t nb_intersections = static_cast< index_t >( std::count(
                        has_intersection.begin(), has_intersection.end(), 1 ) ) ;

                    if( nb_intersections > 0 ) {
                        GEO::Mesh mesh ;
                        for( index_t f = 0; f < has_intersection.size(); f++ ) {
                            if( !has_intersection[f] ) continue ;
                            GEO::vector< index_t > vertices ;
                            vertices.reserve(
                                validity_.geomodel_.mesh.facets.nb_vertices( f ) ) ;
                            for( index_t v = 0;
                                v < validity_.geomodel_.mesh.facets.nb_vertices( f );
                                v++ ) {
                                index_t id = mesh.vertices.create_vertex(
                                    validity_.geomodel_.mesh.vertices.vertex(
                                        validity_.geomodel_.mesh.facets.vertex( f,
                                            v ) ).data() ) ;
                                vertices.push_back( id ) ;
                            }
                            mesh.facets.create_polygon( vertices ) ;
                        }
                        std::ostringstream file ;
                        file << validity_errors_directory
                            << "/intersected_facets.geogram" ;
                        save_mesh_locating_geomodel_inconsistencies( mesh, file ) ;
                        Logger::out( "I/O" ) ;
                        Logger::warn( "GeoModel", nb_intersections,
                            " facet intersections " ) ;
                        validity_.set_invalid_model() ;
                    }
                } else {
                    Logger::warn( "GeoModel",
                        "Polygonal intersection check not implemented yet" ) ;
                }
            }
        private:
            GeoModelValidityCheck& validity_ ;
        } ;

        void set_invalid_model()
        {
            valid_ = false ;
        }

    private:
        const GeoModel& geomodel_ ;
        bool valid_ ;
        ValidityCheckMode mode_ ;
    } ;

}
// anonymous namespace

namespace RINGMesh {

    void set_validity_errors_directory( const std::string& directory )
    {
        // If trailing / or \ is not removed, the test fails on Windows
        std::string copy( directory ) ;
        if( *copy.rbegin() == '/' || *copy.rbegin() == '\\' ) {
            copy.erase( copy.end() - 1 ) ;
        }
        if( GEO::FileSystem::is_directory( copy ) ) {
            validity_errors_directory = copy + '/' ;
        }
    }

    bool are_geomodel_mesh_entities_valid( const GeoModel& geomodel )
    {
        const std::vector< MeshEntityType >& meshed_types =
            MeshEntityTypeManager::mesh_entity_types() ;
        index_t count_invalid = 0 ;
        for( const MeshEntityType& type : meshed_types ) {
            index_t nb_entities = geomodel.nb_mesh_entities( type ) ;
            for( index_t i = 0; i < nb_entities; ++i ) {
                const GeoModelMeshEntity& E = geomodel.mesh_entity( type, i ) ;
                if( !E.is_valid() ) {
                    count_invalid++ ;
                }
            }
        }
        if( count_invalid != 0 ) {
            Logger::warn( "GeoModel", count_invalid,
                " mesh entities of the geomodel have an invalid mesh." ) ;
        }
        return count_invalid == 0 ;
    }

    bool are_geomodel_mesh_entities_connectivity_valid( const GeoModel& geomodel )
    {
        const std::vector< MeshEntityType >& meshed_types =
            MeshEntityTypeManager::mesh_entity_types() ;
        index_t count_invalid = 0 ;
        for( const MeshEntityType& type : meshed_types ) {
            index_t nb_entities = geomodel.nb_mesh_entities( type ) ;
            for( index_t i = 0; i < nb_entities; ++i ) {
                const GeoModelMeshEntity& E = geomodel.mesh_entity( type, i ) ;
                if( !E.is_connectivity_valid() ) {
                    count_invalid++ ;
                }
            }
        }
        if( count_invalid != 0 ) {
            Logger::warn( "GeoModel", count_invalid,
                " mesh entities of the geomodel have an invalid connectivity." ) ;
        }
        return count_invalid == 0 ;
    }

    bool are_geomodel_geological_entities_valid( const GeoModel& geomodel )
    {
        const std::vector< GeologicalEntityType >& geological_types =
            geomodel.entity_type_manager().geological_entity_manager.geological_entity_types() ;
        index_t count_invalid = 0 ;
        for( const GeologicalEntityType& type : geological_types ) {
            index_t nb_entities = geomodel.nb_geological_entities( type ) ;
            for( index_t i = 0; i < nb_entities; ++i ) {
                const GeoModelGeologicalEntity& E = geomodel.geological_entity( type,
                    i ) ;
                if( !E.is_valid() ) {
                    count_invalid++ ;
                }
            }
        }
        if( count_invalid != 0 ) {
            Logger::warn( "GeoModel", count_invalid,
                " geological entities of the geomodel are invalid " ) ;
        }
        return count_invalid == 0 ;
    }

    bool is_geomodel_valid(
        const GeoModel& geomodel,
        ValidityCheckMode validity_check_mode )
    {
        if( validity_check_mode == GEOMETRY
            && !GEO::CmdLine::get_arg_bool( "in:intersection_check" ) ) {
            validity_check_mode = GEOMETRY_EXCEPT_FACET_INTERSECTION ;
        } else if( validity_check_mode == ALL
            && !GEO::CmdLine::get_arg_bool( "in:intersection_check" ) ) {
            validity_check_mode = ALL_EXCEPT_FACET_INTERSECTION ;
        }

        GeoModelValidityCheck validity_checker( geomodel, validity_check_mode ) ;

        bool valid = validity_checker.is_geomodel_valid() ;

        if( valid ) {
            Logger::out( "GeoModel", "Model ", geomodel.name(), " is valid " ) ;
        } else {
            Logger::warn( "GeoModel", "Model ", geomodel.name(), " is invalid " ) ;
            if( !GEO::CmdLine::get_arg_bool( "validity_save" ) ) {
                Logger::out( "Info", "To save geomodel invalidities in files ",
                    "(.geogram) set \"validity_save\" to true in the command line." ) ;
            }
        }
        return valid ;
    }

} // namespace RINGMesh
