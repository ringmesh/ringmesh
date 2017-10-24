/*
 * Copyright (c) 2012-2017, Association Scientifique pour la Geologie et ses
 * Applications (ASGA). All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL ASGA BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *     http://www.ring-team.org
 *
 *     RING Project
 *     Ecole Nationale Superieure de Geologie - GeoRessources
 *     2 Rue du Doyen Marcel Roubault - TSA 70605
 *     54518 VANDOEUVRE-LES-NANCY
 *     FRANCE
 */

/*! \author Francois Bonneau */

#include <ringmesh/mesh/mesh.h>

#include <numeric>
#include <ringmesh/basic/algorithm.h>
#include <ringmesh/basic/geometry.h>
#include <stack>

#include <ringmesh/mesh/geogram_mesh.h>
#include <ringmesh/mesh/mesh_index.h>

namespace RINGMesh
{
    template < index_t DIMENSION >
    std::unique_ptr< PointSetMesh< DIMENSION > >
        PointSetMesh< DIMENSION >::create_mesh( const MeshType type )
    {
        auto new_type = type;
        if( new_type.empty() )
        {
            new_type = GeogramPointSetMesh< DIMENSION >::type_name_static();
        }
        auto mesh = PointSetMeshFactory< DIMENSION >::create( new_type );
        if( !mesh )
        {
            Logger::warn( "PointSetMesh",
                "Could not create mesh data structure: ", new_type );
            Logger::warn( "PointSetMesh",
                "Falling back to GeogramPointSetMesh data structure" );

            mesh.reset( new GeogramPointSetMesh< DIMENSION > );
        }
        return mesh;
    }

    template < index_t DIMENSION >
    std::tuple< index_t, std::vector< index_t > >
        PointSetMesh< DIMENSION >::connected_components() const
    {
        const auto nb_compoments = this->nb_vertices();
        std::vector< index_t > components( nb_compoments );
        std::iota( components.begin(), components.end(), 0 );
        return std::make_tuple( nb_compoments, components );
    }

    template < index_t DIMENSION >
    std::unique_ptr< LineMesh< DIMENSION > > LineMesh< DIMENSION >::create_mesh(
        const MeshType type )
    {
        MeshType new_type = type;
        if( new_type.empty() )
        {
            new_type = GeogramLineMesh< DIMENSION >::type_name_static();
        }
        auto mesh = LineMeshFactory< DIMENSION >::create( new_type );
        if( !mesh )
        {
            Logger::warn( "LineMesh", "Could not create mesh data structure: ",
                new_type );
            Logger::warn(
                "LineMesh", "Falling back to GeogramLineMesh data structure" );

            mesh.reset( new GeogramLineMesh< DIMENSION > );
        }
        return mesh;
    }

    template < index_t DIMENSION >
    double LineMesh< DIMENSION >::edge_length( index_t edge_id ) const
    {
        const auto& e0 = this->vertex( edge_vertex( { edge_id, 0 } ) );
        const auto& e1 = this->vertex( edge_vertex( { edge_id, 1 } ) );
        return ( e1 - e0 ).length();
    }

    template < index_t DIMENSION >
    vecn< DIMENSION > LineMesh< DIMENSION >::edge_barycenter( index_t edge_id ) const
    {
        const auto& e0 = this->vertex( edge_vertex( { edge_id, 0 } ) );
        const auto& e1 = this->vertex( edge_vertex( { edge_id, 1 } ) );
        return ( e1 + e0 ) / 2.;
    }

    template < index_t DIMENSION >
    bool LineMesh< DIMENSION >::is_mesh_valid() const
    {
        bool valid{ true };

        if( this->nb_vertices() < 2 )
        {
            Logger::err( "LineMesh", "Mesh has less than 2 vertices " );
            valid = false;
        }

        if( nb_edges() == 0 )
        {
            Logger::err( "LineMesh", "Mesh has no edge" );
            valid = false;
        }

        // No isolated vertices
        std::vector< index_t > nb( this->nb_vertices(), 0 );
        for( auto p : range( nb_edges() ) )
        {
            for( auto v : range( 2 ) )
            {
                nb[edge_vertex( { p, v } )]++;
            }
        }
        auto nb_isolated_vertices =
            static_cast< index_t >( std::count( nb.begin(), nb.end(), 0 ) );
        if( nb_isolated_vertices > 0 )
        {
            Logger::warn( "LineMesh", "Mesh has ", nb_isolated_vertices,
                " isolated vertices " );
            valid = false;
        }

        return valid;
    }

    template < index_t DIMENSION >
    std::tuple< index_t, std::vector< index_t > >
        LineMesh< DIMENSION >::connected_components() const
    {
        std::vector< index_t > components( nb_edges(), NO_ID );
        std::vector< index_t > vertex_components( this->nb_vertices(), NO_ID );
        index_t nb_components{ 0 };

        for( auto edge : range( nb_edges() ) )
        {
            ringmesh_assert( components[edge] == NO_ID );
            const auto v0 = edge_vertex( { edge, 0 } );
            const auto v1 = edge_vertex( { edge, 1 } );
            if( vertex_components[v0] == NO_ID
                && vertex_components[v1] == NO_ID )
            {
                vertex_components[v0] = nb_components;
                vertex_components[v1] = nb_components;
                components[edge] = nb_components;
                ++nb_components;
            }
            else if( vertex_components[v0] != NO_ID
                     && vertex_components[v1] == NO_ID )
            {
                vertex_components[v1] = vertex_components[v0];
                components[edge] = vertex_components[v0];
            }
            else if( vertex_components[v0] == NO_ID
                     && vertex_components[v1] != NO_ID )
            {
                vertex_components[v0] = vertex_components[v1];
                components[edge] = vertex_components[v1];
            }
            else
            {
                // Case both nodes have already a connected component.
                if( vertex_components[v0] == vertex_components[v1] )
                {
                    components[edge] = vertex_components[v0];
                }
                else
                {
                    // It appears that 2 previously identified connected
                    // components
                    // correspond in fact to a unique connected component.
                    auto min_connected_components = std::min(
                        vertex_components[v0], vertex_components[v1] );
                    auto max_connected_components = std::max(
                        vertex_components[v0], vertex_components[v1] );
                    ringmesh_assert( min_connected_components != NO_ID );
                    ringmesh_assert( max_connected_components != NO_ID );
                    for( auto previous_edge : range( edge ) )
                    {
                        ringmesh_assert( components[previous_edge] != NO_ID );
                        ringmesh_assert( vertex_components[edge_vertex(
                                             { previous_edge, 0 } )]
                                         != NO_ID );
                        ringmesh_assert( vertex_components[edge_vertex(
                                             { previous_edge, 1 } )]
                                         != NO_ID );
                        if( components[previous_edge]
                            == max_connected_components )
                        {
                            components[previous_edge] =
                                min_connected_components;
                            vertex_components[edge_vertex( { previous_edge,
                                0 } )] = min_connected_components;
                            vertex_components[edge_vertex( { previous_edge,
                                1 } )] = min_connected_components;
                        }
                        else if( components[previous_edge]
                                 > max_connected_components )
                        {
                            ringmesh_assert(
                                components[previous_edge] - 1 >= 0 );
                            ringmesh_assert( vertex_components[edge_vertex(
                                                 { previous_edge, 0 } )]
                                                 - 1
                                             >= 0 );
                            ringmesh_assert( vertex_components[edge_vertex(
                                                 { previous_edge, 1 } )]
                                                 - 1
                                             >= 0 );
                            --components[previous_edge];
                            vertex_components[edge_vertex( { previous_edge,
                                0 } )] = components[previous_edge];
                            vertex_components[edge_vertex( { previous_edge,
                                1 } )] = components[previous_edge];
                            ringmesh_assert(
                                components[previous_edge] != NO_ID );
                            ringmesh_assert( vertex_components[edge_vertex(
                                                 { previous_edge, 0 } )]
                                             != NO_ID );
                            ringmesh_assert( vertex_components[edge_vertex(
                                                 { previous_edge, 1 } )]
                                             != NO_ID );
                            ringmesh_assert( components[previous_edge]
                                             == vertex_components[edge_vertex(
                                                    { previous_edge, 0 } )] );
                            ringmesh_assert( components[previous_edge]
                                             == vertex_components[edge_vertex(
                                                    { previous_edge, 1 } )] );
                        }
                    }
                    components[edge] = min_connected_components;
                    vertex_components[v0] = min_connected_components;
                    vertex_components[v1] = min_connected_components;
                    --nb_components;
                }
            }
            ringmesh_assert( components[edge] != NO_ID );
            ringmesh_assert( vertex_components[v0] != NO_ID );
            ringmesh_assert( vertex_components[v1] != NO_ID );
            ringmesh_assert( components[edge] == vertex_components[v0] );
            ringmesh_assert( components[edge] == vertex_components[v1] );
        }

        return std::make_tuple( nb_components, components );
    }

    template < index_t DIMENSION >
    std::unique_ptr< SurfaceMesh< DIMENSION > >
        SurfaceMeshBase< DIMENSION >::create_mesh( const MeshType type )
    {
        MeshType new_type = type;
        if( new_type.empty() )
        {
            new_type = GeogramSurfaceMesh< DIMENSION >::type_name_static();
        }
        auto mesh = SurfaceMeshFactory< DIMENSION >::create( new_type );
        if( !mesh )
        {
            Logger::warn( "SurfaceMesh",
                "Could not create mesh data structure: ", new_type );
            Logger::warn( "SurfaceMesh",
                "Falling back to GeogramSurfaceMesh data structure" );

            mesh.reset( new GeogramSurfaceMesh< DIMENSION > );
        }
        return mesh;
    }

    template < index_t DIMENSION >
    ElementLocalVertex SurfaceMeshBase< DIMENSION >::next_polygon_vertex(
        const ElementLocalVertex& polygon_local_vertex ) const
    {
        const index_t local_vertex_id =
            polygon_local_vertex.local_vertex_id;
        ringmesh_assert(
            local_vertex_id
            < nb_polygon_vertices( polygon_local_vertex.element_id ) );
        if( local_vertex_id
            != nb_polygon_vertices( polygon_local_vertex.element_id ) - 1 )
        {
            return { polygon_local_vertex.element_id,
                local_vertex_id + 1 };
        }
        return { polygon_local_vertex.element_id, 0 };
    }

    template < index_t DIMENSION >
    ElementLocalVertex SurfaceMeshBase< DIMENSION >::prev_polygon_vertex(
        const ElementLocalVertex& polygon_local_vertex ) const
    {
        ringmesh_assert(
            polygon_local_vertex.local_vertex_id
            < nb_polygon_vertices( polygon_local_vertex.element_id ) );
        if( polygon_local_vertex.local_vertex_id > 0 )
        {
            return { polygon_local_vertex.element_id,
                polygon_local_vertex.local_vertex_id - 1 };
        }
        return { polygon_local_vertex.element_id,
            nb_polygon_vertices( polygon_local_vertex.element_id ) - 1 };
    }

    template < index_t DIMENSION >
    index_t SurfaceMeshBase< DIMENSION >::polygon_edge_vertex(
        const PolygonLocalEdge& polygon_local_edge,
        index_t vertex_id ) const
    {
        ringmesh_assert( vertex_id < 2 );
        if( vertex_id == 0 )
        {
            return polygon_vertex( polygon_local_edge );
        }
        return polygon_vertex( {
            polygon_local_edge.polygon_id,
            ( polygon_local_edge.local_edge_id + vertex_id )
                % nb_polygon_vertices( polygon_local_edge.polygon_id ) } );
    }

    template < index_t DIMENSION >
    vecn< DIMENSION > SurfaceMeshBase< DIMENSION >::polygon_barycenter( index_t polygon_id ) const
    {
        vecn< DIMENSION > result;
        ringmesh_assert( nb_polygon_vertices( polygon_id ) >= 1 );
        for( auto v : range( nb_polygon_vertices( polygon_id ) ) )
        {
            result += this->vertex( polygon_vertex( { polygon_id, v } ) );
        }
        return ( 1.0 / nb_polygon_vertices( polygon_id ) ) * result;
    }

    template < index_t DIMENSION >
    bool SurfaceMeshBase< DIMENSION >::is_polygon_on_border( index_t polygon_index ) const
    {
        for( auto v : range( nb_polygon_vertices( polygon_index ) ) )
        {
            if( is_edge_on_border( { polygon_index, v } ) )
            {
                return true;
            }
        }
        return false;
    }

    template < index_t DIMENSION >
    double SurfaceMeshBase< DIMENSION >::polygon_edge_length(
        const PolygonLocalEdge& polygon_local_edge ) const
    {
        const auto& e0 =
            this->vertex( polygon_edge_vertex( polygon_local_edge, 0 ) );
        const auto& e1 =
            this->vertex( polygon_edge_vertex( polygon_local_edge, 1 ) );
        return ( e1 - e0 ).length();
    }

    template < index_t DIMENSION >
    vecn< DIMENSION > SurfaceMeshBase< DIMENSION >::polygon_edge_barycenter(
        const PolygonLocalEdge& polygon_local_edge ) const
    {
        const auto& e0 =
            this->vertex( polygon_edge_vertex( polygon_local_edge, 0 ) );
        const auto& e1 =
            this->vertex( polygon_edge_vertex( polygon_local_edge, 1 ) );
        return ( e1 + e0 ) / 2.;
    }

    template < index_t DIMENSION >
    PolygonLocalEdge SurfaceMeshBase< DIMENSION >::next_on_border(
        const PolygonLocalEdge& polygon_local_edge ) const
    {
        ringmesh_assert(
            polygon_local_edge.local_edge_id
            < nb_polygon_vertices( polygon_local_edge.polygon_id ) );
        ringmesh_assert( is_edge_on_border( polygon_local_edge ) );

        // Global indices in the surfaces
        auto next_v_id =
            polygon_vertex( next_polygon_vertex( polygon_local_edge ) );

        // Get the polygons around the shared vertex (next_v_id) that are on the
        // boundary
        // There must be one (the current one) or two (the next one on boundary)
        auto polygons_around_next_v_id = polygons_around_vertex(
            next_v_id, true, polygon_local_edge.polygon_id );
        auto nb_around =
            static_cast< index_t >( polygons_around_next_v_id.size() );
        ringmesh_assert( nb_around == 1 || nb_around == 2 );

        PolygonLocalEdge next_polygon_local_edge{ NO_ID, NO_ID };
        auto& next_p = next_polygon_local_edge.polygon_id;
        next_p = polygons_around_next_v_id[0];
        auto& next_e = next_polygon_local_edge.local_edge_id;

        if( nb_around == 2 )
        {
            if( next_p == polygon_local_edge.polygon_id )
            {
                next_p = polygons_around_next_v_id[1];
            }
            ringmesh_assert( next_p != NO_ID );
            ringmesh_assert( is_polygon_on_border( next_p ) );

            // Local index of next vertex in the next polygon
            next_e = vertex_index_in_polygon( next_p, next_v_id );
            ringmesh_assert( is_edge_on_border( next_polygon_local_edge ) );
        }
        else if( nb_around == 1 )
        {
            // next_v_id must be in two border edges of polygon p
            next_e = vertex_index_in_polygon( next_p, next_v_id );
            ringmesh_assert( is_edge_on_border( next_polygon_local_edge ) );
        }

        return next_polygon_local_edge;
    }

    template < index_t DIMENSION >
    PolygonLocalEdge SurfaceMeshBase< DIMENSION >::prev_on_border(
        const PolygonLocalEdge& polygon_local_edge ) const
    {
        ringmesh_assert(
            polygon_local_edge.local_edge_id
            < nb_polygon_vertices( polygon_local_edge.polygon_id ) );
        ringmesh_assert( is_edge_on_border( polygon_local_edge ) );

        // Global indices in the surfaces
        auto v_id = polygon_vertex( { polygon_local_edge } );

        // Get the polygons around the shared vertex (v_id) that are on the
        // boundary
        // There must be one (the current one) or two (the next one on boundary)
        auto polygons_around_v_id = polygons_around_vertex(
            v_id, true, polygon_local_edge.polygon_id );
        auto nb_around = static_cast< index_t >( polygons_around_v_id.size() );
        ringmesh_assert( nb_around == 1 || nb_around == 2 );

        PolygonLocalEdge prev_polygon_local_edge{ NO_ID, NO_ID };
        auto& prev_p = prev_polygon_local_edge.polygon_id;
        prev_p = polygons_around_v_id[0];
        auto& prev_e = prev_polygon_local_edge.local_edge_id;

        if( nb_around == 2 )
        {
            if( prev_p == polygon_local_edge.polygon_id )
            {
                prev_p = polygons_around_v_id[1];
            }
            ringmesh_assert( prev_p != NO_ID );
            ringmesh_assert( is_polygon_on_border( prev_p ) );

            // Local index of given vertex in the prev polygon
            auto v_in_prev_f = vertex_index_in_polygon( prev_p, v_id );
            // Local index of previous vertex in the prev polygon
            prev_e =
                prev_polygon_vertex( { prev_p, v_in_prev_f } ).local_vertex_id;
            ringmesh_assert( is_edge_on_border( prev_polygon_local_edge ) );
        }
        else if( nb_around == 1 )
        {
            // v_id must be in two border edges of polygon p
            auto v_in_next_polygon = vertex_index_in_polygon( prev_p, v_id );
            prev_e = prev_polygon_vertex( { prev_p, v_in_next_polygon } )
                         .local_vertex_id;
            ringmesh_assert( is_edge_on_border( prev_polygon_local_edge ) );
        }

        return prev_polygon_local_edge;
    }

    template < index_t DIMENSION >
    index_t SurfaceMeshBase< DIMENSION >::polygon_from_vertex_ids(
        index_t in0, index_t in1 ) const
    {
        ringmesh_assert(
            in0 < this->nb_vertices() && in1 < this->nb_vertices() );

        // Another possible, probably faster, algorithm is to check if the 2
        // indices
        // are neighbors in polygons_ and check that they are in the same
        // polygon

        // Check if the edge is in one of the polygon
        for( auto poly : range( nb_polygons() ) )
        {
            bool found = false;
            auto prev =
                polygon_vertex( { poly, nb_polygon_vertices( poly ) - 1 } );
            for( auto v : range( nb_polygon_vertices( poly ) ) )
            {
                auto p = polygon_vertex( { poly, v } );
                if( ( prev == in0 && p == in1 ) || ( prev == in1 && p == in0 ) )
                {
                    found = true;
                    break;
                }
                prev = p;
            }
            if( found )
            {
                return poly;
            }
        }
        return NO_ID;
    }

    template < index_t DIMENSION >
    index_t SurfaceMeshBase< DIMENSION >::vertex_index_in_polygon(
        index_t polygon_index, index_t vertex_id ) const
    {
        ringmesh_assert( polygon_index < nb_polygons() );
        for( auto v : range( nb_polygon_vertices( polygon_index ) ) )
        {
            if( polygon_vertex( { polygon_index, v } ) == vertex_id )
            {
                return v;
            }
        }
        return NO_ID;
    }

    template < index_t DIMENSION >
    index_t SurfaceMeshBase< DIMENSION >::closest_vertex_in_polygon(
        index_t polygon_index, const vecn< DIMENSION >& query_point ) const
    {
        index_t result{ 0 };
        double dist{ DBL_MAX };
        for( auto v_id : range( nb_polygon_vertices( polygon_index ) ) )
        {
            double distance = length2(
                query_point
                - this->vertex(
                      polygon_vertex( ElementLocalVertex( polygon_index, v_id ) ) ) );
            if( dist > distance )
            {
                dist = distance;
                result = v_id;
            }
        }
        return result;
    }

    template < index_t DIMENSION >
    std::vector< index_t > SurfaceMeshBase< DIMENSION >::polygons_around_vertex(
        index_t vertex_id, bool border_only, index_t first_polygon ) const
    {
        index_t cur_p{ 0 };
        while( first_polygon == NO_ID && cur_p < nb_polygons() )
        {
            for( auto lv : range( nb_polygon_vertices( cur_p ) ) )
            {
                if( polygon_vertex( { cur_p, lv } ) == vertex_id )
                {
                    first_polygon = cur_p;
                    break;
                }
            }
            cur_p++;
        }
        ringmesh_assert( first_polygon != NO_ID );

        // Flag the visited polygons
        std::vector< index_t > visited;
        visited.reserve( 10 );

        // Stack of the adjacent polygons
        std::stack< index_t > S;
        S.push( first_polygon );
        visited.push_back( first_polygon );

        std::vector< index_t > result;
        result.reserve( 10 );
        do
        {
            auto p = S.top();
            S.pop();

            for( auto v : range( nb_polygon_vertices( p ) ) )
            {
                if( polygon_vertex( { p, v } ) == vertex_id )
                {
                    auto adj_P = polygon_adjacent( { p, v } );
                    auto prev =
                        prev_polygon_vertex( { p, v } ).local_vertex_id;
                    auto adj_prev = polygon_adjacent( { p, prev } );

                    if( adj_P != NO_ID )
                    {
                        // The edge starting at P is not on the boundary
                        if( !contains( visited, adj_P ) )
                        {
                            S.push( adj_P );
                            visited.push_back( adj_P );
                        }
                    }
                    if( adj_prev != NO_ID )
                    {
                        // The edge ending at P is not on the boundary
                        if( !contains( visited, adj_prev ) )
                        {
                            S.push( adj_prev );
                            visited.push_back( adj_prev );
                        }
                    }

                    if( border_only )
                    {
                        if( adj_P == NO_ID || adj_prev == NO_ID )
                        {
                            result.push_back( p );
                        }
                    }
                    else
                    {
                        result.push_back( p );
                    }

                    // We are done with this polygon
                    break;
                }
            }
        } while( !S.empty() );

        return result;
    }

    double SurfaceMesh< 3 >::polygon_area( index_t polygon_id ) const
    {
        double result = 0.0;
        if( nb_polygon_vertices( polygon_id ) == 0 )
        {
            return result;
        }
        const vec3& p1 =
            vertex( polygon_vertex( ElementLocalVertex( polygon_id, 0 ) ) );
        for( auto i : range( 1, nb_polygon_vertices( polygon_id ) - 1 ) )
        {
            const vec3& p2 =
                vertex( polygon_vertex( ElementLocalVertex( polygon_id, i ) ) );
            const vec3& p3 = vertex(
                polygon_vertex( ElementLocalVertex( polygon_id, i + 1 ) ) );
            result += triangle_signed_area(
                p1, p2, p3, polygon_normal( polygon_id ) );
        }
        return std::fabs( result );
    }

    vec3 SurfaceMesh< 3 >::normal_at_vertex( index_t vertex_id, index_t p0 ) const
    {
        ringmesh_assert( vertex_id < nb_vertices() );
        index_t p = 0;
        while( p0 == NO_ID && p < nb_polygons() )
        {
            for( auto lv : range( nb_polygon_vertices( p ) ) )
            {
                if( polygon_vertex( { p, lv } ) == vertex_id )
                {
                    p0 = p;
                    break;
                }
            }
            p++;
        }

        auto polygon_ids = polygons_around_vertex( vertex_id, false, p0 );
        vec3 norm;
        for( auto polygon_id : polygon_ids )
        {
            norm += polygon_normal( polygon_id );
        }
        return normalize( norm );
    }

    vec3 SurfaceMesh< 3 >::polygon_normal( index_t polygon_id ) const
    {
        const auto& p1 = this->vertex(
            this->polygon_vertex( { polygon_id, 0 } ) );
        const auto& p2 = this->vertex(
            this->polygon_vertex( { polygon_id, 1 } ) );
        const auto& p3 = this->vertex(
            this->polygon_vertex( { polygon_id, 2 } ) );
        auto norm = cross( p2 - p1, p3 - p1 );
        return normalize( norm );
    }

    double SurfaceMesh< 2 >::polygon_area( index_t polygon_id ) const
    {
        double result{ 0 };
        if( nb_polygon_vertices( polygon_id ) == 0 )
        {
            return result;
        }
        const auto& p1 = vertex( polygon_vertex( { polygon_id, 0 } ) );
        for( auto i : range( 1, nb_polygon_vertices( polygon_id ) - 1 ) )
        {
            const auto& p2 = vertex( polygon_vertex( { polygon_id, i } ) );
            const auto& p3 = vertex( polygon_vertex( { polygon_id, i + 1 } ) );
            result += GEO::Geom::triangle_signed_area( p1, p2, p3 );
        }
        return std::fabs( result );
    }

    template < index_t DIMENSION >
    std::tuple< index_t, std::vector< index_t > >
        SurfaceMeshBase< DIMENSION >::connected_components() const
    {
        std::vector< index_t > components( nb_polygons(), NO_ID );
        index_t nb_components{ 0 };
        for( auto polygon : range( nb_polygons() ) )
        {
            if( components[polygon] == NO_ID )
            {
                std::stack< index_t > S;
                S.push( polygon );
                components[polygon] = nb_components;
                do
                {
                    auto cur_polygon = S.top();
                    S.pop();
                    for( auto edge :
                        range( nb_polygon_vertices( cur_polygon ) ) )
                    {
                        auto adj_polygon =
                            polygon_adjacent( { cur_polygon, edge } );
                        if( adj_polygon != NO_ID
                            && components[adj_polygon] == NO_ID )
                        {
                            S.push( adj_polygon );
                            components[adj_polygon] = nb_components;
                        }
                    }
                } while( !S.empty() );
                nb_components++;
            }
        }
        return std::make_tuple( nb_components, components );
    }

    template < index_t DIMENSION >
    bool SurfaceMeshBase< DIMENSION >::is_mesh_valid() const
    {
        bool valid{ true };

        if( this->nb_vertices() < 3 )
        {
            Logger::warn( "SurfaceMesh has less than 3 vertices " );
            valid = false;
        }
        if( nb_polygons() == 0 )
        {
            Logger::warn( "SurfaceMesh has no polygon" );
            valid = false;
        }

        // No isolated vertices
        std::vector< index_t > nb( this->nb_vertices(), 0 );
        for( auto p : range( nb_polygons() ) )
        {
            for( auto v : range( nb_polygon_vertices( p ) ) )
            {
                nb[polygon_vertex( { p, v } )]++;
            }
        }
        auto nb_isolated_vertices =
            static_cast< index_t >( std::count( nb.begin(), nb.end(), 0 ) );
        if( nb_isolated_vertices > 0 )
        {
            Logger::warn( "SurfaceMesh", "Mesh has ", nb_isolated_vertices,
                " isolated vertices " );
            valid = false;
        }

        return valid;
    }

    template < index_t DIMENSION >
    std::unique_ptr< VolumeMesh< DIMENSION > >
        VolumeMesh< DIMENSION >::create_mesh( const MeshType type )
    {
        auto new_type = type;
        if( new_type.empty() )
        {
            new_type = GeogramVolumeMesh< DIMENSION >::type_name_static();
        }
        auto mesh = VolumeMeshFactory< DIMENSION >::create( new_type );
        if( !mesh )
        {
            Logger::warn( "VolumeMesh",
                "Could not create mesh data structure: ", new_type );
            Logger::warn( "VolumeMesh",
                "Falling back to GeogramVolumeMesh data structure" );

            mesh.reset( new GeogramVolumeMesh< DIMENSION > );
        }
        return mesh;
    }

    template< index_t DIMENSION >
    double VolumeMesh< DIMENSION >::cell_edge_length(
        index_t cell_id,
        index_t edge_id ) const
    {
        const auto& e0 =
            this->vertex( cell_edge_vertex( cell_id, edge_id, 0 ) );
        const auto& e1 =
            this->vertex( cell_edge_vertex( cell_id, edge_id, 1 ) );
        return ( e1 - e0 ).length();
    }

    template< index_t DIMENSION >
    vecn< DIMENSION > VolumeMesh< DIMENSION >::cell_edge_barycenter(
        index_t cell_id, index_t edge_id ) const
    {
        const auto& e0 =
            this->vertex( cell_edge_vertex( cell_id, edge_id, 0 ) );
        const auto& e1 =
            this->vertex( cell_edge_vertex( cell_id, edge_id, 1 ) );
        return ( e1 + e0 ) / 2.;
    }

    template< index_t DIMENSION >
    vecn< DIMENSION > VolumeMesh< DIMENSION >::cell_facet_barycenter(
        const CellLocalFacet& cell_local_facet ) const
    {
        vecn< DIMENSION > result;
        index_t nb_vertices = nb_cell_facet_vertices( cell_local_facet );
        for( auto v : range( nb_vertices ) )
        {
            result +=
                this->vertex( cell_facet_vertex( cell_local_facet, v ) );
        }
        ringmesh_assert( nb_vertices > 0 );

        return result / static_cast< double >( nb_vertices );
    }

    template< index_t DIMENSION >
    vecn< DIMENSION > VolumeMesh< DIMENSION >::cell_barycenter( index_t cell_id ) const
    {
        vecn< DIMENSION > result;
        ringmesh_assert( nb_cell_vertices( cell_id ) >= 1 );
        for( auto v : range( nb_cell_vertices( cell_id ) ) )
        {
            result += this->vertex( cell_vertex( { cell_id, v } ) );
        }
        return ( 1.0 / nb_cell_vertices( cell_id ) ) * result;
    }

    template< index_t DIMENSION >
    vecn< DIMENSION > VolumeMesh< DIMENSION >::cell_facet_normal(
        const CellLocalFacet& cell_local_facet ) const
    {
        ringmesh_assert( cell_local_facet.cell_id < nb_cells() );
        ringmesh_assert( cell_local_facet.local_facet_id
                         < nb_cell_facets( cell_local_facet.cell_id ) );

        const auto& p1 =
            this->vertex( cell_facet_vertex( cell_local_facet, 0 ) );
        const auto& p2 =
            this->vertex( cell_facet_vertex( cell_local_facet, 1 ) );
        const auto& p3 =
            this->vertex( cell_facet_vertex( cell_local_facet, 2 ) );

        return cross( p2 - p1, p3 - p1 );
    }

    template< index_t DIMENSION >
    index_t VolumeMesh< DIMENSION >::find_cell_corner(
        index_t cell_id,
        index_t vertex_id ) const
    {
        for( auto v : range( nb_cell_vertices( cell_id ) ) )
        {
            if( cell_vertex( { cell_id, v } ) == vertex_id )
            {
                return v;
            }
        }
        return NO_ID;
    }

    template< index_t DIMENSION >
    const NNSearch< DIMENSION >& VolumeMesh< DIMENSION >::cell_facet_nn_search() const
    {
        if( !cell_facet_nn_search_ )
        {
            std::vector< vecn< DIMENSION > > cell_facet_centers(
                nb_cell_facets() );
            index_t cf = 0;
            for( auto c : range( nb_cells() ) )
            {
                for( auto f : range( nb_cell_facets( c ) ) )
                {
                    cell_facet_centers[cf] =
                        cell_facet_barycenter( { c, f } );
                    ++cf;
                }
            }
            cell_facet_nn_search_.reset(
                new NNSearch< DIMENSION >( cell_facet_centers, true ) );
        }
        return *cell_facet_nn_search_.get();
    }

    template < index_t DIMENSION >
    std::tuple< index_t, std::vector< index_t > >
        VolumeMesh< DIMENSION >::connected_components() const
    {
        std::vector< index_t > components( nb_cells(), NO_ID );
        index_t nb_components{ 0 };
        for( auto cell : range( nb_cells() ) )
        {
            if( components[cell] == NO_ID )
            {
                std::stack< index_t > S;
                S.push( cell );
                components[cell] = nb_components;
                do
                {
                    auto cur_cell = S.top();
                    S.pop();
                    for( auto facet : range( nb_cell_facets( cur_cell ) ) )
                    {
                        auto adj_cell = cell_adjacent( { cur_cell, facet } );
                        if( adj_cell != NO_ID && components[adj_cell] == NO_ID )
                        {
                            S.push( adj_cell );
                            components[adj_cell] = nb_components;
                        }
                    }
                } while( !S.empty() );
                nb_components++;
            }
        }
        return std::make_tuple( nb_components, components );
    }

    template < index_t DIMENSION >
    bool VolumeMesh< DIMENSION >::is_mesh_valid() const
    {
        bool valid{ true };

        if( this->nb_vertices() < 4 )
        {
            Logger::warn( "VolumeMesh has less than 4 vertices " );
            valid = false;
        }
        if( nb_cells() == 0 )
        {
            Logger::warn( "VolumeMesh has no cell" );
            valid = false;
        }

        // No isolated vertices
        std::vector< index_t > nb( this->nb_vertices(), 0 );
        for( auto c : range( nb_cells() ) )
        {
            for( auto v : range( nb_cell_vertices( c ) ) )
            {
                nb[cell_vertex( { c, v } )]++;
            }
        }
        auto nb_isolated_vertices =
            static_cast< index_t >( std::count( nb.begin(), nb.end(), 0 ) );
        if( nb_isolated_vertices > 0 )
        {
            Logger::warn( "VolumeMesh", "Mesh has ", nb_isolated_vertices,
                " isolated vertices " );
            valid = false;
        }

        return valid;
    }

    template < index_t DIMENSION >
    std::vector< index_t > VolumeMesh< DIMENSION >::cells_around_vertex(
        index_t vertex_id, index_t cell_hint ) const
    {
        std::vector< index_t > result;

        if( cell_hint == NO_ID )
        {
            const vecn< DIMENSION > cur_vec = this->vertex( vertex_id );
            index_t cell_vertex_not_used = NO_ID;
            bool found = find_cell_from_colocated_vertex_within_distance_if_any(
                cur_vec, global_epsilon, cell_hint, cell_vertex_not_used );
            if( !found )
            {
                return result;
            }
        }
        ringmesh_assert( cell_hint != NO_ID );

        // Flag the visited cells
        std::vector< index_t > visited;
        visited.reserve( 10 );

        // Stack of the adjacent cells
        std::stack< index_t > S;
        S.push( cell_hint );
        visited.push_back( cell_hint );

        do
        {
            auto c = S.top();
            S.pop();

            bool cell_includes_vertex{ false };
            for( auto v : range( nb_cell_vertices( c ) ) )
            {
                if( cell_vertex( { c, v } ) == vertex_id )
                {
                    result.push_back( c );
                    cell_includes_vertex = true;
                    break;
                }
            }
            if( !cell_includes_vertex )
            {
                continue;
            }

            for( auto f : range( nb_cell_facets( c ) ) )
            {
                for( auto v : range( nb_cell_facet_vertices( { c, f } ) ) )
                {
                    auto vertex = cell_facet_vertex( { c, f }, v );
                    if( vertex == vertex_id )
                    {
                        auto adj_P = cell_adjacent( { c, f } );

                        if( adj_P != NO_ID )
                        {
                            if( !contains( visited, adj_P ) )
                            {
                                S.push( adj_P );
                                visited.push_back( adj_P );
                            }
                        }
                        break;
                    }
                }
            }
        } while( !S.empty() );

        return result;
    }

    template < index_t DIMENSION >
    bool VolumeMesh< DIMENSION >::
        find_cell_from_colocated_vertex_within_distance_if_any(
            const vecn< DIMENSION >& vertex_vec,
            double distance,
            index_t& cell_id,
            index_t& cell_vertex_id ) const
    {
        bool result = false;
        cell_nn_search().get_neighbors( vertex_vec,
            [this, &vertex_vec, &result, &cell_id, &cell_vertex_id, distance](
                index_t i ) {
                for( auto j : range( nb_cell_vertices( i ) ) )
                {
                    if( inexact_equal( this->vertex( cell_vertex( { i, j } ) ),
                            vertex_vec, distance ) )
                    {
                        cell_vertex_id = cell_vertex( { i, j } );
                        cell_id = i;
                        result = true;
                        break;
                    }
                }
                return result;
            } );
        return result;
    }

    template < index_t DIMENSION >
    MeshSetBase< DIMENSION >::MeshSetBase()
    {
        create_point_set_mesh( "" );
        create_line_mesh( "" );
        create_well_mesh( "" );
        create_surface_mesh( "" );
    }

    template < index_t DIMENSION >
    void MeshSetBase< DIMENSION >::create_point_set_mesh( MeshType type )
    {
        point_set_mesh = PointSetMesh< DIMENSION >::create_mesh( type );
    }

    template < index_t DIMENSION >
    void MeshSetBase< DIMENSION >::create_line_mesh( MeshType type )
    {
        line_mesh = LineMesh< DIMENSION >::create_mesh( type );
    }

    template < index_t DIMENSION >
    void MeshSetBase< DIMENSION >::create_well_mesh( MeshType type )
    {
        well_mesh = LineMesh< DIMENSION >::create_mesh( type );
    }

    template < index_t DIMENSION >
    void MeshSetBase< DIMENSION >::create_surface_mesh( MeshType type )
    {
        surface_mesh = SurfaceMesh< DIMENSION >::create_mesh( type );
    }

    MeshSet< 3 >::MeshSet()
    {
        create_volume_mesh( "" );
    }

    void MeshSet< 3 >::create_volume_mesh( MeshType type )
    {
        volume_mesh = VolumeMesh3D::create_mesh( type );
    }

    template class RINGMESH_API PointSetMesh< 2 >;
    template class RINGMESH_API LineMesh< 2 >;
    template class RINGMESH_API SurfaceMeshBase< 2 >;
    template class RINGMESH_API MeshSetBase< 2 >;
    template class RINGMESH_API MeshSet< 2 >;

    template class RINGMESH_API PointSetMesh< 3 >;
    template class RINGMESH_API LineMesh< 3 >;
    template class RINGMESH_API SurfaceMeshBase< 3 >;
    template class RINGMESH_API VolumeMesh< 3 >;
    template class RINGMESH_API MeshSetBase< 3 >;
} // namespace RINGMesh
