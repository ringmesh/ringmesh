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

#include <ringmesh/geomodel/geo_model_mesh.h>

#include <stack>

#include <geogram/basic/algorithm.h>

#include <geogram/mesh/mesh.h>
#include <geogram/mesh/mesh_geometry.h>
#include <geogram/mesh/mesh_repair.h>
#include <geogram/points/colocate.h>

#include <ringmesh/basic/algorithm.h>
#include <ringmesh/basic/box3d.h>
#include <ringmesh/basic/geometry.h>
#include <ringmesh/geogram_extension/geogram_extension.h>
#include <ringmesh/geomodel/geo_model.h>
#include <ringmesh/geomodel/geo_model_builder.h>
#include <ringmesh/mesh/well.h>

/*!
 * @author Arnaud Botella - Jeanne Pellerin - Antoine Mazuyer
 */

namespace {
    using namespace RINGMesh ;

    class GeoModelMeshFacetsSort {
    public:
        GeoModelMeshFacetsSort(
            const Mesh& mesh,
            const GEO::Attribute< index_t >& surface_id )
            : mesh_( mesh ), surface_id_( surface_id )
        {
        }

        bool operator()( index_t i, index_t j ) const
        {
            if( surface_id_[i] != surface_id_[j] ) {
                return surface_id_[i] < surface_id_[j] ;
            } else {
                return mesh_.nb_facet_vertices( i ) < mesh_.nb_facet_vertices( j ) ;
            }
        }
    private:
        const Mesh& mesh_ ;
        const GEO::Attribute< index_t >& surface_id_ ;
    } ;

    class GeoModelMeshCellsSort {
    public:
        GeoModelMeshCellsSort(
            const Mesh& mesh,
            const GEO::Attribute< index_t >& region_id )
            : mesh_( mesh ), region_id_( region_id )
        {
        }

        bool operator()( index_t i, index_t j ) const
        {
            if( region_id_[i] != region_id_[j] ) {
                return region_id_[i] < region_id_[j] ;
            } else {
                return mesh_.cell_type( i ) < mesh_.cell_type( j ) ;
            }
        }
    private:
        const Mesh& mesh_ ;
        const GEO::Attribute< index_t >& region_id_ ;
    } ;

    std::string vertex_map_name()
    {
        return "model_vertex_map" ;
    }

    void cell_facets_around_vertex(
        const Mesh& mesh,
        index_t cell,
        index_t vertex_id,
        std::vector< index_t >& facets )
    {
        facets.reserve( mesh.nb_cell_facets( cell ) ) ;
        for( index_t f = 0; f < mesh.nb_cell_facets( cell ); f++ ) {
            for( index_t v = 0; v < mesh.nb_cell_facet_vertices( cell, f ); v++ ) {
                if( mesh.cell_facet_vertex( cell, f, v ) == vertex_id ) {
                    facets.push_back( f ) ;
                    break ;
                }
            }
        }
    }

    bool is_attribute_a_double(
        GEO::AttributesManager& att_manager,
        const std::string& att_name )
    {
        return GEO::Attribute< double >::is_defined( att_manager, att_name ) ;
    }
}

namespace RINGMesh {
    void GeoModelVertexMapper::test_and_initialize() const
    {
        if( !is_initialized() ) {
            const_cast< GeoModelVertexMapper* >( this )->initialize() ;
        }
    }
    void GeoModelVertexMapper::clear()
    {
        gme_vertices_.clear() ;
        clear_all_mesh_entity_vertex_map() ;
    }

    index_t GeoModelVertexMapper::model_vertex_index(
        const gme_t& mesh_entity_id,
        index_t mesh_entity_vertex_index ) const
    {
        ringmesh_assert(
            EntityTypeManager::is_mesh_entity_type( mesh_entity_id.type ) ) ;
        ringmesh_assert( mesh_entity_vertex_index <
            geomodel_.mesh_entity( mesh_entity_id ).nb_vertices() ) ;

        test_and_initialize_mesh_entity_vertex_map( mesh_entity_id ) ;

        GEO::Attribute< index_t > mesh_entity_vertex_map(
            mesh_entity_vertex_attribute_manager( mesh_entity_id ),
            vertex_map_name() ) ;

        return mesh_entity_vertex_map[mesh_entity_vertex_index] ;
    }

    const std::vector< GMEVertex >& GeoModelVertexMapper::mesh_entity_vertex_indices(
        index_t v ) const
    {
        test_and_initialize() ;
        ringmesh_assert( v < gme_vertices_.size() ) ;
        return gme_vertices_[v] ;
    }

    void GeoModelVertexMapper::mesh_entity_vertex_indices(
        index_t v,
        const EntityType& mesh_entity_type,
        std::vector< GMEVertex >& result ) const
    {
        result.clear() ;
        const std::vector< GMEVertex >& all_gmes = mesh_entity_vertex_indices( v ) ;
        for( index_t i = 0; i < all_gmes.size(); i++ ) {
            if( all_gmes[i].gme_id.type == mesh_entity_type ) {
                result.push_back( all_gmes[i] ) ;
            }
        }
    }

    void GeoModelVertexMapper::mesh_entity_vertex_indices(
        index_t v,
        const gme_t& mesh_entity_id,
        std::vector< index_t >& result ) const
    {
        result.clear() ;
        std::vector< GMEVertex > all_gmes = mesh_entity_vertex_indices( v ) ;
        for( index_t i = 0; i < all_gmes.size(); i++ ) {
            if( all_gmes[i].gme_id == mesh_entity_id ) {
                result.push_back( all_gmes[i].v_id ) ;
            }
        }
    }

    void  GeoModelVertexMapper::set_vertex_map_value(
        const gme_t& mesh_entity_id,
        index_t mesh_entity_vertex_index,
        index_t model_entity_vertex_index ) const
    {
        ringmesh_assert( is_mesh_entity_vertex_map_initialized( mesh_entity_id ) ) ;

        GEO::Attribute< index_t > mesh_entity_vertex_map(
            mesh_entity_vertex_attribute_manager( mesh_entity_id ),
            vertex_map_name() ) ;

        mesh_entity_vertex_map[mesh_entity_vertex_index] =
            model_entity_vertex_index ;
    }

    void GeoModelVertexMapper::initialize_mesh_entity_vertex_map_to_default(
        const gme_t& mesh_entity_id ) const
    {
        ringmesh_assert(
            EntityTypeManager::is_mesh_entity_type( mesh_entity_id.type ) ) ;
        nb_vertex_maps_++ ;

        GEO::Attribute< index_t > mesh_entity_vertex_map(
            mesh_entity_vertex_attribute_manager( mesh_entity_id ),
            vertex_map_name() ) ;
        mesh_entity_vertex_map.fill( NO_ID ) ;
    }

    void GeoModelVertexMapper::update_mesh_entity_maps_and_gmes(
        const std::vector< index_t >& old2new )
    {
        const std::vector< EntityType >& all_mesh_entity_types =
            EntityTypeManager::mesh_entity_types() ;
        for( index_t t = 0; t < all_mesh_entity_types.size(); t++ ) {
            EntityType cur_entity_type = all_mesh_entity_types[t] ;
            for( index_t e = 0; e < geomodel_.nb_mesh_entities( cur_entity_type );
                e++ ) {
                const GeoModelMeshEntity& E = geomodel_.mesh_entity( cur_entity_type,
                    e ) ;
                for( index_t v = 0; v < E.nb_vertices(); v++ ) {
                    index_t old_m_id = model_vertex_index( E.gme_id(), v ) ;
                    index_t new_m_id = old2new[ old_m_id ] ;
                    set_vertex_map_value( E.gme_id(), v, new_m_id ) ;

                    // Merge gme_vertices_ information
                    if( std::find( gme_vertices_[new_m_id].begin(),
                        gme_vertices_[new_m_id].end(), GMEVertex( E.gme_id(), v ) )
                        == gme_vertices_[new_m_id].end() ) {
                        gme_vertices_[new_m_id].push_back( GMEVertex( E.gme_id(), v ) ) ;
                    }
                }
            }
        }

    }



    void GeoModelVertexMapper::initialize()
    {
        // Initializing maps ;
        initialize_mesh_entity_vertex_maps() ;

        // Fill the gme_vertices vectors
        fill_gme_vertices() ;
    }

    bool GeoModelVertexMapper::is_initialized() const
    {
        if( gme_vertices_.empty() ) {
            return false ;
        }
        const_cast< GeoModelVertexMapper* >( this )->check_mesh_entity_maps() ;
        return true ;
    }

    void GeoModelVertexMapper::check_mesh_entity_maps()
    {
        if( nb_vertex_maps_ == total_nb_mesh_entities() ) {
            return ;
        }
        const std::vector< EntityType >& all_mesh_entity_types =
            EntityTypeManager::mesh_entity_types() ;
        for( index_t t = 0; t < all_mesh_entity_types.size(); t++ ) {
            EntityType cur_entity_type = all_mesh_entity_types[t] ;
            for( index_t e = 0; e < geomodel_.nb_mesh_entities( cur_entity_type );
                e++ ) {
                const gme_t cur_mesh_entity( cur_entity_type, e ) ;
                bool cur_map_was_init = test_and_initialize_mesh_entity_vertex_map(
                    cur_mesh_entity ) ;
                if( !cur_map_was_init ) {
                     add_mesh_entity_vertices_to_gme( cur_mesh_entity ) ;
                }
            }
        }
    }

    void GeoModelVertexMapper::initialize_mesh_entity_vertex_maps()
    {
        const std::vector< EntityType >& all_mesh_entity_types =
            EntityTypeManager::mesh_entity_types() ;
        for( index_t t = 0; t < all_mesh_entity_types.size(); t++ ) {
            EntityType cur_entity_type = all_mesh_entity_types[t] ;
            for( index_t e = 0; e < geomodel_.nb_mesh_entities( cur_entity_type );
                e++ ) {
                const gme_t cur_mesh_entity( cur_entity_type, e ) ;
                initialize_mesh_entity_vertex_map( cur_mesh_entity ) ;
            }
        }
    }

    void GeoModelVertexMapper::initialize_mesh_entity_vertex_map(
        const gme_t& mesh_entity_id ) const
    {
        initialize_mesh_entity_vertex_map_to_default( mesh_entity_id ) ;

        const GeoModelMeshEntity& E = geomodel_.mesh_entity( mesh_entity_id ) ;
        GEO::Attribute< index_t > mesh_entity_vertex_map(
            mesh_entity_vertex_attribute_manager( mesh_entity_id ),
            vertex_map_name() ) ;

        for( index_t v = 0; v < E.nb_vertices(); v++ ) {
            std::vector< index_t > result ;
            model_vertices_.colocater().get_neighbors( E.vertex( v ), 1, result ) ;
            mesh_entity_vertex_map[v] = result[0] ;
        }
    }

    bool GeoModelVertexMapper::test_and_initialize_mesh_entity_vertex_map(
        const gme_t& mesh_entity_id ) const
    {
        if( !is_mesh_entity_vertex_map_initialized( mesh_entity_id ) ) {
            initialize_mesh_entity_vertex_map( mesh_entity_id ) ;
            return false ;
        }
        return true ;
    }

    bool GeoModelVertexMapper::is_mesh_entity_vertex_map_initialized(
        const gme_t& mesh_entity_id ) const
    {
        return mesh_entity_vertex_attribute_manager( mesh_entity_id ).is_defined(
            vertex_map_name() ) ;
    }

    void GeoModelVertexMapper::clear_all_mesh_entity_vertex_map()
    {
        const std::vector< EntityType >& all_mesh_entity_types =
            EntityTypeManager::mesh_entity_types() ;
        for( index_t t = 0; t < all_mesh_entity_types.size(); t++ ) {
            EntityType cur_entity_type = all_mesh_entity_types[t] ;
            for( index_t e = 0; e < geomodel_.nb_mesh_entities( cur_entity_type );
                e++ ) {
                const GeoModelMeshEntity& E = geomodel_.mesh_entity( cur_entity_type, e ) ;
                GEO::Attribute< index_t > vertex_map( E.vertex_attribute_manager(),
                    vertex_map_name() ) ;
                vertex_map.unbind() ;
            }
        }
        nb_vertex_maps_ = 0 ;
    }

    void GeoModelVertexMapper::fill_gme_vertices()
    {
        // Set the size of gme_vertices to the total number of model vertices
        gme_vertices_.resize( model_vertices_.nb() ) ;

        for( index_t v = 0; v < model_vertices_.nb(); v++ ) {
            std::vector< GMEVertex >& cur_v_gme_vertices = gme_vertices_[v] ;
            compute_all_mesh_entity_vertices( v, cur_v_gme_vertices ) ;
        }

    }

    void GeoModelVertexMapper::add_mesh_entity_vertices_to_gme( const gme_t& mesh_entity_id )
    {
        const GeoModelMeshEntity& E = geomodel_.mesh_entity( mesh_entity_id ) ;
        for( index_t v = 0; v < E.nb_vertices(); v++ ) {
            const GMEVertex cur_gme_vertex ( mesh_entity_id, v ) ;
            GEO::Attribute< index_t > mesh_entity_vertex_map(
                mesh_entity_vertex_attribute_manager( mesh_entity_id ),
                vertex_map_name() ) ;
            index_t cur_model_v_id = mesh_entity_vertex_map[v] ;
            gme_vertices_[cur_model_v_id].push_back( cur_gme_vertex ) ;
        }
    }

    void GeoModelVertexMapper::compute_all_mesh_entity_vertices(
        index_t v,
        std::vector< GMEVertex >& gme_vertices ) const
    {
        const std::vector< EntityType >& all_mesh_entity_types =
            EntityTypeManager::mesh_entity_types() ;
        for( index_t t = 0; t < all_mesh_entity_types.size(); t++ ) {
            EntityType cur_entity_type = all_mesh_entity_types[t] ;
            compute_mesh_entity_type_vertices( v, cur_entity_type, gme_vertices ) ;
        }
    }

    void GeoModelVertexMapper::compute_mesh_entity_type_vertices(
        index_t v,
        const EntityType& entity_type,
        std::vector< GMEVertex >& gme_vertices ) const
    {
        ringmesh_assert(
            EntityTypeManager::is_mesh_entity_type( entity_type ) ) ;

        for( index_t e = 0; e < geomodel_.nb_mesh_entities( entity_type );
            e++ ) {
            gme_t cur_entity( entity_type, e ) ;
            std::vector< index_t > cur_gme_vertices ;
            compute_mesh_entity_vertex_indices( cur_entity, v, cur_gme_vertices ) ;
            for( index_t gme_v = 0; gme_v < cur_gme_vertices.size(); gme_v++ ) {
                gme_vertices.push_back(
                    GMEVertex( cur_entity, cur_gme_vertices[gme_v] ) ) ;
            }
        }
    }

    void GeoModelVertexMapper::compute_mesh_entity_vertex_indices(
        const gme_t& mesh_entity_id,
        index_t v,
        std::vector< index_t >& result ) const
    {
        ringmesh_assert(
            EntityTypeManager::is_mesh_entity_type( mesh_entity_id.type ) ) ;
        ringmesh_assert( v < model_vertices_.nb() ) ;

        GEO::Attribute< index_t > mesh_entity_vertex_map(
            mesh_entity_vertex_attribute_manager( mesh_entity_id ),
            vertex_map_name() ) ;
        index_t nb_mesh_entity_vertices =
            geomodel_.mesh_entity( mesh_entity_id ).nb_vertices() ;

        for( index_t vertex = 0; vertex < nb_mesh_entity_vertices; vertex++ ) {
            if( mesh_entity_vertex_map[vertex] == v ) {
                result.push_back( vertex ) ;
            }
        }
    }

    index_t GeoModelVertexMapper::total_nb_mesh_entities() const
    {
        index_t nb_total = 0 ;
        const std::vector< EntityType >& all_mesh_entity_types =
            EntityTypeManager::mesh_entity_types() ;
        for( index_t t = 0; t < all_mesh_entity_types.size(); t++ ) {
            nb_total += geomodel_.nb_mesh_entities( all_mesh_entity_types[t] ) ;
        }
        return nb_total ;
    }

    GEO::AttributesManager& GeoModelVertexMapper::mesh_entity_vertex_attribute_manager(
        const gme_t& mesh_entity_id ) const
    {
        ringmesh_assert(
            EntityTypeManager::is_mesh_entity_type( mesh_entity_id.type ) ) ;

        const GeoModelMeshEntity& mesh_entity = geomodel_.mesh_entity(
            mesh_entity_id ) ;
        return mesh_entity.vertex_attribute_manager() ;
    }



    GeoModelMeshVertices::GeoModelMeshVertices(
        GeoModelMesh& gmm,
        GeoModel& gm,
        Mesh& mesh,
        MeshBuilder& mesh_builder )
        :
            gmm_( gmm ),
            gm_( gm ),
            mesh_( mesh ),
            mesh_builder_( mesh_builder ),
            vertex_mapper_( *this, gm )
    {
    }

    GeoModelMeshVertices::~GeoModelMeshVertices()
    {
    }

    bool GeoModelMeshVertices::is_initialized() const
    {
        return mesh_.nb_vertices() > 0 ;
    }

    void GeoModelMeshVertices::test_and_initialize() const
    {
        if( !is_initialized() ) {
            const_cast< GeoModelMeshVertices* >( this )->initialize() ;
        }
    }

    index_t nb_entity_vertices( const GeoModel& M, const std::string& entity_type )
    {
        index_t count = 0 ;
        for( index_t i = 0; i < M.nb_mesh_entities( entity_type ); ++i ) {
            count += M.mesh_entity( entity_type, i ).nb_vertices() ;
        }
        return count ;
    }

    void fill_vertices(
        const GeoModel& M,
        const std::string& entity_type,
        MeshBuilder& builder,
        const GeoModelVertexMapper& vertex_mapper,
        index_t& count )
    {
        for( index_t i = 0; i < M.nb_mesh_entities( entity_type ); ++i ) {
            GeoModelMeshEntity& E = const_cast< GeoModelMeshEntity& >( M.mesh_entity(
                entity_type, i ) ) ;

            // Map initialization ;
            vertex_mapper.initialize_mesh_entity_vertex_map_to_default( E.gme_id() ) ;

            if( E.nb_vertices() == 0 ) {
                continue ;
            }

            // Map and vertex
            for( index_t v = 0; v < E.nb_vertices(); v++ ) {
                builder.set_vertex( count, E.vertex( v ) ) ;
                // Map from vertices of MeshEntities to GeoModelMeshVertices
                vertex_mapper.set_vertex_map_value( E.gme_id(), v, count ) ;
                // Global vertex index increment
                count++ ;
            }
        }
    }

    void GeoModelMeshVertices::initialize()
    {
        MeshBuilder builder( mesh_ ) ;
        builder.clear( true, false ) ;

        // Total number of vertices in the
        // Corners, Lines, Surfaces and Regions of the GeoModel
        index_t nb = 0 ;
        nb += nb_entity_vertices( gm_, Corner::type_name_static() ) ;
        nb += nb_entity_vertices( gm_, Line::type_name_static() ) ;
        nb += nb_entity_vertices( gm_, Surface::type_name_static() ) ;
        nb += nb_entity_vertices( gm_, Region::type_name_static() ) ;

        // Get out if no vertices
        if( nb == 0 ) {
            return ;
        }

        // Fill the vertices
        builder.create_vertices( nb ) ;
        vertex_mapper_.resize_model_vertex_gmes( nb ) ;

        index_t count = 0 ;
        fill_vertices( gm_, Corner::type_name_static(), builder, vertex_mapper_,
            count ) ;
        fill_vertices( gm_, Line::type_name_static(), builder, vertex_mapper_,
            count ) ;
        fill_vertices( gm_, Surface::type_name_static(), builder, vertex_mapper_,
            count ) ;
        fill_vertices( gm_, Region::type_name_static(), builder, vertex_mapper_,
            count ) ;
        // Remove colocated vertices
        remove_colocated() ;

        // Initialize vertex mapper
        vertex_mapper_.test_and_initialize() ;
    }

    void GeoModelMeshVertices::clear()
    {
        gmm_.cells.clear() ;
        gmm_.facets.clear() ;
        gmm_.edges.clear() ;
        vertex_mapper_.clear() ;

        MeshBuilder builder( mesh_ ) ;
        builder.clear_vertices( true, false ) ;
    }

    void GeoModelMeshVertices::unbind_model_vertex_map(
        const GeoModelMeshEntity& E ) const
    {
        GEO::Attribute< index_t > vertex_map( E.vertex_attribute_manager(),
            vertex_map_name() ) ;
        vertex_map.unbind() ;
    }

    index_t GeoModelMeshVertices::nb() const
    {
        test_and_initialize() ;
        return mesh_.nb_vertices() ;
    }

    const vec3& GeoModelMeshVertices::vertex( index_t v ) const
    {
        test_and_initialize() ;
        ringmesh_assert( v < nb() ) ;
        return mesh_.vertex( v ) ;
    }

    index_t GeoModelMeshVertices::index( const vec3& p ) const
    {
        test_and_initialize() ;
        std::vector< index_t > vertices ;
        const ColocaterANN& colocator = mesh_.colocater_ann(
            ColocaterANN::VERTICES ) ;
        colocator.get_neighbors( p, vertices, gm_.epsilon() ) ;
        if( vertices.empty() ) {
            return NO_ID ;
        } else {
            ringmesh_assert( vertices.size() == 1 ) ;
            return vertices[0] ;
        }
    }

    index_t GeoModelMeshVertices::model_vertex_id(
        const gme_t& mesh_entity,
        index_t entity_vertex_index ) const
    {
        test_and_initialize() ;
        return vertex_mapper_.model_vertex_index( mesh_entity, entity_vertex_index ) ;
    }

    index_t GeoModelMeshVertices::model_vertex_id(
        const gme_t& mesh_entity,
        index_t entity_mesh_element_index,
        index_t vertex_local_index ) const
    {
        index_t entity_vertex_index =
            gm_.mesh_entity( mesh_entity ).mesh_element_vertex_index(
                entity_mesh_element_index, vertex_local_index ) ;
        return model_vertex_id( mesh_entity, entity_vertex_index ) ;
    }

    void GeoModelMeshVertices::mesh_entity_vertex_id(
        const gme_t& mesh_entity,
        index_t model_vertex_index,
        std::vector< index_t >& mesh_entity_vertex_ids ) const
    {
        test_and_initialize() ;
        ringmesh_assert( model_vertex_index < nb() ) ;
        vertex_mapper_.mesh_entity_vertex_indices( model_vertex_index, mesh_entity,
            mesh_entity_vertex_ids ) ;
    }

    void GeoModelMeshVertices::gme_vertices(
        index_t v,
        std::vector< GMEVertex >& gme_vertices ) const
    {
        test_and_initialize() ;
        gme_vertices = vertex_mapper_.mesh_entity_vertex_indices( v ) ;
    }

    void GeoModelMeshVertices::gme_type_vertices(
        const EntityType& entity_type,
        index_t v,
        std::vector< GMEVertex >& gme_vertices ) const
    {
        test_and_initialize() ;
        vertex_mapper_.mesh_entity_vertex_indices( v, entity_type, gme_vertices ) ;
    }

    index_t GeoModelMeshVertices::add_vertex( const vec3& point )
    {
        MeshBuilder builder( mesh_ ) ;
        return builder.create_vertex( point ) ;
    }

    void GeoModelMeshVertices::update_point( index_t v, const vec3& point )
    {
        test_and_initialize() ;
        ringmesh_assert( v < nb() ) ;
        // Change the position of the unique_vertex
        mesh_builder_.set_vertex( v, point ) ;

        GeoModelBuilder builder( gm_ ) ;

        std::vector< GMEVertex > gme_v ;
        gme_vertices( v, gme_v ) ;
        for( index_t i = 0; i < gme_v.size(); i++ ) {
            const GMEVertex& info = gme_v[i] ;
            builder.set_mesh_entity_vertex( info.gme_id, info.v_id, point, false ) ;
        }
    }

    void GeoModelMeshVertices::remove_colocated()
    {
        // Get out if nothing to do
        // and compute the points if they are not initialized yet
        if( nb() == 0 ) {
            return ;
        }
        // Identify and invalidate colocated vertices
        GEO::vector< index_t > old2new ;
        index_t nb_colocalised_vertices = mesh_.colocater_ann(
            ColocaterANN::VERTICES ).get_colocated_index_mapping( gm_.epsilon(),
            old2new ) ;
        if( nb_colocalised_vertices > 0 ) {
            std::vector< index_t > vector_copy( old2new.begin(), old2new.end() ) ;
            erase_vertices( vector_copy ) ;
        }
    }

    void GeoModelMeshVertices::erase_vertices( std::vector< index_t >& to_delete )
    {
        ringmesh_assert( to_delete.size() == nb() ) ;

        // For mesh vertices deletion
        GEO::vector< index_t > to_delete_geo( nb(), 0 ) ;

        // Fill the delete information for geogram
        // Recycle the to_delete vertex to get the mapping between
        // new and old points. This is implemented to be the same
        // as what is done in the delete_elements function in geogram
        index_t nb_todelete = 0 ;
        index_t cur = 0 ;
        for( index_t v = 0; v < nb(); ++v ) {
            if( to_delete[v] != v ) {
                to_delete_geo[v] = 1 ;
                nb_todelete++ ;
                if( to_delete[v] != NO_ID ) {
                    ringmesh_assert( to_delete[v] < v ) ;
                    to_delete[v] = to_delete[to_delete[v]] ;
                }
            } else {
                to_delete[v] = cur ;
                ++cur ;
            }
        }
        if( nb_todelete == 0 ) {
            return ;
        }
        if( nb_todelete == nb() ) {
            // Clear everything
            clear() ;
            return ;
        }

        // Empty the gme_vertices_ of the deleted vertices and erase them
        for( index_t v = 0; v < nb(); ++v ) {
            if( to_delete_geo[v] == 1 ) {
                vertex_mapper_.clear_model_vertex_gmes( v ) ;
            }
        }

        // Delete the vertices - false is to not remove
        // isolated vertices (here all the vertices)
        MeshBuilder builder( mesh_ ) ;
        builder.delete_vertices( to_delete_geo, false ) ;

        vertex_mapper_.update_mesh_entity_maps_and_gmes( to_delete ) ;
    }

    /*******************************************************************************/

    GeoModelMeshCells::GeoModelMeshCells(
        GeoModelMesh& gmm,
        Mesh& mesh,
        MeshBuilder& mesh_builder )
        :
            gmm_( gmm ),
            gm_( gmm.model() ),
            mesh_( mesh ),
            mesh_builder_( mesh_builder ),
            nb_tet_( 0 ),
            nb_hex_( 0 ),
            nb_prism_( 0 ),
            nb_pyramid_( 0 ),
            nb_connector_( 0 ),
            mode_( NONE )
    {
    }

    bool GeoModelMeshCells::is_initialized() const
    {
        return mesh_.nb_cells() > 0 ;
    }

    void GeoModelMeshCells::test_and_initialize() const
    {
        if( !is_initialized() ) {
            const_cast< GeoModelMeshCells* >( this )->initialize() ;
        }
    }

    void GeoModelMeshCells::initialize()
    {
        gmm_.vertices.test_and_initialize() ;
        region_cell_ptr_.resize( gm_.nb_regions() * GEO::MESH_NB_CELL_TYPES + 1,
            0 ) ;

        // Total number of  cells
        std::vector< index_t > nb_cells_per_type( GEO::MESH_NB_CELL_TYPES, 0 ) ;
        index_t nb = 0 ;

        for( index_t r = 0; r < gm_.nb_regions(); ++r ) {
            nb += gm_.region( r ).nb_mesh_elements() ;
        }

        // Get out if no cells
        if( nb == 0 ) {
            return ;
        }

        // Compute the number of cell per type and per region
        for( index_t r = 0; r < gm_.nb_regions(); ++r ) {
            const Region& cur_region = gm_.region( r ) ;
            for( index_t c = 0; c < gm_.region( r ).nb_mesh_elements(); ++c ) {
                GEO::MeshCellType cur_cell_type = cur_region.cell_type( c ) ;
                switch( cur_cell_type ) {
                    case GEO::MESH_TET:
                        nb_cells_per_type[GEO::MESH_TET]++ ;
                        region_cell_ptr_[GEO::MESH_NB_CELL_TYPES * r + GEO::MESH_TET
                            + 1]++ ;
                        break ;
                    case GEO::MESH_HEX:
                        nb_cells_per_type[GEO::MESH_HEX]++ ;
                        region_cell_ptr_[GEO::MESH_NB_CELL_TYPES * r + GEO::MESH_HEX
                            + 1]++ ;
                        break ;
                    case GEO::MESH_PRISM:
                        nb_cells_per_type[GEO::MESH_PRISM]++ ;
                        region_cell_ptr_[GEO::MESH_NB_CELL_TYPES * r
                            + GEO::MESH_PRISM + 1]++ ;
                        break ;
                    case GEO::MESH_PYRAMID:
                        nb_cells_per_type[GEO::MESH_PYRAMID]++ ;
                        region_cell_ptr_[GEO::MESH_NB_CELL_TYPES * r
                            + GEO::MESH_PYRAMID + 1]++ ;
                        break ;
                    case GEO::MESH_CONNECTOR:
                        nb_cells_per_type[GEO::MESH_CONNECTOR]++ ;
                        region_cell_ptr_[GEO::MESH_NB_CELL_TYPES * r
                            + GEO::MESH_CONNECTOR + 1]++ ;
                        break ;
                    default:
                        ringmesh_assert_not_reached ;
                        break ;
                }
            }
        }

        // Compute the cell offsets
        std::vector< index_t > cells_offset_per_type( GEO::MESH_NB_CELL_TYPES, 0 ) ;
        for( index_t t = GEO::MESH_TET + 1; t < GEO::MESH_NB_CELL_TYPES; t++ ) {
            cells_offset_per_type[t] += cells_offset_per_type[t - 1] ;
            cells_offset_per_type[t] += nb_cells_per_type[t - 1] ;
        }

        for( index_t i = 1; i < region_cell_ptr_.size() - 1; i++ ) {
            region_cell_ptr_[i + 1] += region_cell_ptr_[i] ;
        }

        // Create "empty" tet, hex, pyr and prism
        for( index_t i = 0; i < GEO::MESH_NB_CELL_TYPES; ++i ) {
            mesh_builder_.create_cells( nb_cells_per_type[i],
                GEO::MeshCellType( i ) ) ;
        }

        // Fill the cells with vertices
        bind_attribute() ;
        std::vector< index_t > cur_cell_per_type( GEO::MESH_NB_CELL_TYPES, 0 ) ;
        const GeoModelMeshVertices& model_vertices = gmm_.vertices ;
        for( index_t r = 0; r < gm_.nb_regions(); ++r ) {
            const Region& cur_region = gm_.region( r ) ;
            for( index_t c = 0; c < cur_region.nb_mesh_elements(); ++c ) {
                GEO::MeshCellType cur_cell_type = cur_region.cell_type( c ) ;
                index_t cur_cell = cells_offset_per_type[cur_cell_type]
                    + cur_cell_per_type[cur_cell_type]++ ;
                for( index_t v = 0; v < mesh_.nb_cell_vertices( cur_cell ); v++ ) {
                    index_t region_vertex_index =
                        cur_region.mesh_element_vertex_index( c, v ) ;
                    index_t global_vertex_id = model_vertices.model_vertex_id(
                        cur_region.gme_id(), region_vertex_index ) ;
                    mesh_builder_.set_cell_vertex( cur_cell, v, global_vertex_id ) ;
                }
                region_id_[cur_cell] = r ;
                cell_id_[cur_cell] = c ;
            }
        }

        // Retrieve the adjacencies
        mesh_builder_.connect_cells() ;

        // Permute cells to sort them per region and per type
        GEO::vector< index_t > sorted_indices( mesh_.nb_cells() ) ;
        for( index_t i = 0; i < mesh_.nb_cells(); i++ ) {
            sorted_indices[i] = i ;
        }
        GeoModelMeshCellsSort action( mesh_, region_id_ ) ;
        std::sort( sorted_indices.begin(), sorted_indices.end(), action ) ;
        mesh_builder_.permute_cells( sorted_indices ) ;

        // Cache some values
        nb_tet_ = nb_cells_per_type[GEO::MESH_TET] ;
        nb_hex_ = nb_cells_per_type[GEO::MESH_HEX] ;
        nb_prism_ = nb_cells_per_type[GEO::MESH_PRISM] ;
        nb_pyramid_ = nb_cells_per_type[GEO::MESH_PYRAMID] ;
        nb_connector_ = nb_cells_per_type[GEO::MESH_CONNECTOR] ;
    }

    void GeoModelMeshCells::bind_attribute()
    {
        if( !region_id_.is_bound() ) {
            region_id_.bind( gmm_.cell_attribute_manager(), region_att_name ) ;
        }
        if( !cell_id_.is_bound() ) {
            cell_id_.bind( gmm_.cell_attribute_manager(), cell_region_att_name ) ;
        }
    }

    void GeoModelMeshCells::unbind_attribute()
    {
        if( region_id_.is_bound() ) {
            region_id_.unbind() ;
        }
        if( cell_id_.is_bound() ) {
            cell_id_.unbind() ;
        }
        if( facet_id_.is_bound() ) {
            facet_id_.unbind() ;
        }
    }

    index_t GeoModelMeshCells::nb() const
    {
        test_and_initialize() ;
        return mesh_.nb_cells() ;
    }

    index_t GeoModelMeshCells::nb_vertices( index_t c ) const
    {
        test_and_initialize() ;
        ringmesh_assert( c < mesh_.nb_cells() ) ;
        return mesh_.nb_cell_vertices( c ) ;
    }

    index_t GeoModelMeshCells::vertex( index_t c, index_t v ) const
    {
        test_and_initialize() ;
        ringmesh_assert( c < mesh_.nb_cells() ) ;
        ringmesh_assert( v < mesh_.nb_cell_vertices( c ) ) ;
        return mesh_.cell_vertex( c, v ) ;
    }

    index_t GeoModelMeshCells::nb_edges( index_t c ) const
    {
        test_and_initialize() ;
        ringmesh_assert( c < mesh_.nb_cells() ) ;
        return mesh_.nb_cell_edges( c ) ;
    }

    index_t GeoModelMeshCells::nb_facets( index_t c ) const
    {
        test_and_initialize() ;
        ringmesh_assert( c < mesh_.nb_cells() ) ;
        return mesh_.nb_cell_facets( c ) ;
    }

    index_t GeoModelMeshCells::nb_facet_vertices( index_t c, index_t lf ) const
    {
        test_and_initialize() ;
        ringmesh_assert( c < mesh_.nb_cells() ) ;
        ringmesh_assert( lf < mesh_.nb_cell_facets( c ) ) ;
        return mesh_.nb_cell_facet_vertices( c, lf ) ;
    }

    index_t GeoModelMeshCells::facet_vertex(
        index_t c,
        index_t lf,
        index_t lv ) const
    {
        test_and_initialize() ;
        ringmesh_assert( c < mesh_.nb_cells() ) ;
        ringmesh_assert( lf < mesh_.nb_cell_facets( c ) ) ;
        return mesh_.cell_facet_vertex( c, lf, lv ) ;
    }

    index_t GeoModelMeshCells::edge_vertex( index_t c, index_t le, index_t lv ) const
    {
        geo_debug_assert( le < nb_edges( c ) ) ;
        geo_debug_assert( lv < 2 ) ;
        return mesh_.cell_edge_vertex( c, le, lv ) ;
    }

    index_t GeoModelMeshCells::adjacent( index_t c, index_t f ) const
    {
        test_and_initialize() ;
        ringmesh_assert( c < mesh_.nb_cells() ) ;
        ringmesh_assert( f < mesh_.nb_cell_facets( c ) ) ;
        return mesh_.cell_adjacent( c, f ) ;
    }

    index_t GeoModelMeshCells::region( index_t c ) const
    {
        test_and_initialize() ;
        ringmesh_assert( c < mesh_.nb_cells() ) ;
        return region_id_[c] ;
    }

    index_t GeoModelMeshCells::index_in_region( index_t c ) const
    {
        test_and_initialize() ;
        ringmesh_assert( c < mesh_.nb_cells() ) ;
        return cell_id_[c] ;
    }

    GEO::MeshCellType GeoModelMeshCells::type( index_t c ) const
    {
        test_and_initialize() ;
        ringmesh_assert( c < mesh_.nb_cells() ) ;
        return mesh_.cell_type( c ) ;
    }

    index_t GeoModelMeshCells::nb_cells( GEO::MeshCellType type ) const
    {
        test_and_initialize() ;
        switch( type ) {
            case GEO::MESH_TET:
                return nb_tet() ;
            case GEO::MESH_HEX:
                return nb_hex() ;
            case GEO::MESH_PRISM:
                return nb_prism() ;
            case GEO::MESH_PYRAMID:
                return nb_pyramid() ;
            case GEO::MESH_CONNECTOR:
                return nb_connector() ;
            case GEO::MESH_NB_CELL_TYPES:
                return nb() ;
            default:
                ringmesh_assert_not_reached ;
                return 0 ;
        }
    }

    index_t GeoModelMeshCells::nb_cells( index_t r, GEO::MeshCellType type ) const
    {
        test_and_initialize() ;
        switch( type ) {
            case GEO::MESH_TET:
                return nb_tet( r ) ;
            case GEO::MESH_HEX:
                return nb_hex( r ) ;
            case GEO::MESH_PRISM:
                return nb_prism( r ) ;
            case GEO::MESH_PYRAMID:
                return nb_pyramid( r ) ;
            case GEO::MESH_CONNECTOR:
                return nb_connector( r ) ;
            case GEO::MESH_NB_CELL_TYPES:
                ringmesh_assert( region_cell_ptr_[GEO::MESH_NB_CELL_TYPES * ( r + 1 )]
                    - region_cell_ptr_[GEO::MESH_NB_CELL_TYPES * r]
                    == gm_.region( r ).nb_mesh_elements() ) ;
                return region_cell_ptr_[GEO::MESH_NB_CELL_TYPES * ( r + 1 )]
                    - region_cell_ptr_[GEO::MESH_NB_CELL_TYPES * r] ;
            default:
                ringmesh_assert_not_reached ;
                return 0 ;
        }
    }

    index_t GeoModelMeshCells::cell(
        index_t r,
        index_t c,
        GEO::MeshCellType type ) const
    {
        test_and_initialize() ;
        switch( type ) {
            case GEO::MESH_TET:
                return tet( r, c ) ;
            case GEO::MESH_HEX:
                return hex( r, c ) ;
            case GEO::MESH_PRISM:
                return prism( r, c ) ;
            case GEO::MESH_PYRAMID:
                return pyramid( r, c ) ;
            case GEO::MESH_CONNECTOR:
                return connector( r, c ) ;
            case GEO::MESH_NB_CELL_TYPES:
                return region_cell_ptr_[GEO::MESH_NB_CELL_TYPES * r] + c ;
            default:
                ringmesh_assert_not_reached ;
                return 0 ;
        }
    }

    index_t GeoModelMeshCells::nb_tet() const
    {
        test_and_initialize() ;
        return nb_tet_ ;
    }

    index_t GeoModelMeshCells::nb_tet( index_t r ) const
    {
        test_and_initialize() ;
        ringmesh_assert( r < gm_.nb_regions() ) ;
        return region_cell_ptr_[GEO::MESH_NB_CELL_TYPES * r + ( GEO::MESH_TET + 1 )]
            - region_cell_ptr_[GEO::MESH_NB_CELL_TYPES * r + GEO::MESH_TET] ;
    }

    index_t GeoModelMeshCells::tet( index_t r, index_t t ) const
    {
        test_and_initialize() ;
        ringmesh_assert( r < gm_.nb_regions() ) ;
        return region_cell_ptr_[GEO::MESH_NB_CELL_TYPES * r + GEO::MESH_TET] + t ;
    }

    index_t GeoModelMeshCells::nb_hex() const
    {
        test_and_initialize() ;
        return nb_hex_ ;
    }

    index_t GeoModelMeshCells::nb_hex( index_t r ) const
    {
        test_and_initialize() ;
        ringmesh_assert( r < gm_.nb_regions() ) ;
        return region_cell_ptr_[GEO::MESH_NB_CELL_TYPES * r + ( GEO::MESH_HEX + 1 )]
            - region_cell_ptr_[GEO::MESH_NB_CELL_TYPES * r + GEO::MESH_HEX] ;
    }

    index_t GeoModelMeshCells::hex( index_t r, index_t h ) const
    {
        test_and_initialize() ;
        ringmesh_assert( r < gm_.nb_regions() ) ;
        return region_cell_ptr_[GEO::MESH_NB_CELL_TYPES * r + GEO::MESH_HEX] + h ;
    }

    index_t GeoModelMeshCells::nb_prism() const
    {
        test_and_initialize() ;
        return nb_prism_ ;
    }

    index_t GeoModelMeshCells::nb_prism( index_t r ) const
    {
        test_and_initialize() ;
        ringmesh_assert( r < gm_.nb_regions() ) ;
        return region_cell_ptr_[GEO::MESH_NB_CELL_TYPES * r + ( GEO::MESH_PRISM + 1 )]
            - region_cell_ptr_[GEO::MESH_NB_CELL_TYPES * r + GEO::MESH_PRISM] ;
    }

    index_t GeoModelMeshCells::prism( index_t r, index_t p ) const
    {
        test_and_initialize() ;
        ringmesh_assert( r < gm_.nb_regions() ) ;
        return region_cell_ptr_[GEO::MESH_NB_CELL_TYPES * r + GEO::MESH_PRISM] + p ;
    }

    index_t GeoModelMeshCells::nb_pyramid() const
    {
        test_and_initialize() ;
        return nb_pyramid_ ;
    }

    index_t GeoModelMeshCells::nb_pyramid( index_t r ) const
    {
        test_and_initialize() ;
        ringmesh_assert( r < gm_.nb_regions() ) ;
        return region_cell_ptr_[GEO::MESH_NB_CELL_TYPES * r
            + ( GEO::MESH_PYRAMID + 1 )]
            - region_cell_ptr_[GEO::MESH_NB_CELL_TYPES * r + GEO::MESH_PYRAMID] ;
    }

    index_t GeoModelMeshCells::pyramid( index_t r, index_t p ) const
    {
        test_and_initialize() ;
        ringmesh_assert( r < gm_.nb_regions() ) ;
        return region_cell_ptr_[GEO::MESH_NB_CELL_TYPES * r + GEO::MESH_PYRAMID] + p ;
    }

    index_t GeoModelMeshCells::nb_connector() const
    {
        test_and_initialize() ;
        return nb_connector_ ;
    }

    index_t GeoModelMeshCells::nb_connector( index_t r ) const
    {
        test_and_initialize() ;
        ringmesh_assert( r < gm_.nb_regions() ) ;
        return region_cell_ptr_[GEO::MESH_NB_CELL_TYPES * r
            + ( GEO::MESH_CONNECTOR + 1 )]
            - region_cell_ptr_[GEO::MESH_NB_CELL_TYPES * r + GEO::MESH_CONNECTOR] ;
    }

    index_t GeoModelMeshCells::connector( index_t r, index_t c ) const
    {
        test_and_initialize() ;
        ringmesh_assert( r < gm_.nb_regions() ) ;
        return region_cell_ptr_[GEO::MESH_NB_CELL_TYPES * r + GEO::MESH_CONNECTOR]
            + c ;
    }

    bool GeoModelMeshCells::is_duplication_initialized() const
    {
        return mode_ == gmm_.duplicate_mode() ;
    }

    void GeoModelMeshCells::test_and_initialize_duplication() const
    {
        if( !is_duplication_initialized() ) {
            const_cast< GeoModelMeshCells* >( this )->initialize_duplication() ;
        }
    }

    void GeoModelMeshCells::initialize_duplication()
    {
        test_and_initialize() ;

        /// 1. Get all the corner vertices (a lot of duplicated vertices)
        std::vector< vec3 > corner_vertices(
            mesh_.cell_end( mesh_.nb_cells() - 1 ) ) ;
        for( index_t c = 0; c < mesh_.nb_cells(); c++ ) {
            index_t begin = mesh_.cell_begin( c ) ;
            for( index_t v = 0; v < mesh_.nb_cell_vertices( c ); v++ ) {
                corner_vertices[begin + v] = mesh_.vertex(
                    mesh_.cell_vertex( c, v ) ) ;
            }
        }

        /// 2. Tag all corners to duplicate (vertices on a surface to duplicate)
        std::vector< ActionOnSurface > actions_on_surfaces( gm_.nb_surfaces(),
            SKIP ) ;
        std::vector< bool > is_vertex_to_duplicate( corner_vertices.size(), false ) ;
        {
            ColocaterANN ann( corner_vertices, false ) ;
            for( index_t s = 0; s < gm_.nb_surfaces(); s++ ) {
                if( !is_surface_to_duplicate( s ) ) continue ;
                actions_on_surfaces[s] = TO_PROCESS ;
                const Surface& surface = gm_.surface( s ) ;
                for( index_t v = 0; v < surface.nb_vertices(); v++ ) {
                    std::vector< index_t > colocated_corners ;
                    ann.get_neighbors( surface.vertex( v ), colocated_corners,
                        gm_.epsilon() ) ;
                    for( index_t co = 0; co < colocated_corners.size(); co++ ) {
                        is_vertex_to_duplicate[colocated_corners[co]] = true ;
                    }
                }
            }
        }
        // Free some memory
        corner_vertices.clear() ;

        /// 3. Duplicate the corners
        /* The goal is to visit the corners of the GeoModelMesh
         * that are on one side of a Surface. We propagate through the cells
         * that have one vertex on a Surface without crossing the Surface.
         * All the corners visited during this propagation around the vertex
         * are duplicated if needed.
         */
        gmm_.facets.test_and_initialize() ;
        for( index_t c = 0; c < mesh_.nb_cells(); c++ ) {
            for( index_t v = 0; v < mesh_.nb_cell_vertices( c ); v++ ) {
                // get the index of the corner inside cell_corners_
                index_t co = mesh_.cell_begin( c ) + v ;

                if( !is_vertex_to_duplicate[co] ) continue ;
                // The vertex is on a surface to duplicate

                // Propagate on the cells around the corresponding vertex.
                // The propagation process cannot cross any surface.
                index_t vertex_id = mesh_.cell_vertex( c, v ) ;

                // all the cell corners resulting of the propagation
                std::vector< index_t > corner_used ;

                // all cells used during the propagation, used to provide
                // adding the same cell several times into the stack
                std::vector< index_t > cell_added ;

                // all the surfaces encountered during the propagation
                // and which side stopped the propagation
                std::vector< action_on_surface > surfaces ;

                // stack of the front of cells
                std::stack< index_t > S ;
                S.push( c ) ;
                cell_added.push_back( c ) ;
                do {
                    index_t cur_c = S.top() ;
                    S.pop() ;
                    // Find which corner of the current cell matches vertex_id
                    index_t cur_co = mesh_.find_cell_corner( cur_c, vertex_id ) ;
                    ringmesh_assert( cur_co != NO_ID ) ;
                    is_vertex_to_duplicate[cur_co] = false ;
                    corner_used.push_back( cur_co ) ;

                    // Find the cell facets including the vertex
                    std::vector< index_t > facets ;
                    cell_facets_around_vertex( mesh_, cur_c, vertex_id, facets ) ;
                    for( index_t cur_f = 0; cur_f < facets.size(); cur_f++ ) {
                        // Find if the facet is on a surface or inside the domain
                        index_t facet = NO_ID ;
                        bool side ;
                        if( is_cell_facet_on_surface( cur_c, cur_f, facet, side ) ) {
                            index_t surface_id = gmm_.facets.surface( facet ) ;
                            surfaces.push_back(
                                action_on_surface( surface_id,
                                    ActionOnSurface( side ) ) ) ;
                        } else {
                            // The cell facet is not on a surface.
                            // Add the adjacent cell to the stack if it exists
                            // and has not already been processed or added into the stack
                            index_t cur_adj = mesh_.cell_adjacent( cur_c, cur_f ) ;
                            if( cur_adj != GEO::NO_CELL
                                && !contains( cell_added, cur_adj ) ) {
                                cell_added.push_back( cur_adj ) ;
                                S.push( cur_adj ) ;
                            }
                        }
                    }
                } while( !S.empty() ) ;

                // Remove redundant occurrences and sort the remaining ones
                GEO::sort_unique( surfaces ) ;

                // Determine if the corners should be duplicated or not because
                // we need to duplicate only one side of the surface
                if( are_corners_to_duplicate( surfaces, actions_on_surfaces ) ) {
                    // Add a new duplicated vertex and its associated vertex

                    /* @todo Review : Use the total_nb_vertices function [JP]
                     * why mm_.vertices.nb_vertices() and not nb_vertices() ?
                     * Please help the reader !! same thing 2 lines below [JP]
                     */
                    index_t duplicated_vertex_id =
                        gmm_.vertices.nb()
                            + static_cast< index_t >( duplicated_vertex_indices_.size() ) ;
                    duplicated_vertex_indices_.push_back( vertex_id ) ;

                    // Update all the cell corners on this side of the surface
                    // to the new duplicated vertex index
                    for( index_t cur_co = 0; cur_co < corner_used.size();
                        cur_co++ ) {
                        mesh_builder_.set_cell_corner_vertex_index(
                            corner_used[cur_co], duplicated_vertex_id ) ;
                    }
                }
            }
        }
        mode_ = gmm_.duplicate_mode() ;
    }

    bool GeoModelMeshCells::is_cell_facet_on_surface(
        index_t c,
        index_t f,
        index_t& facet,
        bool& side ) const
    {
        test_and_initialize_cell_facet() ;
        facet = facet_id_[mesh_.cell_facet( c, f )] ;
        if( facet != NO_ID ) {
            vec3 facet_normal = mesh_.facet_normal( facet ) ;
            vec3 cell_facet_normal = mesh_.cell_facet_normal( c, f ) ;
            side = dot( facet_normal, cell_facet_normal ) > 0 ;
        }
        return facet != NO_ID ;
    }

    bool GeoModelMeshCells::are_corners_to_duplicate(
        const std::vector< action_on_surface >& surfaces,
        std::vector< ActionOnSurface >& info )
    {
        // Temporary vector, it is equal to surfaces
        std::vector< action_on_surface > temp_surfaces ;
        temp_surfaces.reserve( temp_surfaces.size() ) ;

        // Find if a free border was found, if so we encountered the
        // two sides of the surface during the propagation around a vertex
        for( index_t i = 1; i < surfaces.size(); i++ ) {
            if( surfaces[i - 1].first == surfaces[i].first ) {
                // Found free border -> skip
                ringmesh_assert( surfaces[i - 1].second != surfaces[i].second ) ;
                i++ ; // skip the action_on_surface (i-1) and i
            } else {
                temp_surfaces.push_back( surfaces[i - 1] ) ;
            }
        }
        temp_surfaces.push_back( surfaces.back() ) ;

        for( index_t i = 0; i < temp_surfaces.size(); i++ ) {
            index_t s = temp_surfaces[i].first ;
            switch( info[s] ) {
                case SKIP:
                    break ;
                case TO_PROCESS:
                    // First time we encounter this surface, do not duplicate
                    // this side but wait to see if we encounter the other.
                    // In the case of surfaces in the VOI, it is encountered only once
                    ringmesh_assert( temp_surfaces[i].second > TO_PROCESS ) ;
                    info[s] = ActionOnSurface( !temp_surfaces[i].second ) ;
                    break ;
                default:
                    // If the side matches -> duplicate
                    if( info[s] == temp_surfaces[i].second ) {
                        return true ;
                    }
            }
        }

        return false ;
    }

    bool GeoModelMeshCells::is_surface_to_duplicate( index_t surface_id ) const
    {
        if( gm_.surface( surface_id ).is_on_voi() ) return false ;
        switch( gmm_.duplicate_mode() ) {
            case ALL:
                return true ;
            case FAULT: {
                GeoModelEntity::GEOL_FEATURE feature =
                    gm_.surface( surface_id ).geological_feature() ;
                return GME::is_fault( feature ) ;
            }
            default:
                return false ;
        }
        return false ;
    }

    index_t GeoModelMeshCells::nb_duplicated_vertices() const
    {
        test_and_initialize_duplication() ;
        return static_cast< index_t >( duplicated_vertex_indices_.size() ) ;
    }

    index_t GeoModelMeshCells::nb_total_vertices() const
    {
        return nb_duplicated_vertices() + mesh_.nb_vertices() ;
    }

    bool GeoModelMeshCells::is_corner_duplicated(
        index_t c,
        index_t v,
        index_t& duplicate_vertex_index ) const
    {
        test_and_initialize_duplication() ;
        ringmesh_assert( c < mesh_.nb_cells() ) ;
        ringmesh_assert( v < mesh_.nb_cell_vertices( c ) ) ;
        index_t corner_value = mesh_.cell_vertex( c, v ) ;
        if( corner_value < mesh_.nb_vertices() ) {
            return false ;
        } else {
            duplicate_vertex_index = corner_value - mesh_.nb_vertices() ;
            return true ;
        }
    }

    index_t GeoModelMeshCells::duplicated_vertex(
        index_t duplicate_vertex_index ) const
    {
        test_and_initialize_duplication() ;
        ringmesh_assert( duplicate_vertex_index < duplicated_vertex_indices_.size() ) ;
        return duplicated_vertex_indices_[duplicate_vertex_index] ;
    }

    void GeoModelMeshCells::clear()
    {
        mesh_builder_.clear_cells( true, false ) ;
        region_cell_ptr_.clear() ;
        nb_tet_ = 0 ;
        nb_hex_ = 0 ;
        nb_prism_ = 0 ;
        nb_pyramid_ = 0 ;
        nb_connector_ = 0 ;

        mode_ = NONE ;
        duplicated_vertex_indices_.clear() ;
    }

    void GeoModelMeshCells::clear_duplication()
    {
        for( index_t c = 0; c < mesh_.nb_cells(); c++ ) {
            for( index_t v = 0; v < mesh_.nb_cell_vertices( c ); v++ ) {
                index_t index = NO_ID ;
                if( is_corner_duplicated( c, v, index ) ) {
                    mesh_builder_.set_cell_corner_vertex_index( c,
                        duplicated_vertex( index ) ) ;
                }
            }
        }

        mode_ = NONE ;
        duplicated_vertex_indices_.clear() ;
    }

    void GeoModelMeshCells::test_and_initialize_cell_facet() const
    {
        if( !facet_id_.is_bound() ) {
            const_cast< GeoModelMeshCells* >( this )->initialize_cell_facet() ;
        }
    }

    void GeoModelMeshCells::initialize_cell_facet()
    {
        gmm_.facets.test_and_initialize() ;

        facet_id_.bind( mesh_.cell_facet_attribute_manager(), "facet_id" ) ;
        facet_id_.fill( NO_ID ) ;
        const ColocaterANN& ann = mesh_.colocater_ann( ColocaterANN::FACETS ) ;
        for( index_t c = 0; c < mesh_.nb_cells(); c++ ) {
            for( index_t f = 0; f < mesh_.nb_cell_facets( c ); f++ ) {
                std::vector< index_t > result ;
                if( ann.get_neighbors( mesh_.cell_facet_barycenter( c, f ),
                    result, gm_.epsilon() ) ) {
                    facet_id_[mesh_.cell_facet( c, f )] = result[0] ;
                    // If there are more than 1 matching facet, this is WRONG
                    // and the vertex indices should be checked too [Jeanne]
                    ringmesh_assert( result.size() == 1 ) ;
                }
            }
        }
    }

    vec3 GeoModelMeshCells::barycenter( index_t c ) const
    {
        test_and_initialize() ;
        return mesh_.cell_barycenter( c ) ;
    }

    double GeoModelMeshCells::volume( index_t c ) const
    {
        test_and_initialize() ;
        return mesh_.cell_volume( c ) ;
    }

    /*******************************************************************************/

    GeoModelMeshFacets::GeoModelMeshFacets(
        GeoModelMesh& gmm,
        Mesh& mesh,
        MeshBuilder& mesh_builder )
        :
            gmm_( gmm ),
            gm_( gmm.model() ),
            mesh_( mesh ),
            mesh_builder_( mesh_builder ),
            nb_triangle_( 0 ),
            nb_quad_( 0 ),
            nb_polygon_( 0 )
    {
    }

    GeoModelMeshFacets::~GeoModelMeshFacets()
    {
        unbind_attribute() ;
    }

    void GeoModelMeshFacets::bind_attribute()
    {
        if( !surface_id_.is_bound() ) {
            surface_id_.bind( gmm_.facet_attribute_manager(), surface_att_name ) ;
        }
        if( !facet_id_.is_bound() ) {
            facet_id_.bind( gmm_.facet_attribute_manager(),
                facet_surface_att_name ) ;
        }

    }

    void GeoModelMeshFacets::unbind_attribute()
    {
        if( surface_id_.is_bound() ) {
            surface_id_.unbind() ;
        }
        if( facet_id_.is_bound() ) {
            facet_id_.unbind() ;
        }

    }

    bool GeoModelMeshFacets::is_initialized() const
    {
        return mesh_.nb_facets() > 0 ;
    }

    index_t GeoModelMeshFacets::nb() const
    {
        test_and_initialize() ;
        return mesh_.nb_facets() ;
    }

    index_t GeoModelMeshFacets::nb_vertices( index_t f ) const
    {
        test_and_initialize() ;
        ringmesh_assert( f < mesh_.nb_facets() ) ;
        return mesh_.nb_facet_vertices( f ) ;
    }

    index_t GeoModelMeshFacets::vertex( index_t f, index_t v ) const
    {
        test_and_initialize() ;
        ringmesh_assert( f < mesh_.nb_facets() ) ;
        ringmesh_assert( v < mesh_.nb_facet_vertices( f ) ) ;
        return mesh_.facet_vertex( f, v ) ;
    }

    index_t GeoModelMeshFacets::adjacent( index_t f, index_t e ) const
    {
        test_and_initialize() ;
        ringmesh_assert( f < mesh_.nb_facets() ) ;
        ringmesh_assert( e < mesh_.nb_facet_vertices( f ) ) ;
        return mesh_.facet_adjacent( f, e ) ;
    }

    index_t GeoModelMeshFacets::surface( index_t f ) const
    {
        test_and_initialize() ;
        ringmesh_assert( f < mesh_.nb_facets() ) ;
        return surface_id_[f] ;
    }

    index_t GeoModelMeshFacets::index_in_surface( index_t f ) const
    {
        test_and_initialize() ;
        ringmesh_assert( f < mesh_.nb_facets() ) ;
        return facet_id_[f] ;
    }

    GeoModelMeshFacets::FacetType GeoModelMeshFacets::type(
        index_t f,
        index_t& index ) const
    {
        test_and_initialize() ;
        ringmesh_assert( f < mesh_.nb_facets() ) ;
        index_t facet = index_in_surface( f ) ;
        index_t s = surface( f ) ;
        for( index_t t = TRIANGLE; t < ALL; t++ ) {
            FacetType T = static_cast< FacetType >( t ) ;
            if( facet < nb_facets( s, T ) ) {
                index = facet ;
                return T ;
            }
            facet -= nb_facets( s, T ) ;
        }
        index = NO_ID ;
        ringmesh_assert_not_reached ;
        return NO_FACET ;
    }

    index_t GeoModelMeshFacets::nb_facets( FacetType type ) const
    {
        test_and_initialize() ;
        switch( type ) {
            case TRIANGLE:
                return nb_triangle() ;
            case QUAD:
                return nb_quad() ;
            case POLYGON:
                return nb_polygon() ;
            case ALL:
                return nb() ;
            default:
                ringmesh_assert_not_reached ;
                return 0 ;
        }
    }

    index_t GeoModelMeshFacets::nb_facets( index_t s, FacetType type ) const
    {
        test_and_initialize() ;
        ringmesh_assert( s < gm_.nb_surfaces() ) ;
        switch( type ) {
            case TRIANGLE:
                return nb_triangle( s ) ;
            case QUAD:
                return nb_quad( s ) ;
            case POLYGON:
                return nb_polygon( s ) ;
            case ALL:
                return surface_facet_ptr_[ALL * ( s + 1 )]
                    - surface_facet_ptr_[ALL * s] ;
            default:
                ringmesh_assert_not_reached ;
                return 0 ;
        }
    }

    index_t GeoModelMeshFacets::facet( index_t s, index_t f, FacetType type ) const
    {
        test_and_initialize() ;
        ringmesh_assert( s < gm_.nb_surfaces() ) ;
        switch( type ) {
            case TRIANGLE:
                return triangle( s, f ) ;
            case QUAD:
                return quad( s, f ) ;
            case POLYGON:
                return polygon( s, f ) ;
            case ALL:
                return surface_facet_ptr_[ALL * s] + f ;
            default:
                ringmesh_assert_not_reached ;
                return 0 ;
        }
    }

    index_t GeoModelMeshFacets::nb_triangle() const
    {
        test_and_initialize() ;
        return nb_triangle_ ;
    }

    index_t GeoModelMeshFacets::nb_triangle( index_t s ) const
    {
        test_and_initialize() ;
        ringmesh_assert( s < gm_.nb_surfaces() ) ;
        return surface_facet_ptr_[ALL * s + ( TRIANGLE + 1 )]
            - surface_facet_ptr_[ALL * s + TRIANGLE] ;
    }

    index_t GeoModelMeshFacets::triangle( index_t s, index_t t ) const
    {
        test_and_initialize() ;
        ringmesh_assert( s < gm_.nb_surfaces() ) ;
        return surface_facet_ptr_[ALL * s + TRIANGLE] + t ;
    }

    index_t GeoModelMeshFacets::nb_quad() const
    {
        test_and_initialize() ;
        return nb_quad_ ;
    }

    index_t GeoModelMeshFacets::nb_quad( index_t s ) const
    {
        test_and_initialize() ;
        ringmesh_assert( s < gm_.nb_surfaces() ) ;
        return surface_facet_ptr_[ALL * s + ( QUAD + 1 )]
            - surface_facet_ptr_[ALL * s + QUAD] ;
    }

    index_t GeoModelMeshFacets::quad( index_t s, index_t q ) const
    {
        test_and_initialize() ;
        ringmesh_assert( s < gm_.nb_surfaces() ) ;
        return surface_facet_ptr_[ALL * s + QUAD] + q ;
    }

    index_t GeoModelMeshFacets::nb_polygon() const
    {
        test_and_initialize() ;
        return nb_polygon_ ;
    }

    index_t GeoModelMeshFacets::nb_polygon( index_t s ) const
    {
        test_and_initialize() ;
        ringmesh_assert( s < gm_.nb_surfaces() ) ;
        return surface_facet_ptr_[ALL * s + ( POLYGON + 1 )]
            - surface_facet_ptr_[ALL * s + POLYGON] ;
    }

    index_t GeoModelMeshFacets::polygon( index_t s, index_t p ) const
    {
        test_and_initialize() ;
        ringmesh_assert( s < gm_.nb_surfaces() ) ;
        return surface_facet_ptr_[ALL * s + POLYGON] + p ;
    }

    void GeoModelMeshFacets::clear()
    {
        surface_facet_ptr_.clear() ;
        nb_triangle_ = 0 ;
        nb_quad_ = 0 ;
        mesh_builder_.clear_facets( true, false ) ;
    }

    void GeoModelMeshFacets::test_and_initialize() const
    {
        if( !is_initialized() ) {
            const_cast< GeoModelMeshFacets* >( this )->initialize() ;
        }
    }

    void GeoModelMeshFacets::initialize()
    {
        gmm_.vertices.test_and_initialize() ;
        clear() ;
        surface_facet_ptr_.resize( gm_.nb_surfaces() * ALL + 1, 0 ) ;

        // Compute the total number of facets per type and per surface
        std::vector< index_t > nb_facet_per_type( ALL, 0 ) ;
        for( index_t s = 0; s < gm_.nb_surfaces(); s++ ) {
            const Surface& surface = gm_.surface( s ) ;
            if( surface.is_simplicial() ) {
                nb_facet_per_type[TRIANGLE] += surface.nb_mesh_elements() ;
                surface_facet_ptr_[ALL * s + TRIANGLE + 1] +=
                    surface.nb_mesh_elements() ;
            } else {
                for( index_t f = 0; f < surface.nb_mesh_elements(); f++ ) {
                    switch( surface.nb_mesh_element_vertices( f ) ) {
                        case 3:
                            nb_facet_per_type[TRIANGLE]++ ;
                            surface_facet_ptr_[ALL * s + TRIANGLE + 1]++ ;
                            break ;
                        case 4:
                            nb_facet_per_type[QUAD]++ ;
                            surface_facet_ptr_[ALL * s + QUAD + 1]++ ;
                            break ;
                        default:
                            nb_facet_per_type[POLYGON]++ ;
                            surface_facet_ptr_[ALL * s + POLYGON + 1]++ ;
                            break ;
                    }
                }
            }
        }

        // Create triangles and quads, the polygons will be handle later
        if( nb_facet_per_type[TRIANGLE] ) {
            mesh_builder_.create_facet_triangles( nb_facet_per_type[TRIANGLE] ) ;
        }
        if( nb_facet_per_type[QUAD] ) {
            mesh_builder_.create_facet_quads( nb_facet_per_type[QUAD] ) ;
        }

        // Compute the facet offset
        std::vector< index_t > facet_offset_per_type( ALL, 0 ) ;
        for( index_t t = TRIANGLE + 1; t < ALL; t++ ) {
            facet_offset_per_type[t] += facet_offset_per_type[t - 1] ;
            facet_offset_per_type[t] += nb_facet_per_type[t - 1] ;
        }
        for( index_t i = 1; i < surface_facet_ptr_.size() - 1; i++ ) {
            surface_facet_ptr_[i + 1] += surface_facet_ptr_[i] ;
        }

        // Fill the triangles and quads created above
        // Create and fill polygons
        bind_attribute() ;
        const GeoModelMeshVertices& model_vertices = gmm_.vertices ;
        std::vector< index_t > cur_facet_per_type( ALL, 0 ) ;
        for( index_t s = 0; s < gm_.nb_surfaces(); s++ ) {
            const Surface& surface = gm_.surface( s ) ;
            for( index_t f = 0; f < surface.nb_mesh_elements(); f++ ) {
                index_t nb_vertices = surface.nb_mesh_element_vertices( f ) ;
                index_t cur_facet = NO_ID ;
                if( nb_vertices < 5 ) {
                    FacetType T = static_cast< FacetType >( nb_vertices - 3 ) ;
                    cur_facet = facet_offset_per_type[T] + cur_facet_per_type[T]++ ;
                    for( index_t v = 0; v < nb_vertices; v++ ) {
                        index_t v_id = model_vertices.model_vertex_id( surface.gme_id(), f,
                            v ) ;
                        ringmesh_assert( v_id != NO_ID ) ;
                        mesh_builder_.set_facet_vertex( cur_facet, v, v_id ) ;
                    }
                } else {
                    GEO::vector< index_t > vertices( nb_vertices ) ;
                    for( index_t v = 0; v < nb_vertices; v++ ) {
                        vertices[v] = model_vertices.model_vertex_id(
                            surface.gme_id(), f, v ) ;
                    }
                    cur_facet = mesh_builder_.create_facet_polygon( vertices ) ;
                }
                surface_id_[cur_facet] = s ;
                facet_id_[cur_facet] = f ;
            }
        }

        // Compute facet adjacencies
        mesh_builder_.connect_facets() ;

        // Permute facets to sort them per surface and per type
        // Example for a mesh with two surfaces and only triangles and quads
        // [TRGL,TRGL, .. , QUAD, QUAD .. , TRGL, TRGL, ... , QUAD, QUAD ..]
        // |          surface 0           |             surface 1           |
        GEO::vector< index_t > sorted_indices( mesh_.nb_facets() ) ;
        for( index_t i = 0; i < mesh_.nb_facets(); i++ ) {
            sorted_indices[i] = i ;
        }
        GeoModelMeshFacetsSort action( mesh_, surface_id_ ) ;
        std::sort( sorted_indices.begin(), sorted_indices.end(), action ) ;
        mesh_builder_.permute_facets( sorted_indices ) ;

        // Cache some values
        nb_triangle_ = nb_facet_per_type[TRIANGLE] ;
        nb_quad_ = nb_facet_per_type[QUAD] ;
        nb_polygon_ = nb_facet_per_type[POLYGON] ;
    }

    vec3 GeoModelMeshFacets::center( index_t f ) const
    {
        test_and_initialize() ;
        return mesh_.facet_barycenter( f ) ;
    }

    vec3 GeoModelMeshFacets::normal( index_t f ) const
    {
        test_and_initialize() ;
        return mesh_.facet_normal( f ) ;
    }

    double GeoModelMeshFacets::area( index_t f ) const
    {
        test_and_initialize() ;
        return mesh_.facet_area( f ) ;
    }
    /*******************************************************************************/

    GeoModelMeshEdges::GeoModelMeshEdges(
        GeoModelMesh& gmm,
        Mesh& mesh,
        MeshBuilder& mesh_builder )
        :
            gmm_( gmm ),
            gm_( gmm.model() ),
            mesh_( mesh ),
            mesh_builder_( mesh_builder )
    {
    }

    GeoModelMeshEdges::~GeoModelMeshEdges()
    {
    }

    index_t GeoModelMeshEdges::nb_wells() const
    {
        test_and_initialize() ;
        return gm_.wells() ? gm_.wells()->nb_wells() : 0 ;
    }

    index_t GeoModelMeshEdges::nb_edges() const
    {
        test_and_initialize() ;
        return mesh_.nb_edges() ;
    }

    index_t GeoModelMeshEdges::nb_edges( index_t w ) const
    {
        test_and_initialize() ;
        return well_ptr_[w + 1] - well_ptr_[w] ;
    }

    index_t GeoModelMeshEdges::vertex( index_t w, index_t e, index_t v ) const
    {
        test_and_initialize() ;
        return mesh_.edge_vertex( well_ptr_[w] + e, v ) ;
    }

    void GeoModelMeshEdges::clear()
    {
        mesh_builder_.clear_edges( true, false ) ;
        well_ptr_.clear() ;
    }

    bool GeoModelMeshEdges::is_initialized() const
    {
        return mesh_.nb_edges() > 0 ;
    }

    void GeoModelMeshEdges::test_and_initialize() const
    {
        if( !is_initialized() ) {
            const_cast< GeoModelMeshEdges* >( this )->initialize() ;
        }
    }

    void GeoModelMeshEdges::initialize()
    {
        if( !gm_.wells() ) return ;
        gmm_.vertices.test_and_initialize() ;
        clear() ;

        // Compute the total number of edge per well
        const WellGroup& wells = *gm_.wells() ;
        well_ptr_.resize( wells.nb_wells() + 1, 0 ) ;
        index_t nb_edges = 0 ;
        for( index_t w = 0; w < wells.nb_wells(); w++ ) {
            nb_edges += wells.well( w ).nb_edges() ;
            well_ptr_[w + 1] = nb_edges ;
        }

        // Compute the edge offset
        for( index_t i = 1; i < well_ptr_.size() - 1; i++ ) {
            well_ptr_[i + 1] += well_ptr_[i] ;
        }

        // Create edges
        mesh_builder_.create_edges( well_ptr_.back() ) ;

        // Fill edges
        index_t cur_edge = 0 ;
        for( index_t w = 0; w < wells.nb_wells(); w++ ) {
            const Well& well = wells.well( w ) ;
            for( index_t p = 0; p < well.nb_parts(); p++ ) {
                const GEO::Mesh& part = well.part( p ).mesh() ;
                for( index_t e = 0; e < well.part( p ).nb_edges(); e++ ) {
                    const vec3& e0 = GEO::Geom::mesh_vertex( part,
                        part.edges.vertex( e, 0 ) ) ;
                    mesh_builder_.set_edge_vertex( cur_edge, 0,
                        gmm_.vertices.index( e0 ) ) ;
                    const vec3& e1 = GEO::Geom::mesh_vertex( part,
                        part.edges.vertex( e, 1 ) ) ;
                    mesh_builder_.set_edge_vertex( cur_edge, 1,
                        gmm_.vertices.index( e1 ) ) ;
                    cur_edge++ ;
                }
            }
        }
    }

    /*******************************************************************************/

    GeoModelMeshOrder::GeoModelMeshOrder( GeoModelMesh& gmm, Mesh& mesh )
        :
            gmm_( gmm ),
            gm_( gmm.model() ),
            mesh_( mesh ),
            nb_vertices_( 0 ),
            high_order_vertices_( 0 ),
            max_new_points_on_cell_( 0 ),
            max_new_points_on_facet_( 0 )
    {
        for( index_t i = 0; i < 4; i++ ) {
            nb_high_order_points_per_cell_type_[i] = 0 ;
        }

        for( index_t i = 0; i < 2; i++ ) {
            nb_high_order_points_per_facet_type_[i] = 0 ;
        }
    }

    void GeoModelMeshOrder::initialize()
    {

        index_t offset = 0 ;
        nb_vertices_ = gmm_.vertices.nb() ;
        ;
        index_t order = gmm_.get_order() ;
        if( order != 1 ) {

            index_t nb_total_edges = 0 ;

            if( gmm_.cells.nb_cells() == gmm_.cells.nb_tet() ) {
                max_new_points_on_cell_ = 6 * ( order - 1 ) ;
                max_new_points_on_facet_ = 3 * ( order - 1 ) ;
            } else {
                max_new_points_on_cell_ = 12 * ( order - 1 ) ;
                max_new_points_on_facet_ = 4 * ( order - 1 ) ;
            }

            GEO::Attribute< index_t > order_vertices_cell ;
            ( gmm_.cell_attribute_manager(), order_att_name ) ;
            order_vertices_cell.create_vector_attribute(
                gmm_.cell_attribute_manager(), order_att_name,
                max_new_points_on_cell_ ) ;
            GEO::Attribute< index_t > order_vertices_facet ;
            order_vertices_facet.create_vector_attribute(
                gmm_.facet_attribute_manager(), order_att_name,
                max_new_points_on_facet_ ) ;

            /// First loop to find a maximum number of new points
            for( index_t r = 0; r < gm_.nb_regions(); r++ ) {
                for( index_t c = 0; c < gmm_.cells.nb(); c++ ) {
                    for( index_t e = 0; e < gmm_.cells.nb_edges( c ); e++ ) {
                        nb_total_edges++ ;
                    }
                }
            }

            /// Fill nb_added_points_per_cell_type_ with the number of high order vertices
            nb_high_order_points_per_cell_type_[GEO::MESH_TET] = 6 * ( order - 1 ) ;
            nb_high_order_points_per_cell_type_[GEO::MESH_HEX] = 12 * ( order - 1 ) ;
            nb_high_order_points_per_cell_type_[GEO::MESH_PYRAMID] = 8
                * ( order - 1 ) ;
            nb_high_order_points_per_cell_type_[GEO::MESH_PRISM] = 9
                * ( order - 1 ) ;

            /// Fill nb_added_points_per_facet_type_ with the number of high order vertices
            nb_high_order_points_per_facet_type_[0] = 3 * ( order - 1 ) ;
            nb_high_order_points_per_facet_type_[1] = 4 * ( order - 1 ) ;

            std::vector< vec3 > new_points( nb_total_edges * ( order - 1 ) ) ;

            /// Adding new indices on cells edges
            for( index_t c = 0; c < gmm_.cells.nb(); c++ ) {
                for( index_t e = 0; e < gmm_.cells.nb_edges( c ); e++ ) {
                    std::vector< vec3 > new_points_in_edge ;
                    vec3 node0 = gmm_.vertices.vertex(
                        gmm_.cells.edge_vertex( c, e, 0 ) ) ;
                    vec3 node1 = gmm_.vertices.vertex(
                        gmm_.cells.edge_vertex( c, e, 1 ) ) ;
                    divide_edge_in_parts( node0, node1, order, new_points_in_edge ) ;

                    for( index_t v = 0; v < new_points_in_edge.size(); v++ ) {
                        new_points[offset] = new_points_in_edge[v] ;
                        order_vertices_cell[c * max_new_points_on_cell_ + e + v] =
                            offset ;
                        offset++ ;
                    }
                }
            }

            MakeUnique uniq( new_points ) ;
            uniq.unique( gm_.epsilon() ) ;
            std::vector< vec3 > uniq_points ;
            uniq.unique_points( uniq_points ) ;
            std::vector< index_t > map = uniq.indices() ;
            ColocaterANN ann( uniq_points, false ) ;

            /// Rewriting the right new indices on the cell attribute
            for( index_t c = 0; c < gmm_.cells.nb(); c++ ) {
                for( index_t v = 0; v < gmm_.cells.nb_edges( c ) * ( order - 1 );
                    v++ ) {
                    order_vertices_cell[c * max_new_points_on_cell_ + v] =
                        map[order_vertices_cell[c * max_new_points_on_cell_ + v]]
                            + nb_vertices_ ;
                }
            }

            /// Writing new indices on an attribute for the facet
            for( index_t f = 0; f < gmm_.facets.nb(); f++ ) {
                for( index_t e = 0; e < gmm_.facets.nb_vertices( f ); e++ ) {
                    vec3 node0 ;
                    vec3 node1 ;
                    std::vector< vec3 > new_points_in_edge ;
                    if( e == gmm_.facets.nb_vertices( f ) - 1 ) {
                        node0 = gmm_.vertices.vertex( gmm_.facets.vertex( f, e ) ) ;
                        node1 = gmm_.vertices.vertex( gmm_.facets.vertex( f, 0 ) ) ;
                    } else {
                        node0 = gmm_.vertices.vertex( gmm_.facets.vertex( f, e ) ) ;
                        node1 = gmm_.vertices.vertex(
                            gmm_.facets.vertex( f, e + 1 ) ) ;
                    }
                    divide_edge_in_parts( node0, node1, order, new_points_in_edge ) ;
                    for( index_t v = 0; v < new_points_in_edge.size(); v++ ) {
                        std::vector< index_t > colocated_vertices ;
                        ringmesh_assert( colocated_vertices.size() == 1 ) ;

                        order_vertices_facet[f * max_new_points_on_facet_ + e + v] =
                            colocated_vertices[0] + nb_vertices_ ;

                    }
                }
            }
            nb_vertices_ += static_cast< index_t >( uniq_points.size() ) ;
        }

    }

    void GeoModelMeshOrder::test_and_initialize() const
    {
        if( !is_initialized() ) {
            const_cast< GeoModelMeshOrder* >( this )->initialize() ;
        }
    }

    bool GeoModelMeshOrder::is_initialized() const
    {
        return nb_vertices_ > 0 ;
    }

    void GeoModelMeshOrder::clear()
    {
        nb_vertices_ = 0 ;
    }

    index_t GeoModelMeshOrder::nb_total_vertices() const
    {
        test_and_initialize() ;
        return nb_vertices_ ;
    }

    const vec3& GeoModelMeshOrder::vertex( const index_t index ) const
    {
        const_cast< GeoModelMeshOrder* >( this )->test_point_list_initialized() ;
        return high_order_vertices_[index] ;
    }

    index_t GeoModelMeshOrder::indice_on_cell( index_t c, index_t component ) const
    {
        test_and_initialize() ;
        ringmesh_assert( c < gmm_.cells.nb() ) ;
        ringmesh_assert( component < max_new_points_on_cell_ ) ;
        GEO::Attribute< index_t > order_vertices_cell ;
        ( gmm_.cell_attribute_manager(), order_att_name ) ;
        return order_vertices_cell[max_new_points_on_cell_ * c + component] ;
    }

    index_t GeoModelMeshOrder::indice_on_facet( index_t f, index_t component ) const
    {
        test_and_initialize() ;
        ringmesh_assert( f < gmm_.facets.nb_facets() ) ;
        ringmesh_assert( component < max_new_points_on_cell_ ) ;
        GEO::Attribute< index_t > order_vertices_facet ;
        ( gmm_.facet_attribute_manager(), order_att_name ) ;
        return order_vertices_facet[max_new_points_on_cell_ * f + component] ;
    }

    void GeoModelMeshOrder::move_point( const index_t index, const vec3& u )
    {
        test_point_list_initialized() ;
        for( index_t i = 0; i < 3; i++ ) {
            high_order_vertices_[index][i] += u[i] ;
        }
    }

    void GeoModelMeshOrder::test_point_list_initialized()
    {
        if( high_order_vertices_.empty() ) {
            index_t order = gmm_.get_order() ;

            if( high_order_vertices_.empty() && order > 1 ) {
                index_t offset = 0 ;
                index_t nb_total_edges = 0 ;
                for( index_t c = 0; c < gmm_.cells.nb(); c++ ) {
                    for( index_t e = 0; e < gmm_.cells.nb_edges( c ); e++ ) {
                        nb_total_edges++ ;
                    }
                }

                std::vector< vec3 > new_points( nb_total_edges * ( order - 1 ) ) ;
                for( index_t c = 0; c < gmm_.cells.nb(); c++ ) {
                    for( index_t e = 0; e < gmm_.cells.nb_edges( c ); e++ ) {
                        std::vector< vec3 > new_points_in_edge ;
                        vec3 node0 = gmm_.vertices.vertex(
                            gmm_.cells.edge_vertex( c, e, 0 ) ) ;
                        vec3 node1 = gmm_.vertices.vertex(
                            gmm_.cells.edge_vertex( c, e, 1 ) ) ;
                        divide_edge_in_parts( node0, node1, order,
                            new_points_in_edge ) ;

                        for( index_t v = 0; v < new_points_in_edge.size(); v++ ) {
                            new_points[offset] = new_points_in_edge[v] ;
                            offset++ ;
                        }
                    }
                }

                MakeUnique uniq( new_points ) ;
                uniq.unique( gm_.epsilon() ) ;
                uniq.unique_points( high_order_vertices_ ) ;
            }
        }
    }

    index_t GeoModelMeshOrder::nb_high_order_vertices_per_facet( index_t f ) const
    {
        test_and_initialize() ;
        ringmesh_assert( f < gmm_.facets.nb() ) ;
        return nb_high_order_points_per_facet_type_[gmm_.facets.nb_vertices( f ) - 3] ;
    }

    index_t GeoModelMeshOrder::nb_high_order_vertices_per_cell( index_t c ) const
    {
        test_and_initialize() ;
        ringmesh_assert( c < gmm_.cells.nb() ) ;
        return nb_high_order_points_per_cell_type_[gmm_.cells.type( c )] ;
    }

    /*******************************************************************************/

    GeoModelMesh::GeoModelMesh( GeoModel& gm )
        :
            geo_model_( gm ),
            mesh_( new Mesh( gm, 3, false ) ),
            mesh_builder_( new MeshBuilder( *mesh_ ) ),
            mode_( GeoModelMeshCells::NONE ),
            order_value_( 1 ),
            vertices( *this, gm, *mesh_, *mesh_builder_ ),
            edges( *this, *mesh_, *mesh_builder_ ),
            facets( *this, *mesh_, *mesh_builder_ ),
            cells( *this, *mesh_, *mesh_builder_ ),
            order( *this, *mesh_ )
    /*! @todo I am no expert but this initialization list looks like
     * a ticking bomb (like those in Mesh, btw I don not understand how these can work)
     * If these classes are derived one day, I don't know what will happen [JP]*/
    {

    }

    void GeoModelMesh::transfert_attributes() const
    {
        transfert_vertex_attributes() ;
        transfert_cell_attributes() ;
    }

    void GeoModelMesh::transfert_vertex_attributes() const
    {
        GEO::vector< std::string > att_v_names ;
        std::vector< std::string > att_v_double_names ;
        vertex_attribute_manager().list_attribute_names( att_v_names ) ;
        for( index_t att_v = 0; att_v < vertex_attribute_manager().nb(); att_v++ ) {

            if( !is_attribute_a_double( vertex_attribute_manager(),
                att_v_names[att_v] ) ) {
                continue ;
            }
            att_v_double_names.push_back( att_v_names[att_v] ) ;
            for( index_t reg = 0; reg < geo_model_.nb_regions(); reg++ ) {

                if( geo_model_.region( reg ).vertex_attribute_manager().is_defined(
                    att_v_names[att_v] ) ) {
                    Logger::warn( "Transfer attribute" ) << "The attribute "
                        << att_v_names[att_v] << " already exist on the region "
                        << reg << std::endl ;
                    continue ;
                }
                GEO::Attribute< double > cur_v_att ;
                cur_v_att.create_vector_attribute(
                    geo_model_.region( reg ).vertex_attribute_manager(),
                    att_v_names[att_v],
                    vertex_attribute_manager().find_attribute_store(
                        att_v_names[att_v] )->dimension() ) ;
            }
        }
        for( index_t att_v = 0; att_v < att_v_double_names.size(); att_v++ ) {

            GEO::Attribute< double > cur_att_on_geomodelmesh(
                vertex_attribute_manager(), att_v_double_names[att_v] ) ;
            index_t att_dim = cur_att_on_geomodelmesh.dimension() ;

            AttributeVector< double > att_on_regions( geo_model_.nb_regions() ) ;

            for( index_t reg = 0; reg < geo_model_.nb_regions(); reg++ ) {
                att_on_regions.bind_one_attribute( reg,
                    geo_model_.region( reg ).vertex_attribute_manager(),
                    att_v_double_names[att_v] ) ;
            }

            for( index_t v = 0; v < vertices.nb(); v++ ) {
                std::vector< GMEVertex > vertices_on_geomodel_region ;
                vertices.gme_type_vertices( Region::type_name_static(), v,
                    vertices_on_geomodel_region ) ;
                for( index_t gme_v = 0; gme_v < vertices_on_geomodel_region.size();
                    gme_v++ ) {
                    const GMEVertex& cur_vertex_on_geo_model =
                        vertices_on_geomodel_region[gme_v] ;
                    for( index_t att_e = 0; att_e < att_dim; att_e++ ) {
                        att_on_regions[cur_vertex_on_geo_model.gme_id.index][cur_vertex_on_geo_model.v_id
                            * att_dim + att_e] = cur_att_on_geomodelmesh[v * att_dim
                            + att_e] ;
                    }
                }

            }
        }
    }

    void GeoModelMesh::transfert_cell_attributes() const
    {

        GEO::vector< std::string > att_c_names ;
        cell_attribute_manager().list_attribute_names( att_c_names ) ;

        const ColocaterANN& ann = mesh_->colocater_ann( ColocaterANN::CELLS ) ;

        for( index_t att_c = 0; att_c < att_c_names.size(); att_c++ ) {
            if( !is_attribute_a_double( cell_attribute_manager(),
                att_c_names[att_c] ) ) {
                continue ;
            }
            GEO::Attribute< double > cur_att_on_geo_model_mesh(
                cell_attribute_manager(), att_c_names[att_c] ) ;
            index_t att_dim = cur_att_on_geo_model_mesh.dimension() ;

            for( index_t reg = 0; reg < geo_model_.nb_regions(); reg++ ) {
                if( geo_model_.region( reg ).cell_attribute_manager().is_defined(
                    att_c_names[att_c] ) ) {
                    Logger::warn( "Transfer attribute" ) << "The attribute "
                        << att_c_names[att_c] << " already exist on the region "
                        << reg << std::endl ;
                    continue ;
                }
                GEO::Attribute< double > cur_att_on_geo_model_mesh_entity ;
                cur_att_on_geo_model_mesh_entity.create_vector_attribute(
                    geo_model_.region( reg ).cell_attribute_manager(),
                    att_c_names[att_c], att_dim ) ;
                for( index_t c = 0; c < geo_model_.region( reg ).nb_mesh_elements();
                    c++ ) {
                    vec3 center = geo_model_.region( reg ).mesh_element_barycenter(
                        c ) ;
                    std::vector< index_t > c_in_geom_model_mesh ;
                    ann.get_neighbors( center, c_in_geom_model_mesh,
                        geo_model_.epsilon() ) ;
                    ringmesh_assert( c_in_geom_model_mesh.size() == 1 ) ;
                    for( index_t att_e = 0; att_e < att_dim; att_e++ ) {
                        cur_att_on_geo_model_mesh_entity[c * att_dim + att_e] =
                            cur_att_on_geo_model_mesh[c_in_geom_model_mesh[0]
                                * att_dim + att_e] ;
                    }
                }
            }
        }
    }

    GeoModelMesh::~GeoModelMesh()
    {
        facets.unbind_attribute() ;
        cells.unbind_attribute() ;
        delete mesh_builder_ ;
        delete mesh_ ;
    }

    void GeoModelMesh::remove_colocated_vertices()
    {
        vertices.remove_colocated() ;
    }

    void GeoModelMesh::erase_vertices( std::vector< index_t >& to_delete )
    {
        vertices.erase_vertices( to_delete ) ;
    }

//    void GeoModelMesh::erase_invalid_vertices()
//    {
//        vertices.erase_invalid_vertices() ;
//    }

}
