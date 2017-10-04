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

#include <ringmesh/geomodel/geomodel_builder_repair.h>

#include <array>

#include <geogram/basic/algorithm.h>

#include <ringmesh/geomodel/geomodel_builder.h>

#include <ringmesh/mesh/mesh.h>
#include <ringmesh/mesh/mesh_builder.h>
#include <ringmesh/mesh/mesh_index.h>

/*!
 * @file Implementation of repair function of the surfaces of a GeoModel
 * @author Jeanne Pellerin
 * @author Pierre Anquez
 */

namespace RINGMesh
{
    template < index_t DIMENSION >
    GeoModelBuilderRepair< DIMENSION >::GeoModelBuilderRepair(
        GeoModelBuilder< DIMENSION >& builder, GeoModel< DIMENSION >& geomodel )
        : builder_( builder ),
          geomodel_( geomodel ),
          geomodel_access_( geomodel )
    {
    }

    template < index_t DIMENSION >
    void GeoModelBuilderRepair< DIMENSION >::repair( RepairMode repair_mode )
    {
        switch( repair_mode )
        {
        case ALL:
            geomodel_mesh_repair();
            break;
        case BASIC:
            builder_.end_geomodel();
            break;
        case COLOCATED_VERTICES:
            remove_colocated_entity_vertices_and_update_gm();
            break;
        case DEGENERATE_FACETS_EDGES:
            remove_degenerate_polygons_and_edges_and_update_gm();
            break;
        case LINE_BOUNDARY_ORDER:
            repair_line_boundary_vertex_order();
            break;
        case CONTACTS:
            build_contacts();
            break;
        case ISOLATED_VERTICES:
            remove_isolated_vertices();
            break;
        default:
            ringmesh_assert_not_reached;
        }
    }

    template < index_t DIMENSION >
    void GeoModelBuilderRepair< DIMENSION >::geomodel_mesh_repair()
    {
        // Remove colocated vertices in each entity
        remove_colocated_entity_vertices_and_update_gm();

        // Basic mesh repair for surfaces and lines
        remove_degenerate_polygons_and_edges_and_update_gm();

        // Proper reordering of line boundaries
        repair_line_boundary_vertex_order();

        // This is basic requirement ! no_colocated geomodel vertices !
        // So remove them if there are any
        geomodel_.mesh.remove_colocated_vertices();

        // Builds the contacts
        build_contacts();

        // Remove isolated vertices on mesh entities
        remove_isolated_vertices();

        builder_.end_geomodel();
    }

    template < index_t DIMENSION >
    void GeoModelBuilderRepair< DIMENSION >::
        remove_colocated_entity_vertices_and_update_gm()
    {
        std::set< gmme_id > empty_mesh_entities;
        std::set< gmge_id > empty_geological_entities;

        remove_colocated_entity_vertices( empty_mesh_entities );
        if( !empty_mesh_entities.empty() )
        {
            builder_.topology.get_dependent_entities(
                empty_mesh_entities, empty_geological_entities );
            builder_.removal.remove_mesh_entities( empty_mesh_entities );
        }
    }

    template < index_t DIMENSION >
    void GeoModelBuilderRepair< DIMENSION >::
        remove_degenerate_polygons_and_edges_and_update_gm()
    {
        std::set< gmme_id > empty_mesh_entities;
        remove_degenerate_polygons_and_edges( empty_mesh_entities );
        /// TODO when it will works,
        /// use GeoModelBuilderRemoval::remove_entities_and_dependencies
        if( !empty_mesh_entities.empty() )
        {
            builder_.removal.remove_mesh_entities( empty_mesh_entities );
        }

        // This is basic requirement ! no_colocated geomodel vertices !
        // So remove them if there are any
        geomodel_.mesh.remove_colocated_vertices();

        builder_.end_geomodel();
    }

    template < index_t DIMENSION >
    void GeoModelBuilderRepair< DIMENSION >::repair_line_boundary_vertex_order()
    {
        for( const auto& line : geomodel_.lines() )
        {
            if( !line.is_first_corner_first_vertex() )
            {
                const auto first_boundary_index = line.boundary( 0 ).index();
                builder_.topology.set_line_corner_boundary(
                    line.gmme().index(), 0, line.boundary_gmme( 1 ).index() );
                builder_.topology.set_line_corner_boundary(
                    line.gmme().index(), 1, first_boundary_index );
            }
        }
    }

    template <>
    void GeoModelBuilderRepair< 3 >::remove_isolated_vertices()
    {
        remove_isolated_vertices_base();
        for( const auto& surface : geomodel_.surfaces() )
        {
            remove_isolated_vertices_on_mesh_entity( surface );
        }
        for( const auto& region : geomodel_.regions() )
        {
            if( region.is_meshed() )
            {
                remove_isolated_vertices_on_mesh_entity( region );
            }
        }
    }

    template <>
    void GeoModelBuilderRepair< 2 >::remove_isolated_vertices()
    {
        remove_isolated_vertices_base();
        for( const auto& surface : geomodel_.surfaces() )
        {
            if( surface.is_meshed() )
            {
                remove_isolated_vertices_on_mesh_entity( surface );
            }
        }
    }

    template < index_t DIMENSION >
    void GeoModelBuilderRepair< DIMENSION >::remove_isolated_vertices_base()
    {
        for( const auto& line : geomodel_.lines() )
        {
            remove_isolated_vertices_on_mesh_entity( line );
        }
    }

    template < index_t DIMENSION >
    void GeoModelBuilderRepair< DIMENSION >::
        remove_isolated_vertices_on_mesh_entity(
            const GeoModelMeshEntity< DIMENSION >& geomodel_mesh_entity )
    {
        std::vector< bool > vertices_to_delete(
            geomodel_mesh_entity.nb_vertices(), true );
        for( auto mesh_element_index :
            range( geomodel_mesh_entity.nb_mesh_elements() ) )
        {
            for( auto vertex :
                range( geomodel_mesh_entity.nb_mesh_element_vertices(
                    mesh_element_index ) ) )
            {
                vertices_to_delete[geomodel_mesh_entity
                                       .mesh_element_vertex_index(
                                           { mesh_element_index, vertex } )] =
                    false;
            }
        }
        builder_.geometry.delete_mesh_entity_vertices(
            geomodel_mesh_entity.gmme(), vertices_to_delete );
    }

    template < index_t DIMENSION >
    bool GeoModelBuilderRepair< DIMENSION >::polygon_is_degenerate(
        const Surface< DIMENSION >& surface,
        index_t polygon_id,
        std::vector< index_t >& colocated_vertices )
    {
        auto nb_vertices = surface.nb_mesh_element_vertices( polygon_id );
        if( nb_vertices != 3 )
        {
            std::vector< index_t > vertices( nb_vertices );
            for( auto v : range( nb_vertices ) )
            {
                vertices[v] =
                    colocated_vertices[surface.mesh_element_vertex_index(
                        ElementLocalVertex( polygon_id, v ) )];
            }
            GEO::sort_unique( vertices );
            return vertices.size() != nb_vertices;
        }
        auto v1 = colocated_vertices[surface.mesh_element_vertex_index(
            { polygon_id, 0 } )];
        auto v2 = colocated_vertices[surface.mesh_element_vertex_index(
            { polygon_id, 1 } )];
        auto v3 = colocated_vertices[surface.mesh_element_vertex_index(
            { polygon_id, 2 } )];
        return v1 == v2 || v2 == v3 || v3 == v1;
    }

    template < index_t DIMENSION >
    std::vector< index_t >
        GeoModelBuilderRepair< DIMENSION >::surface_detect_degenerate_polygons(
            const Surface< DIMENSION >& surface,
            std::vector< index_t >& colocated_vertices )
    {
        std::vector< index_t > f_is_degenerate( surface.nb_mesh_elements() );
        for( auto p : range( surface.nb_mesh_elements() ) )
        {
            f_is_degenerate[p] =
                polygon_is_degenerate( surface, p, colocated_vertices );
        }
        return f_is_degenerate;
    }

    template < index_t DIMENSION >
    index_t GeoModelBuilderRepair< DIMENSION >::detect_degenerate_polygons(
        const Surface< DIMENSION >& surface )
    {
        std::vector< index_t > colocated;
        const auto& nn_search = surface.vertex_nn_search();
        std::tie( std::ignore, colocated ) =
            nn_search.get_colocated_index_mapping( geomodel_.epsilon() );

        auto degenerate =
            surface_detect_degenerate_polygons( surface, colocated );
        return static_cast< index_t >(
            std::count( degenerate.begin(), degenerate.end(), 1 ) );
    }

    template < index_t DIMENSION >
    std::vector< bool >
        GeoModelBuilderRepair< DIMENSION >::line_detect_degenerate_edges(
            const Line< DIMENSION >& line,
            std::vector< index_t >& colocated_vertices )
    {
        std::vector< bool > e_is_degenerate( line.nb_mesh_elements() );
        for( auto e : range( line.nb_mesh_elements() ) )
        {
            e_is_degenerate[e] =
                edge_is_degenerate( line, e, colocated_vertices );
        }
        return e_is_degenerate;
    }

    template < index_t DIMENSION >
    index_t GeoModelBuilderRepair< DIMENSION >::repair_line_mesh(
        const Line< DIMENSION >& line )
    {
        std::vector< index_t > colocated;
        const auto& nn_search = line.vertex_nn_search();
        std::tie( std::ignore, colocated ) =
            nn_search.get_colocated_index_mapping( geomodel_.epsilon() );

        auto degenerate = line_detect_degenerate_edges( line, colocated );
        auto nb = static_cast< index_t >(
            std::count( degenerate.begin(), degenerate.end(), 1 ) );
        /// We have a problem if some vertices are left isolated
        /// If we remove them here we can kill all indices correspondances
        builder_.geometry.delete_line_edges( line.index(), degenerate, false );
        return nb;
    }

    template < index_t DIMENSION >
    void GeoModelBuilderRepair< DIMENSION >::
        remove_degenerate_polygons_and_edges( std::set< gmme_id >& to_remove )
    {
        to_remove.clear();
        for( const auto& line : geomodel_.lines() )
        {
            auto nb = repair_line_mesh( line );
            if( nb > 0 )
            {
                Logger::out( "GeoModel", nb,
                    " degenerated edges removed in LINE ", line.index() );
                // If the Line is set it to remove
                if( line.nb_mesh_elements() == 0 )
                {
                    to_remove.insert( line.gmme() );
                }
            }
        }
        // The builder might be needed

        double epsilon_sq = geomodel_.epsilon() * geomodel_.epsilon();
        for( const auto& surface : geomodel_.surfaces() )
        {
            auto nb = detect_degenerate_polygons( surface );
            /// @todo Check if that cannot be simplified
            if( nb > 0 )
            {
                // If there are some degenerated polygons
                // Using repair function of geogram
                // Warning - This triangulates the mesh
                if( surface.nb_vertices() > 0 )
                {
                    // Colocated vertices must be processed before
                    // MESH_REPAIR_DUP_F 2 ;
                    auto mode = static_cast< GEO::MeshRepairMode >( 2 );
                    auto builder = builder_.geometry.create_surface_builder(
                        surface.index() );
                    builder->repair( mode, 0.0 );

                    // This might create some small components - remove them
                    builder->remove_small_connected_components( epsilon_sq, 3 );

                    // Alright, this is a bit of an overkill [JP]
                    if( surface.nb_vertices() > 0 )
                    {
                        builder->repair( mode, 0.0 );
                    }
                }
                if( surface.nb_vertices() == 0
                    || surface.nb_mesh_elements() == 0 )
                {
                    to_remove.insert( surface.gmme() );
                }
            }
        }
    }

    template < index_t DIMENSION >
    std::set< index_t >
        GeoModelBuilderRepair< DIMENSION >::vertices_on_inside_boundary(
            const gmme_id& E_id )
    {
        std::set< index_t > vertices;
        if( E_id.type() == Corner< DIMENSION >::type_name_static() )
        {
            return vertices;
        }
        const auto& mesh_entity = geomodel_.mesh_entity( E_id );
        if( E_id.type() == Line< DIMENSION >::type_name_static() )
        {
            if( mesh_entity.boundary( 0 ).is_inside_border( mesh_entity ) )
            {
                vertices.insert( mesh_entity.nb_vertices() - 1 );
            }
            return vertices;
        }
        std::vector< const GeoModelMeshEntity< DIMENSION >* > inside_border;
        for( auto i : range( mesh_entity.nb_boundaries() ) )
        {
            if( mesh_entity.boundary( i ).is_inside_border( mesh_entity ) )
            {
                inside_border.push_back(
                    dynamic_cast< const GeoModelMeshEntity< DIMENSION >* >(
                        &mesh_entity.boundary( i ) ) );
            }
        }
        if( !inside_border.empty() )
        {
            // We want to get the indices of the vertices in E
            // that are colocated with those of the inside boundary
            // We assume that the geomodel vertices are not computed
            const auto& nn_search = mesh_entity.vertex_nn_search();

            for( const auto& entity : inside_border )
            {
                for( auto v : range( entity->nb_vertices() ) )
                {
                    auto colocated_indices = nn_search.get_neighbors(
                        entity->vertex( v ), geomodel_.epsilon() );
                    if( colocated_indices.size() > 1 )
                    {
                        std::sort( colocated_indices.begin(),
                            colocated_indices.end() );
                        // Add colocated vertices except one to the duplicated
                        // vertices set
                        vertices.insert( colocated_indices.begin() + 1,
                            colocated_indices.end() );
                    }
                }
            }
        }
        return vertices;
    }

    template < index_t DIMENSION >
    void GeoModelBuilderRepair< DIMENSION >::remove_colocated_entity_vertices(
        std::set< gmme_id >& to_remove )
    {
        to_remove.clear();
        // For all Lines and Surfaces
        std::array< const MeshEntityType, 2 > types{
            { Line< DIMENSION >::type_name_static(),
                Surface< DIMENSION >::type_name_static() }
        };
        for( const auto& type : types )
        {
            for( auto e : range( geomodel_.nb_mesh_entities( type ) ) )
            {
                gmme_id entity_id{ type, e };
                const auto& E = geomodel_.mesh_entity( entity_id );

                const auto& kdtree = E.vertex_nn_search();
                std::vector< index_t > colocated;
                std::tie( std::ignore, colocated ) =
                    kdtree.get_colocated_index_mapping( geomodel_.epsilon() );

                // Get the vertices to delete
                auto inside_border = vertices_on_inside_boundary( entity_id );

                std::vector< bool > to_delete( colocated.size(), false );
                index_t nb_todelete{ 0 };
                for( auto v : range( colocated.size() ) )
                {
                    if( colocated[v] == v
                        || inside_border.find( v ) != inside_border.end() )
                    {
                        // This point is kept
                        // No colocated or on an inside boundary
                    }
                    else
                    {
                        // The point is to remove
                        to_delete[v] = true;
                        nb_todelete++;
                    }
                }

                if( nb_todelete == 0 )
                {
                    // Nothing to do there
                    continue;
                }
                if( nb_todelete == E.nb_vertices() )
                {
                    // The complete entity should be removed
                    to_remove.insert( E.gmme() );
                    continue;
                }
                if( type == Surface< DIMENSION >::type_name_static() )
                {
                    auto builder =
                        builder_.geometry.create_surface_builder( e );
                    for( auto p_itr : range( E.nb_mesh_elements() ) )
                    {
                        for( auto fpv_itr :
                            range( E.nb_mesh_element_vertices( p_itr ) ) )
                        {
                            builder->set_polygon_vertex( { p_itr, fpv_itr },
                                colocated[E.mesh_element_vertex_index(
                                    { p_itr, fpv_itr } )] );
                        }
                    }
                    builder->delete_vertices( to_delete );
                    Logger::out( "Repair", nb_todelete,
                        " colocated vertices deleted in ", entity_id );
                }
                else if( type == Line< DIMENSION >::type_name_static() )
                {
                    auto builder = builder_.geometry.create_line_builder( e );
                    for( auto e_itr : range( E.nb_mesh_elements() ) )
                    {
                        builder->set_edge_vertex(
                            { e_itr, 0 }, colocated[E.mesh_element_vertex_index(
                                              { e_itr, 0 } )] );
                        builder->set_edge_vertex(
                            { e_itr, 1 }, colocated[E.mesh_element_vertex_index(
                                              { e_itr, 1 } )] );
                    }
                    builder->delete_vertices( to_delete );
                    Logger::out( "Repair", nb_todelete,
                        " colocated vertices deleted in ", entity_id );
                }
                else
                {
                    ringmesh_assert_not_reached;
                }
            }
        }
    }

    template < index_t DIMENSION >
    bool GeoModelBuilderRepair< DIMENSION >::edge_is_degenerate(
        const Line< DIMENSION >& line,
        index_t edge,
        const std::vector< index_t >& colocated_vertices )
    {
        auto v1 =
            colocated_vertices[line.mesh_element_vertex_index( { edge, 0 } )];
        auto v2 =
            colocated_vertices[line.mesh_element_vertex_index( { edge, 1 } )];
        return v1 == v2;
    }

    template < index_t DIMENSION >
    void GeoModelBuilderRepair< DIMENSION >::build_contacts()
    {
        builder_.geology.build_contacts();
    }
    template class RINGMESH_API GeoModelBuilderRepair< 2 >;
    template class RINGMESH_API GeoModelBuilderRepair< 3 >;
} // namespace RINGMesh
