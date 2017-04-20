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

#pragma once

#include <ringmesh/basic/common.h>

#include <geogram/basic/command_line.h>

#include <geogram/mesh/mesh_preprocessing.h>

#include <geogram/voronoi/CVT.h>

#include <ringmesh/mesh/geogram_mesh.h>
#include <ringmesh/mesh/mesh_builder.h>

namespace RINGMesh {
    class GeoModel;
}

namespace RINGMesh {

#define COMMON_GEOGRAM_MESH_BUILDER_IMPLEMENTATION( Class )                                     \
    public:                                                                                     \
        Class ## Builder()                                                                      \
            : mesh_( nullptr )                                                                  \
        {                                                                                       \
        }                                                                                       \
        virtual ~Class ## Builder() = default ;                                                 \
        virtual void copy( const MeshBase& rhs, bool copy_attributes ) override                 \
        {                                                                                       \
            const Class& geogrammesh = dynamic_cast< const Class& >( rhs );                     \
            mesh_->mesh_->copy( *geogrammesh.mesh_, copy_attributes,                            \
                GEO::MESH_ALL_ELEMENTS );                                                       \
            clear_vertex_linked_objects();                                                      \
        }                                                                                       \
        virtual void load_mesh( const std::string& filename ) override                          \
        {                                                                                       \
            GEO::MeshIOFlags ioflags;                                                           \
            ioflags.set_attribute( GEO::MESH_ALL_ATTRIBUTES );                                  \
            GEO::mesh_load( filename, *mesh_->mesh_, ioflags );                                 \
        }                                                                                       \
        virtual void clear( bool keep_attributes, bool keep_memory ) override                   \
        {                                                                                       \
            mesh_->mesh_->clear( keep_attributes, keep_memory );                                \
            clear_vertex_linked_objects();                                                      \
        }                                                                                       \
        virtual void mesh_repair( GEO::MeshRepairMode mode, double colocate_epsilon ) override  \
        {                                                                                       \
            GEO::mesh_repair( *mesh_->mesh_, mode, colocate_epsilon );                          \
            clear_vertex_linked_objects();                                                      \
        }                                                                                       \
        virtual void set_vertex( index_t v_id, const vec3& vertex ) override                    \
        {                                                                                       \
            mesh_->mesh_->vertices.point( v_id ) = vertex;                                      \
            clear_vertex_linked_objects();                                                      \
        }                                                                                       \
        virtual index_t create_vertex() override                                                \
        {                                                                                       \
            clear_vertex_linked_objects();                                                      \
            return mesh_->mesh_->vertices.create_vertex();                                      \
        }                                                                                       \
        virtual index_t create_vertices( index_t nb ) override                                  \
        {                                                                                       \
            clear_vertex_linked_objects();                                                      \
            return mesh_->mesh_->vertices.create_vertices( nb );                                \
        }                                                                                       \
        virtual void assign_vertices(                                                           \
            const std::vector< double >& points_xyz_coordinates ) override                      \
        {                                                                                       \
            GEO::vector< double > points_xyz_coordinates_cp =                                   \
                copy_std_vector_to_geo_vector( points_xyz_coordinates );                        \
            mesh_->mesh_->vertices.assign_points( points_xyz_coordinates_cp, 3,                 \
                false );                                                                        \
            clear_vertex_linked_objects();                                                      \
        }                                                                                       \
        virtual void delete_vertices( const std::vector< bool >& to_delete ) override           \
        {                                                                                       \
            GEO::vector< index_t > vertices_to_delete =                                         \
                copy_std_vector_to_geo_vector< bool, index_t >( to_delete );                    \
            mesh_->mesh_->vertices.delete_elements( vertices_to_delete, false );                \
            clear_vertex_linked_objects();                                                      \
        }                                                                                       \
        virtual void clear_vertices( bool keep_attributes, bool keep_memory ) override          \
        {                                                                                       \
            mesh_->mesh_->vertices.clear( keep_attributes, keep_memory );                       \
            clear_vertex_linked_objects();                                                      \
        }                                                                                       \
        virtual void permute_vertices( const std::vector< index_t >& permutation ) override     \
        {                                                                                       \
            GEO::vector< index_t > geo_vector_permutation =                                     \
                copy_std_vector_to_geo_vector( permutation );                                   \
            mesh_->mesh_->vertices.permute_elements( geo_vector_permutation );                  \
            clear_vertex_linked_objects();                                                      \
        }                                                                                       \
        void set_geogram_mesh( Class& mesh )                                                    \
        {                                                                                       \
            mesh_ = &dynamic_cast< Class& >( mesh );                                            \
        }                                                                                       \
    protected:                                                                                  \
        void delete_vertex_nn_search()                                                          \
        {                                                                                       \
            mesh_->vertices_nn_search_.reset();                                                 \
        }                                                                                       \
    private:                                                                                    \
        Class* mesh_

    class RINGMESH_API GeogramMesh0DBuilder: public Mesh0DBuilder {
    COMMON_GEOGRAM_MESH_BUILDER_IMPLEMENTATION( GeogramMesh0D );
    public:
        virtual void set_mesh( Mesh0D& mesh ) override
        {
            set_geogram_mesh( dynamic_cast< GeogramMesh0D& >( mesh ) );
        }
        virtual void clear_vertex_linked_objects() override
        {
            delete_vertex_nn_search();
        }
    };

    class RINGMESH_API GeogramMesh1DBuilder: public Mesh1DBuilder {
    COMMON_GEOGRAM_MESH_BUILDER_IMPLEMENTATION( GeogramMesh1D );
    public:

        virtual void set_mesh( Mesh1D& mesh ) override
        {
            set_geogram_mesh( dynamic_cast< GeogramMesh1D& >( mesh ) );
        }
        /*!
         * @brief Create a new edge.
         * @param[in] v1_id index of the starting vertex.
         * @param[in] v2_id index of the ending vertex.
         */
        virtual void create_edge( index_t v1_id, index_t v2_id ) override
        {
            mesh_->mesh_->edges.create_edge( v1_id, v2_id );
            clear_edge_linked_objects();
        }
        /*!
         * \brief Creates a contiguous chunk of edges
         * \param[in] nb_edges number of edges to create
         * \return the index of the first edge
         */
        virtual index_t create_edges( index_t nb_edges ) override
        {
            return mesh_->mesh_->edges.create_edges( nb_edges );
        }
        /*!
         * @brief Sets a vertex of a facet by local vertex index.
         * @param[in] edge_id index of the edge, in 0..nb()-1.
         * @param[in] local_vertex_id index of the vertex in the facet. Local index between 0 and @function nb_vertices(cell_id) - 1.
         * @param[in] vertex_id specifies the vertex \param local_vertex_id of facet \param of the facet facet_id. Index between 0 and @function nb() - 1.
         */
        virtual void set_edge_vertex(
            index_t edge_id,
            index_t local_vertex_id,
            index_t vertex_id ) override
        {
            mesh_->mesh_->edges.set_vertex( edge_id, local_vertex_id, vertex_id );
            clear_edge_linked_objects();
        }
        /*!
         * @brief Deletes a set of edges.
         * @param[in] to_delete     a vector of size @function nb().
         * If to_delete[e] is true, then entity e will be destroyed, else it will be kept.
         * @param[in] remove_isolated_vertices if true, then the vertices that are
         * no longer incident to any entity are deleted.
         */
        virtual void delete_edges(
            const std::vector< bool >& to_delete,
            bool remove_isolated_vertices ) override
        {
            GEO::vector< index_t > edges_to_delete = copy_std_vector_to_geo_vector<
                bool, index_t >( to_delete );
            mesh_->mesh_->edges.delete_elements( edges_to_delete, false );
            if( remove_isolated_vertices ) {
                this->remove_isolated_vertices();
            }
            clear_edge_linked_objects();
        }
        /*!
         * @brief Removes all the edges and attributes.
         * @param[in] keep_attributes if true, then all the existing attribute
         * names / bindings are kept (but they are cleared). If false, they are destroyed.
         * @param[in] keep_memory if true, then memory is kept and can be reused
         * by subsequent mesh entity creations.
         */
        virtual void clear_edges( bool keep_attributes, bool keep_memory ) override
        {
            mesh_->mesh_->edges.clear( keep_attributes, keep_memory );
            clear_edge_linked_objects();
        }

        /*!
         * @brief Remove vertices not connected to any mesh element
         */
        virtual void remove_isolated_vertices() override
        {
            std::vector< bool > to_delete( mesh_->nb_vertices(), true );

            for( index_t e = 0; e < mesh_->nb_edges(); e++ ) {
                for( index_t v = 0; v < 2; v++ ) {
                    index_t vertex_id = mesh_->edge_vertex( e, v );
                    to_delete[vertex_id] = false;
                }
            }
            delete_vertices( to_delete );

        }

        virtual void permute_edges( const std::vector< index_t >& permutation ) override
        {
            GEO::vector< index_t > geo_vector_permutation =
                copy_std_vector_to_geo_vector( permutation );
            mesh_->mesh_->edges.permute_elements( geo_vector_permutation );
        }

        virtual void clear_vertex_linked_objects() override
        {
            delete_vertex_nn_search();
            clear_edge_linked_objects();
        }
        virtual void clear_edge_linked_objects() override
        {
            delete_edge_nn_search();
        }

    protected:
        /*!
         * @brief Deletes the NNSearch on edges
         */
        void delete_edge_nn_search()
        {
            mesh_->edges_nn_search_.reset();
        }
    };

    class RINGMESH_API GeogramMesh2DBuilder: public Mesh2DBuilder {
    COMMON_GEOGRAM_MESH_BUILDER_IMPLEMENTATION( GeogramMesh2D );
    public:

        virtual void set_mesh( Mesh2D& mesh ) override
        {
            set_geogram_mesh( dynamic_cast< GeogramMesh2D& >( mesh ) );
        }
        /*!
         * \brief Removes the connected components that have an area
         *  smaller than a given threshold.
         * \param[in] min_area the connected components with an
         *  area smaller than this threshold are removed
         * \param[in] min_facets the connected components with
         *  less than \param min_facets facets are removed
         */
        virtual void remove_small_connected_components(
            double min_area,
            index_t min_facets ) override
        {
            GEO::remove_small_connected_components( *mesh_->mesh_, min_area,
                min_facets );
        }

        virtual void triangulate( const Mesh2D& surface_in ) override
        {
            Logger::instance()->set_minimal( true );
            const GeogramMesh2D& geogram_surf_in =
                dynamic_cast< const GeogramMesh2D& >( surface_in );
            GEO::CentroidalVoronoiTesselation CVT( geogram_surf_in.mesh_.get(), 3,
                GEO::CmdLine::get_arg( "algo:delaunay" ) );
            CVT.set_points( mesh_->nb_vertices(),
                mesh_->mesh_->vertices.point_ptr( 0 ) );
            CVT.compute_surface( mesh_->mesh_.get(), false );
            Logger::instance()->set_minimal( false );
        }
        /*!
         * brief create facet polygons
         * @param[in] facets is the vector of vertex index for each facet
         * @param[in] facet_ptr is the vector addressing the first facet vertex for each facet.
         */
        virtual void create_facet_polygons(
            const std::vector< index_t >& facets,
            const std::vector< index_t >& facet_ptr ) override
        {
            for( index_t f = 0; f + 1 < facet_ptr.size(); f++ ) {
                index_t start = facet_ptr[f];
                index_t end = facet_ptr[f + 1];
                GEO::vector< index_t > facet_vertices =
                    copy_std_vector_to_geo_vector( facets, start, end );
                mesh_->mesh_->facets.create_polygon( facet_vertices );
            }
            clear_facet_linked_objects();
        }
        /*!
         * \brief Creates a polygonal facet
         * \param[in] vertices a const reference to a vector that
         *  contains the vertices
         * \return the index of the created facet
         */
        virtual index_t create_facet_polygon(
            const std::vector< index_t >& vertices ) override
        {
            GEO::vector< index_t > facet_vertices = copy_std_vector_to_geo_vector(
                vertices );
            index_t index = mesh_->mesh_->facets.create_polygon( facet_vertices );
            clear_facet_linked_objects();
            return index;
        }

        /*!
         * \brief Creates a contiguous chunk of triangles
         * \param[in] nb_triangles number of triangles to create
         * \return the index of the first triangle
         */
        virtual index_t create_facet_triangles( index_t nb_triangles ) override
        {
            return mesh_->mesh_->facets.create_triangles( nb_triangles );

        }
        /*!
         * \brief Creates a contiguous chunk of quads
         * \param[in] nb_quads number of quads to create
         * \return the index of the first quad
         */
        virtual index_t create_facet_quads( index_t nb_quads ) override
        {
            return mesh_->mesh_->facets.create_quads( nb_quads );
        }
        /*!
         * @brief Sets a vertex of a facet by local vertex index.
         * @param[in] facet_id index of the facet, in 0.. @function nb() - 1.
         * @param[in] local_vertex_id index of the vertex in the facet. Local index between 0 and @function nb_vertices(cell_id) - 1.
         * @param[in] vertex_id specifies the vertex \param local_vertex_id of the facet \param facet_id. Index between 0 and @function nb() - 1.
         */
        virtual void set_facet_vertex(
            index_t facet_id,
            index_t local_vertex_id,
            index_t vertex_id ) override
        {
            mesh_->mesh_->facets.set_vertex( facet_id, local_vertex_id, vertex_id );
            clear_facet_linked_objects();
        }
        /*!
         * @brief Sets an adjacent facet by both its facet \param facet_id and its local edge index \param edge_id.
         * @param[in] facet_id the facet index
         * @param[in] edge_id the local index of an edge in facet \p facet_id
         * @param[in] specifies the facet adjacent to \param facet_id along edge \param edge_id or GEO::NO_FACET if the parameter \param edge_id is on the border.
         */
        virtual void set_facet_adjacent(
            index_t facet_id,
            index_t edge_id,
            index_t specifies ) override
        {
            mesh_->mesh_->facets.set_adjacent( facet_id, edge_id, specifies );
        }
        /*!
         * \brief Copies a triangle mesh into this Mesh.
         * \details Facet adjacence are not computed.
         *   Facet and corner attributes are zeroed.
         * \param[in] triangles facet to vertex links
         * (using vector::swap).
         */
        virtual void assign_facet_triangle_mesh(
            const std::vector< index_t >& triangles ) override
        {
            GEO::vector< index_t > geo_triangles = copy_std_vector_to_geo_vector(
                triangles );
            mesh_->mesh_->facets.assign_triangle_mesh( geo_triangles, false );
            clear_facet_linked_objects();
        }
        /*!
         * @brief Removes all the facets and attributes.
         * @param[in] keep_attributes if true, then all the existing attribute
         * names / bindings are kept (but they are cleared). If false, they are destroyed.
         * @param[in] keep_memory if true, then memory is kept and can be reused
         * by subsequent mesh entity creations.
         */
        virtual void clear_facets( bool keep_attributes, bool keep_memory ) override
        {
            mesh_->mesh_->facets.clear( keep_attributes, keep_memory );
        }
        /*!
         * @brief Retrieve the adjacencies of facets
         */
        virtual void connect_facets() override
        {
            mesh_->mesh_->facets.connect();
        }
        virtual void permute_facets( const std::vector< index_t >& permutation ) override
        {
            GEO::vector< index_t > geo_vector_permutation =
                copy_std_vector_to_geo_vector( permutation );
            mesh_->mesh_->facets.permute_elements( geo_vector_permutation );
        }
        /*!
         * @brief Deletes a set of facets.
         * @param[in] to_delete     a vector of size @function nb().
         * If to_delete[e] is true, then entity e will be destroyed, else it will be kept.
         * @param[in] remove_isolated_vertices if true, then the vertices that are
         * no longer incident to any entity are deleted.
         */
        virtual void delete_facets(
            const std::vector< bool >& to_delete,
            bool remove_isolated_vertices ) override
        {
            GEO::vector< index_t > facets_to_delete = copy_std_vector_to_geo_vector<
                bool, index_t >( to_delete );
            mesh_->mesh_->facets.delete_elements( facets_to_delete, false );
            if( remove_isolated_vertices ) {
                this->remove_isolated_vertices();
            }
            clear_facet_linked_objects();
        }
        /*!
         * @brief Remove vertices not connected to any mesh element
         */
        virtual void remove_isolated_vertices() override
        {
            std::vector< bool > to_delete( mesh_->nb_vertices(), true );

            for( index_t f = 0; f < mesh_->nb_facets(); f++ ) {
                for( index_t v = 0; v < mesh_->nb_facet_vertices( f ); v++ ) {
                    index_t vertex_id = mesh_->facet_vertex( f, v );
                    to_delete[vertex_id] = false;
                }
            }

            delete_vertices( to_delete );
        }
        virtual void clear_vertex_linked_objects() override
        {
            delete_vertex_nn_search();
            clear_facet_linked_objects();
        }
        virtual void clear_facet_linked_objects() override
        {
            delete_facet_aabb();
            delete_facet_nn_search();
        }
    protected:
        /*!
         * @brief Deletes the NNSearch on facets
         */
        void delete_facet_nn_search()
        {
            mesh_->nn_search_.reset();
        }
        /*!
         * @brief Deletes the AABB on facets
         */
        void delete_facet_aabb()
        {
            mesh_->facets_aabb_.reset();
        }
    };

    class RINGMESH_API GeogramMesh3DBuilder: public Mesh3DBuilder {
    COMMON_GEOGRAM_MESH_BUILDER_IMPLEMENTATION( GeogramMesh3D );
    public:

        virtual void set_mesh( Mesh3D& mesh ) override
        {
            set_geogram_mesh( dynamic_cast< GeogramMesh3D& >( mesh ) );
        }
        /*!
         * @brief Creates a contiguous chunk of cells of the same type.
         * @param[in] nb_cells  number of cells to create
         * @param[in] type   type of the cells to create, one of GEO::MESH_TET, GEO::MESH_HEX,
         * GEO::MESH_PRISM, GEO::MESH_PYRAMID, GEO::MESH_CONNECTOR.
         * @return the first created cell.
         */
        virtual index_t create_cells( index_t nb_cells, GEO::MeshCellType type ) override
        {
            return mesh_->mesh_->cells.create_cells( nb_cells, type );
        }

        /*!
         * \brief Copies a tets mesh into this Mesh.
         * \details Cells adjacence are not computed.
         *   cell and corner attributes are zeroed.
         * \param[in] tets cells to vertex links
         */
        virtual void assign_cell_tet_mesh( const std::vector< index_t >& tets ) override
        {
            GEO::vector< index_t > copy = copy_std_vector_to_geo_vector( tets );
            mesh_->mesh_->cells.assign_tet_mesh( copy, false );
            clear_cell_linked_objects();
        }

        /*!
         * @brief Sets a vertex of a cell by local vertex index.
         * @param[in] cell_id index of the cell, in 0.. @function nb() - 1.
         * @param[in] local_vertex_id index of the vertex in the cell. Local index between 0 and @function nb_vertices(cell_id) - 1.
         * @param[in] vertex_id specifies the global index of the vertex \param local_vertex_id in the cell \param cell_id. Index between 0 and @function nb() - 1.
         */
        virtual void set_cell_vertex(
            index_t cell_id,
            index_t local_vertex_id,
            index_t vertex_id ) override
        {
            mesh_->mesh_->cells.set_vertex( cell_id, local_vertex_id, vertex_id );
            clear_cell_linked_objects();
        }
        /*!
         * \brief Sets the vertex that a corner is incident to
         * \param[in] corner_index the corner, in 0.. @function nb() - 1
         * \param[in] vertex_index specifies the vertex that corner \param corner_index is incident to
         */
        virtual void set_cell_corner_vertex_index(
            index_t corner_index,
            index_t vertex_index ) override
        {
            mesh_->mesh_->cell_corners.set_vertex( corner_index, vertex_index );
            clear_cell_linked_objects();
        }
        /*!
         * \brief Sets the cell adjacent
         * \param[in] cell_index index of the cell
         * \param[in] facet_index local index of the cell facet
         * \param[in] cell_adjacent adjacent value to set
         */
        virtual void set_cell_adjacent(
            index_t cell_index,
            index_t facet_index,
            index_t cell_adjacent ) override
        {
            mesh_->mesh_->cells.set_adjacent( cell_index, facet_index,
                cell_adjacent );
        }
        /*!
         * @brief Retrieve the adjacencies
         */
        virtual void connect_cells() override
        {
            mesh_->mesh_->cells.connect();
        }

        /*!
         * @brief Removes all the cells and attributes.
         * @param[in] keep_attributes if true, then all the existing attribute
         * names / bindings are kept (but they are cleared). If false, they are destroyed.
         * @param[in] keep_memory if true, then memory is kept and can be reused
         * by subsequent mesh entity creations.
         */
        virtual void clear_cells( bool keep_attributes, bool keep_memory ) override
        {
            mesh_->mesh_->cells.clear( keep_attributes, keep_memory );
        }
        /*!
         * @brief Applies a permutation to the entities and their attributes.
         * On exit, permutation is modified (used for internal bookkeeping).
         * Applying a permutation permutation is equivalent to:
         * <code>
         *  for(i=0; i<permutation.size(); i++) {
         *      data2[i] = data[permutation[i]]
         *       }
         *  data = data2 ;
         *  </code>
         */
        virtual void permute_cells( const std::vector< index_t >& permutation ) override
        {
            GEO::vector< index_t > geo_vector_permutation =
                copy_std_vector_to_geo_vector( permutation );
            mesh_->mesh_->cells.permute_elements( geo_vector_permutation );
        }
        /*!
         * @brief Deletes a set of cells.
         * @param[in] to_delete     a vector of size @function nb().
         * If to_delete[e] is true, then entity e will be destroyed, else it will be kept.
         * @param[in] remove_isolated_vertices if true, then the vertices that are
         * no longer incident to any entity are deleted.
         */
        virtual void delete_cells(
            const std::vector< bool >& to_delete,
            bool remove_isolated_vertices ) override
        {
            GEO::vector< index_t > geo_to_delete = copy_std_vector_to_geo_vector<
                bool, index_t >( to_delete );
            mesh_->mesh_->cells.delete_elements( geo_to_delete, false );
            if( remove_isolated_vertices ) {
                this->remove_isolated_vertices();
            }
            clear_cell_linked_objects();
        }
        /*!
         * @brief Remove vertices not connected to any mesh element
         */
        virtual void remove_isolated_vertices() override
        {
            std::vector< bool > to_delete( mesh_->nb_vertices(), true );

            for( index_t c = 0; c < mesh_->nb_cells(); c++ ) {
                for( index_t v = 0; v < mesh_->nb_cell_vertices( c ); v++ ) {
                    index_t vertex_id = mesh_->cell_vertex( c, v );
                    to_delete[vertex_id] = false;
                }
            }

            delete_vertices( to_delete );
        }

        virtual void clear_vertex_linked_objects() override
        {
            delete_vertex_nn_search();
            clear_cell_linked_objects();
        }

        virtual void clear_cell_linked_objects() override
        {
            delete_cell_aabb();
            delete_cell_nn_search();
        }

    protected:
        /*!
         * @brief Deletes the NNSearch on cells
         */
        void delete_cell_nn_search()
        {
            mesh_->cell_nn_search_.reset();
            mesh_->cell_facets_nn_search_.reset();
        }
        /*!
         * @brief Deletes the AABB on cells
         */
        void delete_cell_aabb()
        {
            mesh_->cell_aabb_.reset();
        }
    };

}
