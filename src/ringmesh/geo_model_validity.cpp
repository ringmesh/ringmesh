/*
 * Copyright (c) 2012-2016, Association Scientifique pour la Geologie et ses Applications (ASGA)
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

#include <ringmesh/geo_model_validity.h>

#include <cmath>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <set>

#include <geogram/basic/algorithm.h>
#include <geogram/basic/geometry_nd.h>
#include <geogram/basic/logger.h>
#include <geogram/basic/string.h>
#include <geogram/basic/command_line.h>

#include <geogram/mesh/mesh.h>
#include <geogram/mesh/mesh_AABB.h>
#include <geogram/mesh/mesh_geometry.h>
#include <geogram/mesh/mesh_intersection.h>
#include <geogram/mesh/mesh_io.h>
#include <geogram/mesh/mesh_repair.h>
#include <geogram/mesh/mesh_topology.h>
#include <geogram/mesh/triangle_intersection.h>

#include <geogram/points/colocate.h>

#include <ringmesh/mesh.h>
#include <ringmesh/geometry.h>
#include <ringmesh/geo_model.h>
#include <ringmesh/geo_model_api.h>
#include <ringmesh/geo_model_entity.h>
#include <ringmesh/geo_model_mesh_entity.h>
#include <ringmesh/geo_model_geological_entity.h>
#include <ringmesh/geogram_extension.h>
#include <ringmesh/geogram_mesh_repair.h>

/*!
 * @file ringmesh/geo_model_validity.cpp
 * @brief Implementation of functions to check the validity of GeoModels
 * @author Jeanne Pellerin
 * @todo Refactor the functions - reorganize to have a cleaner code.
 */

namespace {

    using namespace RINGMesh ;
    using GEO::index_t ;
    using GEO::vec3 ;

    typedef GeoModelMeshEntity GMME ;

    /*---------------------------------------------------------------------------*/
    /*----- Some pieces of the code below are copied or modified from -----------*/
    /*----- geogram\mesh\mesh_intersection.cpp-----------------------------------*/
    /*
     *  Copyright (c) 2012-2014, Bruno Levy
     *  All rights reserved.
     *
     *  Redistribution and use in source and binary forms, with or without
     *  modification, are permitted provided that the following conditions are met:
     *
     *  * Redistributions of source code must retain the above copyright notice,
     *  this list of conditions and the following disclaimer.
     *  * Redistributions in binary form must reproduce the above copyright notice,
     *  this list of conditions and the following disclaimer in the documentation
     *  and/or other materials provided with the distribution.
     *  * Neither the name of the ALICE Project-Team nor the names of its
     *  contributors may be used to endorse or promote products derived from this
     *  software without specific prior written permission.
     *
     *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
     *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
     *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
     *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
     *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
     *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
     *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
     *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
     *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
     *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
     *  POSSIBILITY OF SUCH DAMAGE.
     */

    /** \note Copied from geogram
     * \brief Computes the intersection between two triangular facets in
     *  a mesh
     * \param[in] M the mesh
     * \param[in] f1 index of the first facet
     * \param[in] f2 index of the second facet
     * \param[warn] sym symbolic representation of the intersection (if any)
     * \return true if facets \p f1 and \p f2 have an intersection, false
     *  otherwise
     */
    bool triangles_intersect(
        const GEO::Mesh& M,
        index_t f1,
        index_t f2,
        GEO::vector< GEO::TriangleIsect >& sym )
    {
        ringmesh_assert( M.facets.nb_vertices( f1 ) == 3 ) ;
        ringmesh_assert( M.facets.nb_vertices( f2 ) == 3 ) ;
        index_t c1 = M.facets.corners_begin( f1 ) ;
        const vec3& p1 = GEO::Geom::mesh_vertex( M, M.facet_corners.vertex( c1 ) ) ;
        const vec3& p2 = GEO::Geom::mesh_vertex( M,
            M.facet_corners.vertex( c1 + 1 ) ) ;
        const vec3& p3 = GEO::Geom::mesh_vertex( M,
            M.facet_corners.vertex( c1 + 2 ) ) ;

        index_t c2 = M.facets.corners_begin( f2 ) ;
        const vec3& q1 = GEO::Geom::mesh_vertex( M, M.facet_corners.vertex( c2 ) ) ;
        const vec3& q2 = GEO::Geom::mesh_vertex( M,
            M.facet_corners.vertex( c2 + 1 ) ) ;
        const vec3& q3 = GEO::Geom::mesh_vertex( M,
            M.facet_corners.vertex( c2 + 2 ) ) ;
        return triangles_intersections( p1, p2, p3, q1, q2, q3, sym ) ;
    }

    /*!
     * @brief Returns the Line identification if the given points define
     *       an edge of one of the Line of the model
     * @param model The GeoModel to consider
     * @param v0 Index of the first point in the model
     * @param v1 Index of the second point in the model
     */
    GME::gme_t is_edge_on_line( const GeoModel& model, index_t v0, index_t v1 )
    {
        const std::vector< GMEVertex >& v0_bme = model.mesh.vertices.gme_vertices(
            v0 ) ;
        const std::vector< GMEVertex >& v1_bme = model.mesh.vertices.gme_vertices(
            v1 ) ;

        // Get the local indices of the vertices in 
        // a common Line if any 
        GME::gme_t result ;
        index_t lv0 = NO_ID ;
        index_t lv1 = NO_ID ;

        // No sorting to optimize since 
        // v0_bme and v1_bme are very small sets ( < 10 entities ) [JP]
        for( index_t i = 0; i < v0_bme.size(); ++i ) {
            if( v0_bme[i].gme_id.type == Line::type_name_static() ) {
                for( index_t j = 0; j < v1_bme.size(); ++j ) {
                    if( v1_bme[j].gme_id.type == Line::type_name_static()
                        && v0_bme[i].gme_id.index == v1_bme[j].gme_id.index ) {
                        if( lv0 == NO_ID ) {
                            lv0 = v0_bme[i].v_id ;
                            lv1 = v1_bme[j].v_id ;
                            result = v0_bme[i].gme_id ;
                        } else {
                            if( !model.line( result.index ).is_closed() ) {
                                // Most certainly there is a problem (JP)
                                return GME::gme_t() ;
                            }

                        }
                    }
                }
            }
        }
        if( !result.is_defined() ) {
            // The two points are not on the same Line
            return GME::gme_t() ;
        } else {
            // Determine if the points define an edge
            if( lv0 > lv1 ) {
                std::swap( lv0, lv1 ) ;
            }
            // Casts are here to avoid a compiler warning [JP]
//            index_t delta_i = static_cast< int >( lv1 ) - static_cast< int >( lv0 ) ;
            index_t delta_i = lv1 - lv0 ;

            if( delta_i == 1 ) {
                // There is an edge if their indices in the Line are i and i+1
                return result ;
            } else if( model.line( result.index ).is_closed()
                && delta_i == model.line( result.index ).nb_vertices() - 2 ) {
                // If the Line is closed we can also have 0; n-2 or n-1; 1
                return result ;
            } else {
                // The two points are on the same line but
                // do not define an edge
                return GME::gme_t() ;
            }
        }
    }

    /*!
     * @brief Returns the Line identification if the given points define
     *       an edge of one of the Line of the model
     */
    GME::gme_t is_edge_on_line(
        const GeoModel& model,
        const vec3& p0,
        const vec3& p1 )
    {
        // Get the ids in the model of these 2 points
        index_t v0 = model.mesh.vertices.index( p0 ) ;
        index_t v1 = model.mesh.vertices.index( p1 ) ;
        ringmesh_assert( v0 != NO_ID && v1 != NO_ID ) ;

        return is_edge_on_line( model, v0, v1 ) ;
    }

    /*!
     * @brief Returns true if the facets @param f1 and @param f2
     *        of the mesh @param M share an edge
     *        that is on one Line of the boundary model @param BM
     * @pre The mesh M is triangulated
     *
     */
    bool facets_share_line_edge(
        const GEO::Mesh& M,
        const GeoModel& BM,
        index_t f1,
        index_t f2 )
    {
        ringmesh_assert( M.facets.nb_vertices( f1 ) == 3 ) ;
        ringmesh_assert( M.facets.nb_vertices( f2 ) == 3 ) ;

        // I only want to test the edges that are on boundary 
        for( index_t i = 0; i < 3; ++i ) {
            if( M.facets.adjacent( f1, i ) == NO_ID ) {
                for( index_t j = 0; j < 3; ++j ) {
                    if( M.facets.adjacent( f2, j ) == NO_ID ) {
                        const vec3& p10 = M.vertices.point(
                            M.facets.vertex( f1, i ) ) ;
                        const vec3& p11 = M.vertices.point(
                            M.facets.vertex( f1, i == 2 ? 0 : i + 1 ) ) ;

                        const vec3& p20 = M.vertices.point(
                            M.facets.vertex( f2, j ) ) ;
                        const vec3& p21 = M.vertices.point(
                            M.facets.vertex( f2, j == 2 ? 0 : j + 1 ) ) ;

                        index_t v10 = BM.mesh.vertices.index( p10 ) ;
                        index_t v11 = BM.mesh.vertices.index( p11 ) ;
                        ringmesh_assert( v10 != NO_ID && v11 != NO_ID ) ;

                        index_t v20 = BM.mesh.vertices.index( p20 ) ;
                        index_t v21 = BM.mesh.vertices.index( p21 ) ;

                        if( v10 == v20 && v11 == v21
                            && is_edge_on_line( BM, p20, p21 ).is_defined() ) {
                            return true ;
                        }
                        if( v10 == v21 && v11 == v20
                            && is_edge_on_line( BM, p20, p21 ).is_defined() ) {
                            return true ;
                        }
                    }
                }
            }
        }

        return false ;
    }

    /** \note Copied from geogram
     * \brief Tests whether two facets are adjacent
     * \details Two facets are adjacents if they share an edge
     *
     * \param[in] M the mesh
     * \param[in] f1 index of the first facet
     * \param[in] f2 index of the second facet
     * \return true if facets \p f1 and \p f2 share an edge, false
     *  otherwise
     */
    bool facets_are_adjacent( const GEO::Mesh& M, index_t f1, index_t f2 )
    {
        if( f1 == f2 ) {
            return true ;
        }
        for( index_t c = M.facets.corners_begin( f1 );
            c != M.facets.corners_end( f1 ); ++c ) {
            if( M.facet_corners.adjacent_facet( c ) == f2 ) {
                return true ;
            }
        }
        return false ;
    }

    /** \note Modified from geogram
     * \brief Action class for storing intersections when traversing
     *  a AABBTree.
     */
    class StoreIntersections {
    public:
        /**
         * \brief Constructs the StoreIntersections
         * \param[in] M the mesh
         * \param[out] has_isect the flag that indicates for each facet
         *  whether it has intersections
         */
        StoreIntersections(
            const GEO::Mesh& M,
            const GeoModel& BM,
            GEO::vector< index_t >& has_isect )
            : M_( M ), BM_( BM ), has_intersection_( has_isect )
        {
            has_intersection_.assign( M.facets.nb(), 0 ) ;
        }

        /**
         * \brief Determines the intersections between two facets
         * \details It is a callback for AABBTree traversal
         * \param[in] f1 index of the first facet
         * \param[in] f2 index of the second facet
         */
        void operator()( index_t f1, index_t f2 )
        {
            if( f1 != f2 && !facets_are_adjacent( M_, f1, f2 )
                && !facets_share_line_edge( M_, BM_, f1, f2 )
                && triangles_intersect( M_, f1, f2, sym_ ) ) {
                has_intersection_[f1] = 1 ;
                has_intersection_[f2] = 1 ;
            }
        }

    private:
        const GEO::Mesh& M_ ;
        const GeoModel& BM_ ;
        GEO::vector< index_t >& has_intersection_ ;
        GEO::vector< GEO::TriangleIsect > sym_ ;
    } ;

    void save_mesh_locating_geomodel_inconsistencies(
        const GEO::Mesh& mesh,
        const std::ostringstream& file )
    {
        if( GEO::CmdLine::get_arg_bool( "in:validity_save" ) ) {
            GEO::mesh_save( mesh, file.str() ) ;
        }
    }

    /** \note Copied from geogram
     * \brief Detect intersecting facets in a TRIANGULATED mesh
     * \param[in] M the mesh
     * \return number of intersecting facets
     */
    index_t detect_intersecting_facets( const GeoModel& model, GEO::Mesh& M )
    {
        geo_assert( M.vertices.dimension() >= 3 ) ;

        GEO::vector< index_t > has_intersection ;
        StoreIntersections action( M, model, has_intersection ) ;
        GEO::MeshFacetsAABB AABB( M ) ;
        AABB.compute_facet_bbox_intersections( action ) ;

        index_t nb_intersections = static_cast< index_t >( std::count(
            has_intersection.begin(), has_intersection.end(), 1 ) ) ;

        if( nb_intersections > 0 ) {
            GEO::Mesh mesh ;
            for( index_t f = 0; f < has_intersection.size(); f++ ) {
                if( !has_intersection[f] ) continue ;
                GEO::vector< index_t > vertices ;
                vertices.reserve( 3 ) ;
                for( index_t v = 0; v < M.facets.nb_vertices( f ); v++ ) {
                    index_t id = mesh.vertices.create_vertex(
                        M.vertices.point_ptr( M.facets.vertex( f, v ) ) ) ;
                    vertices.push_back( id ) ;
                }
                mesh.facets.create_polygon( vertices ) ;
            }
            std::ostringstream file ;
            file << validity_errors_directory << "/intersected_facets.mesh" ;
            save_mesh_locating_geomodel_inconsistencies( mesh, file ) ;
            Logger::out( "I/O" ) << std::endl ;
        }
        return nb_intersections ;
    }

    /***************************************************************************/

    /*---------------------------------------------------------------------------*/
    /*----- Some pieces of the code below are copied or modified from -----------*/
    /*----- geogram\mesh\mesh_repair.cpp-----------------------------------------*/

    /*!
     * @brief Trigger an assertion if several vertices of a mesh at the same geometric location

     */
    void assert_no_colocate_vertices( const GEO::Mesh& M, double colocate_epsilon )
    {
        if( has_mesh_colocate_vertices( M, colocate_epsilon ) ) {
            geo_assert_not_reached;
        }
    }

    /*!
     * @brief Get the colocated vertices of a mesh, i.e. which have the same geometric location
     * @note Code modified from geogram/mesh/mesh_repair.cpp
     * @param[in] M the mesh
     * @param[in] colocate_epsilon tolerance for merging vertices
     * @param[out] old2new if old2new[i] == i, point is to keep; otherwise
     *             old2new[i] = j, j is the index of the matching point kept
     * @returns true if there are colocated vertices
     *
     * @todo replace by a call to  GEO::mesh_detect_colocated_vertices since it
     *       is now in the API
     *
     * @pre The mesh has no facet, cell or edges.
     */
    bool colocate_vertices(
        GEO::Mesh& M,
        double colocate_epsilon,
        GEO::vector< index_t >& old2new )
    {
        old2new.clear() ;

        if( M.edges.nb() > 0 || M.facets.nb() > 0 || M.cells.nb() > 0 ) {
            // This function is not sufficient to update the complete mesh.
            ringmesh_assert( false ) ;
        }

        index_t nb_new_vertices = 0 ;
        if( colocate_epsilon == 0.0 ) {
            nb_new_vertices = GEO::Geom::colocate_by_lexico_sort(
                M.vertices.point_ptr( 0 ), 3, M.vertices.nb(), old2new,
                M.vertices.dimension() ) ;
        } else {
            nb_new_vertices = GEO::Geom::colocate( M.vertices.point_ptr( 0 ), 3,
                M.vertices.nb(), old2new, colocate_epsilon,
                M.vertices.dimension() ) ;
        }
        return nb_new_vertices != M.vertices.nb() ;
    }

    /*----------------------------------------------------------------------------*/

    /*!
     * @brief Get the GMME defining the boundaries of an entity
     */
    void boundary_gmme(
        const GeoModelMeshEntity& E,
        std::vector< GME::gme_t >& borders,
        bool with_inside_borders )
    {
        borders.clear() ;
        // We are dealing with basic entities
        for( index_t i = 0; i < E.nb_boundaries(); ++i ) {
            if( with_inside_borders
                || ( !with_inside_borders && !E.boundary( i ).is_inside_border( E ) ) ) {
                borders.push_back( E.boundary_gme( i ) ) ;
            }
        }
    }

    /*!
     * @brief Get the entities in the boundary of which @param E is
     * @details For GMME, get the contents of the in_boundary vector
     *          For high level entities, determine in_boundary high level entities
     */
    void in_boundary_gme(
        const GeoModelGeologicalEntity& E,
        std::vector< GME::gme_t >& in_boundary )
    {
        in_boundary.clear() ;

        // We are dealing with high level entities
        // Need to go through the children to get information
        for( index_t i = 0; i < E.nb_children(); ++i ) {
            for( index_t j = 0; j < E.child( i ).nb_in_boundary(); ++j ) {
                in_boundary.push_back(
                    E.child( i ).in_boundary( j ).parent_gme(
                        Layer::type_name_static() ) ) ;
            }
        }
        // Remove duplicates
        GEO::sort_unique( in_boundary ) ;
    }

    /*!
     * @brief Check if entity @param is of the @param model is in the
     *        in_boundary vector of entity @param in.
     */
    bool is_in_in_boundary( const GeoModel& model, GME::gme_t is, GME::gme_t in )
    {
        const GeoModelMeshEntity& E = model.mesh_entity( in ) ;
        for( index_t i = 0; i < E.nb_in_boundary(); ++i ) {
            if( E.in_boundary_gme( i ) == is ) {
                return true ;
            }
        }
        return false ;
    }

    void save_invalid_points(
        const std::ostringstream& file,
        const GeoModel& M,
        const std::vector< bool >& valid )
    {
        GEO::Mesh point_mesh ;
        for( index_t i = 0; i < valid.size(); ++i ) {
            if( !valid[i] ) {
                const vec3& V = M.mesh.vertices.vertex( i ) ;
                point_mesh.vertices.create_vertex( V.data() ) ;
            }
        }
        save_mesh_locating_geomodel_inconsistencies( point_mesh, file ) ;
    }

    /*!
     * @brief Check the geometrical-topological consistency of the model
     * @details Verification is based on the information stored by the unique
     *          vertices of the model which validity must be checked beforehand
     * @todo Check that the model vertices are consistent with the model_vertex_ids
     *       stored at by the GMME
     * @todo Implementation for regions
     * @todo Split in smaller functions
     */
    bool check_model_points_validity( const GeoModel& M )
    {
        // For all the vertices of the model 
        // We check that the entities in which they are are consistent 
        // to have a valid B-Rep model
        std::vector< bool > valid( M.mesh.vertices.nb(), true ) ;
        for( index_t i = 0; i < M.mesh.vertices.nb(); ++i ) {
            bool valid_vertex = true ;

            // Get the mesh entities in which this vertex is            
            index_t corner = NO_ID ;
            std::vector< index_t > lines ;
            std::vector< index_t > surfaces ;
            std::vector< index_t > regions ;

            const std::vector< GMEVertex >& bmes = M.mesh.vertices.gme_vertices(
                i ) ;

            for( index_t j = 0; j < bmes.size(); ++j ) {
                const std::string& T = bmes[j].gme_id.type ;
                index_t id = bmes[j].gme_id.index ;
                if( T == Region::type_name_static() ) {
                    regions.push_back( id ) ;
                } else if( T == Surface::type_name_static() ) {
                    surfaces.push_back( id ) ;
                } else if( T == Line::type_name_static() ) {
                    lines.push_back( id ) ;
                } else if( T == Corner::type_name_static() ) {
                    if( corner != NO_ID ) {
                        Logger::warn( "GeoModel" ) << " Vertex " << i
                            << " is in at least 2 Corners" << std::endl ;
                        valid_vertex = false ;
                    } else {
                        corner = id ;
                    }
                } else {
                    Logger::warn( "GeoModel" ) << " Vertex " << i
                        << " is in no Entity of the Model" << std::endl ;
                    valid_vertex = false ;
                    break ;
                }
            }

            if( valid_vertex ) {
                if( surfaces.empty() ) {
                    if( regions.size() != 1 ) {
                        Logger::warn( "GeoModel" ) << " Vertex " << i
                            << " is in " << regions.size() << " Regions: " ;
                        for( index_t j = 0; j < surfaces.size(); ++j ) {
                            Logger::warn( "GeoModel" ) << regions[j] << " ; " ;
                        }
                        Logger::warn( "GeoModel" ) << std::endl ;
                        valid_vertex = false ;
                    } /// @todo Implement the other conditions for Region point validity
                } else if( corner == NO_ID && lines.empty() ) {
                    // This is a point on one SURFACE and only one
                    if( surfaces.size() != 1 ) {
                        Logger::warn( "GeoModel" ) << " Vertex " << i
                            << " is in " << surfaces.size() << " Surfaces: " ;
                        for( index_t j = 0; j < surfaces.size(); ++j ) {
                            Logger::warn( "GeoModel" ) << surfaces[j] << " ; " ;
                        }
                        Logger::warn( "GeoModel" ) << std::endl ;
                        valid_vertex = false ;
                    }
                } else if( corner == NO_ID && !lines.empty() ) {
                    // This is a point on one LINE 
                    if( lines.size() != 1 ) {
                        Logger::warn( "GeoModel" ) << " Vertex " << i
                            << " is in " << lines.size() << " Lines " ;
                        for( index_t j = 0; j < lines.size(); ++j ) {
                            Logger::warn( "GeoModel" ) << lines[j] << " ; " ;
                        }
                        Logger::warn( "GeoModel" ) << std::endl ;
                        valid_vertex = false ;
                    } else {
                        // This point must also be in at least one SURFACE
                        if( surfaces.empty() ) {
                            Logger::warn( "GeoModel" ) << " Vertex " << i
                                << " is in a Line but in no Surface " << std::endl ;
                            valid_vertex = false ;
                        }
                        // Check that one point is no more than twice in a SURFACE
                        for( index_t k = 0; k < surfaces.size(); ++k ) {
                            index_t nb = static_cast< index_t >( std::count(
                                surfaces.begin(), surfaces.end(), surfaces[k] ) ) ;
                            if( nb > 2 ) {
                                Logger::warn( "GeoModel" ) << " Vertex " << i
                                    << " is " << nb << " times in Surface "
                                    << M.surface( surfaces[k] ).gme_id()
                                    << std::endl ;
                                valid_vertex = false ;
                            } else if( nb == 2 ) {
                                // If a point is twice in a SURFACE, it must be
                                // on an internal boundary Line.
                                bool internal_boundary = false ;
                                for( index_t l = 0; l < lines.size(); ++l ) {
                                    if( M.line( lines[l] ).is_inside_border(
                                        M.surface( surfaces[k] ) ) ) {
                                        internal_boundary = true ;
                                        break ;
                                    }
                                }
                                if( !internal_boundary ) {
                                    Logger::warn( "GeoModel" ) << " Vertex "
                                        << i << " appears " << nb
                                        << " times in Surface "
                                        << M.surface( surfaces[k] ).gme_id()
                                        << std::endl ;
                                    valid_vertex = false ;
                                }
                            }
                        }
                        // Check that all the surfaces are in in_boundary of all
                        // the lines 
                        for( index_t k = 0; k < surfaces.size(); ++k ) {
                            for( index_t l = 0; l < lines.size(); ++l ) {
                                GME::gme_t s_id( Surface::type_name_static(), surfaces[k] ) ;
                                GME::gme_t l_id( Line::type_name_static(), lines[l] ) ;
                                if( !is_in_in_boundary( M, s_id, l_id ) ) {
                                    Logger::warn( "GeoModel" )
                                        << " Inconsistent Line-Surface connectivity "
                                        << " Vertex " << i << " shows that " << s_id
                                        << " must be in the boundary of " << l_id
                                        << std::endl ;
                                    valid_vertex = false ;
                                }
                            }
                        }
                    }
                } else if( corner != NO_ID ) {
                    // This is one point at a CORNER
                    // It must be in at least one LINE
                    if( lines.empty() ) {
                        Logger::warn( "GeoModel" ) << " Vertex " << i
                            << " is at a Corner but in no Line " << std::endl ;
                        valid_vertex = false ;
                    } else {
                        if( lines.size() < 2 ) {
                            Logger::warn( "GeoModel" ) << " Vertex " << i
                                << " is in at a Corner but in one Line only: "
                                << lines[0] << std::endl ;
                            valid_vertex = false ;
                        }
                        // Check that a point is no more than twice in a LINE
                        for( index_t k = 0; k < lines.size(); ++k ) {
                            index_t nb = static_cast< index_t >( std::count(
                                lines.begin(), lines.end(), lines[k] ) ) ;
                            if( nb == 2 ) {
                                // The line must be closed
                                if( !M.line( lines[k] ).is_closed() ) {
                                    Logger::warn( "GeoModel" ) << " Vertex "
                                        << i << " is twice in Line " << lines[k]
                                        << std::endl ;
                                    valid_vertex = false ;
                                }
                            }
                            if( nb > 2 ) {
                                Logger::warn( "GeoModel" ) << " Vertex " << i
                                    << " appears " << nb << " times in Line "
                                    << lines[k] << std::endl ;
                                valid_vertex = false ;
                                break ;
                            }
                        }
                        // Check that all the lines are in in_boundary of this corner
                        for( index_t k = 0; k < lines.size(); ++k ) {
                            GME::gme_t l_id( Line::type_name_static(), lines[k] ) ;
                            GME::gme_t c_id( Corner::type_name_static(), corner ) ;
                            if( !is_in_in_boundary( M, l_id, c_id ) ) {
                                Logger::warn( "GeoModel" )
                                    << " Inconsistent Line-Corner connectivity "
                                    << " vertex " << i << " shows that " << l_id
                                    << " must be in the boundary of " << c_id
                                    << std::endl ;
                                valid_vertex = false ;
                            }
                        }
                    }
                    // It must also be in a least one surface ? perhaps 2
                    if( surfaces.empty() ) {
                        Logger::warn( "GeoModel" ) << " Vertex " << i
                            << " is at a Corner but in no Surface " << std::endl ;
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
            file << validity_errors_directory << "/invalid_global_vertices.mesh" ;
            save_invalid_points( file, M, valid ) ;

            Logger::warn( "GeoModel" ) << nb_invalid << " invalid vertices "
                << std::endl << "Saved in file: " << file.str() << std::endl ;
            return false ;
        } else {
            return true ;
        }
    }

    void save_edges(
        const std::ostringstream& file,
        const GeoModel& M,
        const std::vector< index_t >& e )
    {
        GEO::Mesh edge_mesh ;
        index_t previous_vertex_id = NO_ID ;
        for( index_t i = 0; i < e.size(); ++i ) {
            index_t cur_vertex_id = edge_mesh.vertices.create_vertex(
                M.mesh.vertices.vertex( e[i] ).data() ) ;
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
        for( index_t f = 0; f < facets.size(); ++f ) {
            index_t cur_facet = facets[f] ;
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
     *          of the associated model
     *          The Line boundaries must form a closed manifold line.
     */
    bool surface_boundary_valid( const Surface& S )
    {
        std::vector< index_t > invalid_corners ;
        for( index_t f = 0; f < S.nb_mesh_elements(); ++f ) {
            for( index_t v = 0; v < S.nb_mesh_element_vertices( f ); ++v ) {
                if( S.facet_adjacent_index( f, v ) == NO_ID
                    && !is_edge_on_line( S.model(), S.model_vertex_id( f, v ),
                        S.model_vertex_id( f, S.next_facet_vertex_index( f, v ) ) ).is_defined() ) {
                    invalid_corners.push_back( S.model_vertex_id( f, v ) ) ;
                    invalid_corners.push_back(
                        S.model_vertex_id( f, S.next_facet_vertex_index( f, v ) ) ) ;
                }
            }
        }
        if( !invalid_corners.empty() ) {
            std::ostringstream file ;
            file << validity_errors_directory << "/invalid_boundary_surface_"
                << S.index() << ".mesh" ;
            save_edges( file, S.model(), invalid_corners ) ;

            Logger::warn( "GeoModel" ) << " Invalid surface boundary: "
                << invalid_corners.size() / 2 << " boundary edges of " << S.gme_id()
                << "  are in no line of the model " << std::endl
                << " Saved in file: " << file.str() << std::endl ;
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
        const std::vector< index_t >& edge_vertices )
    {
        std::ostringstream file_name(
            validity_errors_directory + "/non_manifold_edges.mesh" ) ;

        save_edges( file_name, geomodel, edge_vertices ) ;
    }

    bool is_surface_conformal_to_volume(
        const Surface& surface,
        const ColocaterANN& cell_facet_barycenter_ann )
    {
        std::vector< index_t > unconformal_facets ;
        for( index_t f = 0; f < surface.nb_mesh_elements(); f++ ) {
            vec3 center = surface.mesh_element_center( f ) ;
            std::vector< index_t > result ;
            if( !cell_facet_barycenter_ann.get_colocated( center, result ) ) {
                unconformal_facets.push_back( f ) ;
            }
        }
        if( !unconformal_facets.empty() ) {
            std::ostringstream file ;
            file << validity_errors_directory << "/unconformal_surface_"
                << surface.index() << ".mesh" ;
            save_facets( file.str(), surface, unconformal_facets ) ;

            Logger::warn( "GeoModel" ) << " Unconformal surface: "
                << unconformal_facets.size() << " facets of " << surface.gme_id()
                << " are unconformal with the model cells " << std::endl
                << " Saved in file: " << file.str() << std::endl ;
            return false ;
        } else {
            return true ;
        }
    }

    // This cannot be general and is to move where
    // the geological types are defined....
    // To check that their contract is indeed true
    class GeoModelGeologicalValidityChecker {
    public:
        GeoModelGeologicalValidityChecker( const GeoModel& geomodel ) :
            geomodel_(geomodel), valid_(true)
        {}

    private:
        void set_invalid_model()
        {
            valid_ = false ;
        }
        /*!
         * Verify the geological validity if the model has interfaces and layers
         */
        void test_geological_validity()
        {
            if( geomodel_.nb_geological_entities( Interface::type_name_static() ) > 0
                && geomodel_.nb_geological_entities( Layer::type_name_static() ) > 0 ) {
                if( !is_geomodel_geology_valid( geomodel_ ) ) {
                    set_invalid_model() ;
                }
            }
        }
        index_t nb_invalid_geological_entities( const std::string& type ) const 
        {
            index_t nb_invalid = 0 ;
            for( index_t e = 0; e < geomodel_.nb_geological_entities( type ); ++e ) {
                const GeoModelGeologicalEntity& E = geomodel_.geological_entity( type, e ) ;
                if( !E.is_valid() ) {
                    nb_invalid++ ;
                    continue ;
                }
            }
            return nb_invalid ;
        }
        bool are_geomodel_geological_entities_valid() const
        {
            index_t nb_invalid = 0 ;
            const std::vector< std::string >& types =
                geomodel_.entity_type_manager().geological_entity_types() ;
            for( index_t i = 0; i < types.size() ; i++ ) {
                nb_invalid += nb_invalid_geological_entities( types[i] ) ;
            }
            if( nb_invalid != 0 ) {
                Logger::warn( "GeoModel" ) << nb_invalid
                    << " individual geological entities of the model are invalid " << std::endl ;
            }
            return nb_invalid == 0 ;
        }

    private: 
        const GeoModel& geomodel_ ;
        bool valid_ ;
    };

    /*!
     * @brief Implementation class for validity checks on a GeoModel
     */
    class GeoModelValidityCheck {
    public:
        GeoModelValidityCheck(
            const GeoModel& geomodel,
            bool check_surface_intersections )
            :
                geomodel_( geomodel ),
                valid_( true ),
                check_surface_intersections_( check_surface_intersections )
        {
            // Ensure that the model vertices are computed and up-to-date
            // Without that we cannot do anything        
            geomodel_.mesh.vertices.test_and_initialize() ;
            geomodel_.mesh.cells.test_and_initialize() ;
        }

        bool is_geomodel_valid()
        {
            do_check_validity() ;
            return valid_ ;
        }

    private:
        void do_check_validity()
        {
            test_model_entities_validity() ;
            test_finite_extension() ;
            test_geometry_connectivity_consistency() ;
            test_non_manifold_edges() ;
            if( check_surface_intersections_ ) {
                test_facet_intersections() ;
            }
        }
   
        /*! 
         * @brief Verify the validity of all GeoModelEntities
         */
        void test_model_entities_validity()
        {
            if( !are_geomodel_mesh_entities_valid() ) {
                set_invalid_model() ;
            }
        }
        
        /*!
         * @brief Check that the model has a finite extension 
         * @details The boundary of the universe region is a one connected component 
         * manifold closed surface.
         */
        void test_finite_extension()
        {
            if( !geomodel_.universe().is_valid() ) {
                set_invalid_model() ;
            }
        }
        /*!
         * Check geometrical-connectivity consistency
         * @todo Add consistency test for facets on boundary of Regions 
         * @todo Check that all Line segments correspond to a Surface
         *  edge that is on the boundary.
         */
        void test_geometry_connectivity_consistency()
        {
            // Check relationships between GeoModelEntities
            // sharing the same point of the model
            if( !check_model_points_validity( geomodel_ ) ) {
                set_invalid_model() ;
            }
            // Check on that Surface edges are in a Line
            for( index_t i = 0; i < geomodel_.nb_surfaces(); ++i ) {
                if( !surface_boundary_valid( geomodel_.surface( i ) ) ) {
                    set_invalid_model() ;
                }
            }
            if( geomodel_.mesh.cells.nb() > 0 ) {
                // Check the consistency between Surface facets and Region cell facets
                const ColocaterANN& ann = geomodel_.mesh.cells.cell_facet_colocater() ;
                for( index_t i = 0; i < geomodel_.nb_surfaces(); ++i ) {
                    if( !is_surface_conformal_to_volume( geomodel_.surface( i ),
                        ann ) ) {
                        set_invalid_model() ;
                    }
                }
            }
        }
        /*! 
         * @brief Creates a Mesh from the GeoModel and triangulates it
         */
        void create_model_mesh()
        {
            Logger::instance()->set_quiet( true ) ;

            bool connect_facets = false ;
            build_mesh_from_geomodel( geomodel_, triangulated_global_model_mesh_,
                connect_facets ) ;
            GEO::mesh_repair( triangulated_global_model_mesh_,
                GEO::MESH_REPAIR_TRIANGULATE ) ;

            Logger::instance()->set_quiet( false ) ;
        }

        /*!
         * @brief Returns true if there are non-manifold edges that are
         *        not in any Line of the model
         * @note Connect the facets of the global mesh
         * @note This is a quite expensive test.
         */
        void test_non_manifold_edges()
        {
            create_model_mesh() ;
            std::vector< index_t > non_manifold_edges ;
            connect_mesh_facets_except_on_mesh_edges(
                triangulated_global_model_mesh_, non_manifold_edges ) ;

            if( !non_manifold_edges.empty() ) {
                Logger::warn( "GeoModel" ) << non_manifold_edges.size() / 2
                    << "non-manifold edges " << std::endl ;
                debug_save_non_manifold_edges( geomodel_, non_manifold_edges ) ;

                set_invalid_model() ;
            }
        }

        /*!
         * @brief Returns true if there are intersections between facets
         * @details Operates on the global mesh
         * @note This is a very expensive test.
         */
        void test_facet_intersections()
        {
            index_t nb_intersections = detect_intersecting_facets( geomodel_,
                triangulated_global_model_mesh_ ) ;

            if( nb_intersections > 0 ) {
                Logger::warn( "GeoModel" ) << nb_intersections
                    << " facet intersections " << std::endl ;
                set_invalid_model() ;
            }
        }

        void set_invalid_model()
        {
            valid_ = false ;
        }
        
        index_t nb_invalid_mesh_entities( const std::string& type ) const
        {
            index_t nb_invalid = 0 ;
            for( index_t e = 0; e < geomodel_.nb_mesh_entities( type ); ++e ) {
                const GeoModelMeshEntity& E = geomodel_.mesh_entity( type, e ) ;
                if( !E.is_valid() ) {
                    nb_invalid++ ;
                    continue ;
                }
            }
            return nb_invalid ;
        }
        bool are_geomodel_mesh_entities_valid() const
        {
            index_t nb_invalid = 0 ;
            nb_invalid += nb_invalid_mesh_entities( Corner::type_name_static() ) ;
            nb_invalid += nb_invalid_mesh_entities( Line::type_name_static() ) ;
            nb_invalid += nb_invalid_mesh_entities( Surface::type_name_static() ) ;
            nb_invalid += nb_invalid_mesh_entities( Region::type_name_static() ) ;
            if( nb_invalid != 0 ) {
                Logger::warn( "GeoModel" ) << nb_invalid
                    << " individual mesh entities of the model are invalid " << std::endl ;
            }
            return nb_invalid == 0 ;
        }
        
    private:
        const GeoModel& geomodel_ ;
        bool valid_ ;
        bool check_surface_intersections_ ;

        // Global mesh of the GeoModel used for some validity checks
        GEO::Mesh triangulated_global_model_mesh_ ;
    } ;

    

} // anonymous namespace

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

    
    bool is_geomodel_geology_valid( const GeoModel& GM )
    {
        bool valid = true ;
        for( index_t l = 0; l < GM.nb_lines(); ++l ) {
            if( GM.line( l ).nb_in_boundary() == 1 ) {
                const GME& S = GM.line( l ).in_boundary( 0 ) ;
                if( !GME::is_fault( S.geological_feature() ) ) {
                    Logger::warn( "GeoModel" ) << " Invalid free border: "
                        << GM.line( l ).gme_id() << " is in the boundary of Surface "
                        << S.gme_id() << " that is not a FAULT " << std::endl
                        << std::endl ;
                    valid = false ;
                }
            }
        }

        for( index_t i = 0; i < GM.nb_geological_entities( Interface::type_name_static() ); ++i ) {
            std::vector< GME::gme_t > layers ;
            const GeoModelGeologicalEntity& entity = GM.geological_entity(
                Interface::type_name_static(), i ) ;
            in_boundary_gme( entity, layers ) ;
            if( layers.empty() ) {
                Logger::warn( "GeoModel" ) << " Invalid interface: "
                    << entity.gme_id()
                    << " is in the boundary of no Layer " << std::endl ;
                valid = false ;
            }
            if( entity.geological_feature() == GME::STRATI
                && layers.size() > 2 ) {
                Logger::warn( "GeoModel" ) << " Invalid horizon: "
                    << entity.gme_id() << " is in the boundary of "
                    << layers.size() << " Layers: " ;
                for( index_t j = 0; j < layers.size(); ++j ) {
                    Logger::warn( "GeoModel" ) << layers[ j ] << " ; " ;
                }
                Logger::warn( "GeoModel" ) << std::endl ;
                valid = false ;
            }
        }
        return valid ;
    }
 
    bool is_geomodel_valid( const GeoModel& GM )
    {
        GeoModelValidityCheck validity_checker( GM,
            GEO::CmdLine::get_arg_bool( "in:intersection_check" ) ) ;

        bool valid = validity_checker.is_geomodel_valid() ;

        if( valid ) {
            Logger::out( "GeoModel" ) << "Model " << GM.name() << " is valid "
                << std::endl << std::endl ;
        } else {
            Logger::warn( "GeoModel" ) << "Model " << GM.name()
                << " is invalid " << std::endl << std::endl ;
        }
        return valid ;
    }
 

} // namespace RINGMesh
