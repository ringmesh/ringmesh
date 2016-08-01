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

#ifndef __RINGMESH_MESH_BUILDER__
#define __RINGMESH_MESH_BUILDER__

#include <ringmesh/basic/common.h>

#include <geogram/basic/command_line.h>
#include <geogram/mesh/mesh.h>
#include <geogram/mesh/mesh_io.h>
#include <geogram/mesh/mesh_geometry.h>
#include <geogram/mesh/mesh_preprocessing.h>
#include <geogram/mesh/mesh_AABB.h>
#include <geogram/voronoi/CVT.h>

#include <ringmesh/basic/geometry.h>
#include <ringmesh/geogram_extension/geogram_extension.h>

namespace RINGMesh {
    class GeoModel ;
}

namespace RINGMesh {

    class RINGMESH_API MeshBuilder {
    ringmesh_disable_copy( MeshBuilder ) ;

    public:
        MeshBuilder( Mesh& mesh )
            : mesh_( mesh )
        {
        }
        ~MeshBuilder() {} ;

        void load_mesh(
            const std::string& filename,
            const GEO::MeshIOFlags& ioflags )
        {
            GEO::mesh_load( filename, *mesh_.mesh_, ioflags ) ;
        }
        /*!
         * @brief Removes all the entities and attributes of this mesh.
         * @param[in] keep_attributes if true, then all the existing attribute
         * names / bindings are kept (but they are cleared). If false, they are destroyed.
         * @param[in] keep_memory if true, then memory is kept and can be reused
         * by subsequent mesh entity creations.
         */
        void clear( bool keep_attributes, bool keep_memory )
        {
            mesh_.mesh_->clear( keep_attributes, keep_memory ) ;
            delete_vertex_colocater() ;
            delete_edge_colocater() ;
            delete_facet_aabb() ;
            delete_facet_colocater() ;
            delete_cell_aabb() ;
            delete_cell_colocater() ;
        }
        /**
         * \brief Fixes some defaults in a mesh.
         * \param[in] mode a combination of #MeshRepairMode flags.
         *  Combine them with the 'bitwise or' (|) operator.
         * \param[in] colocate_epsilon tolerance used to colocate vertices
         *  (if #MESH_REPAIR_COLOCATE is set in mode).
         */
        void mesh_repair( GEO::MeshRepairMode mode, double colocate_epsilon )
        {
            GEO::mesh_repair( *mesh_.mesh_, mode, colocate_epsilon ) ;

        }
        /**
         * \brief Removes the connected components that have an area
         *  smaller than a given threshold.
         * \param[in] min_component_area the connected components with an
         *  area smaller than this threshold are removed
         * \param[in] min_component_facets the connected components with
         *  less than min_component_facets facets are removed
         */
        void remove_small_connected_components( double min_area, index_t min_facets )
        {
            GEO::remove_small_connected_components( *mesh_.mesh_, min_area,
                min_facets ) ;
        }

        void triangulate(
            const Mesh& surface_in )
        {
            Logger::instance()->set_minimal( true ) ;
            GEO::CentroidalVoronoiTesselation CVT( surface_in.mesh_, 3,
                GEO::CmdLine::get_arg( "algo:delaunay" ) ) ;
            CVT.set_points( mesh_.nb_vertices(), mesh_.mesh_->vertices.point_ptr( 0 ) ) ;
            CVT.compute_surface( mesh_.mesh_, false ) ;
            Logger::instance()->set_minimal( false ) ;
        }


        /*!
         * \name Vertex methods
         * @{
         */
        /*!
         * @brief Gets a point.
         * @param[in] v_id the vertex, in 0.. @function nb_vetices()-1.
         * @return reference to the point that corresponds to the vertex.
         */
        vec3& vertex( index_t v_id )
        {
            return mesh_.mesh_->vertices.point( v_id ) ;
        }
        /*!
         * \brief Gets a point
         * \param[in] v_id the vertex, in 0..nb()-1
         * \return a pointer to the coordinates of the point
         *  that correspond to the vertex
         * \pre !single_precision()
         */
        double* point_ptr( index_t v_id )
        {
            return mesh_.mesh_->vertices.point_ptr( v_id ) ;
        }
        /*!
         * @brief Creates a new vertex.
         * @return the index of the created vertex
         */
        index_t create_vertex()
        {
            return mesh_.mesh_->vertices.create_vertex() ;
        }
        /*!
         * @brief Creates a new vertex.
         * @param[in] coords a pointer to dimension() coordinate.
         * @return the index of the created vertex
         */
        index_t create_vertex( const double* coords )
        {
            return mesh_.mesh_->vertices.create_vertex( coords ) ;
        }
        /*!
         * @brief Creates a contiguous chunk of vertices.
         * @param[in] nb number of sub-entities to create.
         * @return the index of the first created vertex
         */
        index_t create_vertices( index_t nb )
        {
            return mesh_.mesh_->vertices.create_vertices( nb ) ;
        }
        /*!
         * @brief Deletes a set of vertices.
         * @param[in] to_delete     a vector of size nb(). If to_delete[e] is different from 0,
         * then entity e will be destroyed, else it will be kept. On exit, to_delete is modified
         * (it is used for internal bookkeeping).
         * @param[in] remove_isolated_vertices if true, then the vertices that are no longer incident to any entity are deleted.
         */
        void delete_vertices(
            GEO::vector< index_t >& to_delete,
            bool remove_isolated_vertices )
        {
            mesh_.mesh_->vertices.delete_elements( to_delete,
                remove_isolated_vertices ) ;
            delete_vertex_colocater() ;
        }
        /*!
         * @brief Removes all the vertices and attributes.
         * @param[in] keep_attributes if true, then all the existing attribute
         * names / bindings are kept (but they are cleared). If false, they are destroyed.
         * @param[in] keep_memory if true, then memory is kept and can be reused
         * by subsequent mesh entity creations.
         */
        void clear_vertices( bool keep_attributes, bool keep_memory )
        {
            mesh_.mesh_->vertices.clear( keep_attributes, keep_memory ) ;
            delete_vertex_colocater() ;
        }
        /*!
         * @brief Deletes the ColocaterANN on vertices
         */
        void delete_vertex_colocater()
        {
            if( mesh_.ann_[ColocaterANN::VERTICES] ) {
                delete mesh_.ann_[ColocaterANN::VERTICES] ;
                mesh_.ann_[ColocaterANN::VERTICES] = nil ;
            }
        }

        /*!@}
         * \section Edge methods
         * @{
         */
        /*!
         * @brief Create a new edge.
         * @param[in] v1_id index of the starting vertex.
         * @param[in] v2_id index of the ending vertex.
         */
        void create_edge( index_t v1_id, index_t v2_id )
        {
            mesh_.mesh_->edges.create_edge( v1_id, v2_id ) ;
        }
        /*!
         * \brief Creates a contiguous chunk of edges
         * \param[in] nb_edges number of edges to create
         * \return the index of the first edge
         */
        index_t create_edges( index_t nb_edges )
        {
           return mesh_.mesh_->edges.create_edges( nb_edges ) ;
        }
        /*!
         * @brief Sets a vertex of a facet by local vertex index.
         * @param[in] edge_id index of the edge, in 0..nb()-1.
         * @param[in] local_vertex_id index of the vertex in the facet. Local index between 0 and nb_vertices(cell_id)-1.
         * @param[in] vertex_id specifies the vertex \param local_vertex_id of facet \param of the facet facet_id. Index between 0 and nb()-1.
         */
        void set_edge_vertex(
            index_t edge_id,
            index_t local_vertex_id,
            index_t vertex_id )
        {
            mesh_.mesh_->edges.set_vertex( edge_id, local_vertex_id, vertex_id ) ;
        }
        /*!
         * @brief Deletes a set of edges.
         * @param[in] to_delete a vector of size nb(). If to_delete[e] is different from 0,
         * then entity e will be destroyed, else it will be kept. On exit, to_delete is modified
         * (it is used for internal bookkeeping).
         * @param[in] remove_isolated_vertices if true, then the vertices that are no longer incident to any entity are deleted.
         */
        void delete_edges(
            GEO::vector< index_t > to_delete,
            bool remove_isolated_vertices )
        {
            mesh_.mesh_->edges.delete_elements( to_delete,
                remove_isolated_vertices ) ;
            delete_edge_colocater() ;
        }
        /*!
         * @brief Removes all the edges and attributes.
         * @param[in] keep_attributes if true, then all the existing attribute
         * names / bindings are kept (but they are cleared). If false, they are destroyed.
         * @param[in] keep_memory if true, then memory is kept and can be reused
         * by subsequent mesh entity creations.
         */
        void clear_edges( bool keep_attributes, bool keep_memory ) {
            mesh_.mesh_->edges.clear( keep_attributes, keep_memory ) ;
            delete_edge_colocater() ;
        }
        /*!
         * @brief Deletes the ColocaterANN on edges
         */
        void delete_edge_colocater()
        {
            if( mesh_.ann_[ColocaterANN::EDGES] ) {
                delete mesh_.ann_[ColocaterANN::EDGES] ;
                mesh_.ann_[ColocaterANN::EDGES] = nil ;
            }
        }

        /*!@}
         * \section Facet methods
         * @{
         */
        /*!
         * brief create facet polygons
         * @param[in] facets is the vector of vertex index for each facet
         * @param[in] facet_ptr is the vector addressing the first facet vertex for each facet.
         */
        void create_facet_polygons(
            const std::vector< index_t >& facets,
            const std::vector< index_t >& facet_ptr )
        {
            for( index_t f = 0; f + 1 < facet_ptr.size(); f++ ) {
                index_t start = facet_ptr[f] ;
                index_t end = facet_ptr[f + 1] ;
                GEO::vector< index_t > facet_vertices ;
                copy_std_vector_to_geo_vector( facets, start, end, facet_vertices ) ;

                mesh_.mesh_->facets.create_polygon( facet_vertices ) ;
            }
        }
        /*!
         * \brief Creates a polygonal facet
         * \param[in] vertices a const reference to a vector that
         *  contains the vertices
         * \return the index of the created facet
         */
        index_t create_facet_polygon( const GEO::vector< index_t >& vertices ) {
            return mesh_.mesh_->facets.create_polygon( vertices ) ;
        }

        /*!
         * \brief Creates a contiguous chunk of triangles
         * \param[in] nb_triangles number of triangles to create
         * \return the index of the first triangle
         */
       index_t create_facet_triangles( index_t nb_triangles )
       {
           return mesh_.mesh_->facets.create_triangles( nb_triangles ) ;

       }
       /*!
        * \brief Creates a contiguous chunk of quads
        * \param[in] nb_quads number of quads to create
        * \return the index of the first quad
        */
       index_t create_facet_quads( index_t nb_quads )
       {
           return mesh_.mesh_->facets.create_quads( nb_quads ) ;
       }
        /*!
         * @brief Sets a vertex of a facet by local vertex index.
         * @param[in] facet_id index of the facet, in 0..nb()-1.
         * @param[in] local_vertex_id index of the vertex in the facet. Local index between 0 and nb_vertices(cell_id)-1.
         * @param[in] vertex_id specifies the vertex \param local_vertex_id of facet \param of the facet facet_id. Index between 0 and nb()-1.
         */
        void set_facet_vertex(
            index_t facet_id,
            index_t local_vertex_id,
            index_t vertex_id )
        {
            mesh_.mesh_->facets.set_vertex( facet_id, local_vertex_id, vertex_id ) ;
        }
        /*!
         * @brief Sets an adjacent facet by facet and local edge index.
         * @param[in] facet_id the facet index
         * @param[in] edge_id the local index of an edge in facet \p facet_id
         * @param[in] specifies the facet incident to f along edge le or GEO::NO_FACET if \p edge_id is on the border.
         */
        void set_facet_adjacent(
            index_t facet_id,
            index_t edge_id,
            index_t specifies )
        {
            mesh_.mesh_->facets.set_adjacent( facet_id, edge_id, specifies ) ;
        }
        /*
         * \brief Copies a triangle mesh into this Mesh.
         * \details Facet adjacence are not computed.
         *   Facet and corner attributes are zeroed.
         * \param[in] triangles facet to vertex links
         * \param[in] steal_args if set, vertices and triangles
         * are 'stolen' from the arguments
         * (using vector::swap).
         */
        void assign_facet_triangle_mesh(
            const std::vector< index_t >& triangles,
            bool steal_args )
        {
            GEO::vector< index_t > copy ;
            copy_std_vector_to_geo_vector( triangles, copy ) ;
            mesh_.mesh_->facets.assign_triangle_mesh( copy, steal_args ) ;
        }
        /*!
         * @brief Removes all the facets and attributes.
         * @param[in] keep_attributes if true, then all the existing attribute
         * names / bindings are kept (but they are cleared). If false, they are destroyed.
         * @param[in] keep_memory if true, then memory is kept and can be reused
         * by subsequent mesh entity creations.
         */
        void clear_facets( bool keep_attributes, bool keep_memory ) {
            mesh_.mesh_->facets.clear( keep_attributes, keep_memory ) ;
        }
        void connect_facets() {
            mesh_.mesh_->facets.connect() ;
        }
        void permute_facets( GEO::vector<index_t>& permutation ) {
            mesh_.mesh_->facets.permute_elements( permutation ) ;
        }
        /*!
         * @brief Deletes a set of facets.
         * @param[in] to_delete     a vector of size nb(). If to_delete[f] is different from 0,
         * then facet f will be destroyed, else it will be kept. On exit, to_delete is modified
         * (it is used for internal bookkeeping).
         * @param[in] remove_isolated_vertices if true, then the vertices that are no longer incident to any entity are deleted.
         */
        void delete_facets(
            GEO::vector< index_t >& to_delete,
            bool remove_isolated_vertices )
        {
            mesh_.mesh_->facets.delete_elements( to_delete,
                remove_isolated_vertices ) ;
            delete_facet_aabb() ;
            delete_facet_colocater() ;
        }
        /*!
         * @brief Deletes the ColocaterANN on facets
         */
        void delete_facet_colocater()
        {
            if( mesh_.ann_[ColocaterANN::FACETS] ) {
                delete mesh_.ann_[ColocaterANN::FACETS] ;
                mesh_.ann_[ColocaterANN::FACETS] = nil ;
            }
        }
        /*!
         * @brief Deletes the AABB on facets
         */
        void delete_facet_aabb()
        {
            if( mesh_.facets_aabb_ ) {
                delete mesh_.facets_aabb_;
                mesh_.facets_aabb_ = nil ;
            }
        }

        /*!@}
         * \section Cells methods
         * @{
         */
        /*!
         * @brief Creates a contiguous chunk of cells of the same type.
         * @param[in] nb_cells  number of cells to create
         * @param[in] type   type of the cells to create, one of GEO::MESH_TET, GEO::MESH_HEX,
         * GEO::MESH_PRISM, GEO::MESH_PYRAMID, GEO::MESH_CONNECTOR.
         * @return the first created cell.
         */
        index_t create_cells( index_t nb_cells, GEO::MeshCellType type )
        {
            return mesh_.mesh_->cells.create_cells( nb_cells, type ) ;
        }
        /*
         * \brief Copies a tets mesh into this Mesh.
         * \details Cells adjacence are not computed.
         *   cell and corner attributes are zeroed.
         * \param[in] tets cells to vertex links
         * \param[in] steal_args if set, vertices and tets
         * are 'stolen' from the arguments
         * (using vector::swap).
         */
        void assign_cell_tet_mesh(
            const std::vector< index_t >& tets,
            bool steal_args )
        {
            GEO::vector< index_t > copy ;
            copy_std_vector_to_geo_vector( tets, copy ) ;
            mesh_.mesh_->cells.assign_tet_mesh( copy, steal_args ) ;
        }

        /*!
         * @brief Sets a vertex of a cell by local vertex index.
         * @param[in] cell_id index of the cell, in 0..nb()-1.
         * @param[in] local_vertex_id index of the vertex in the cell. Local index between 0 and nb_vertices(cell_id)-1.
         * @param[in] vertex_id specifies the vertex \param local_vertex_id of cell \param of the cell cell_id. Index between 0 and nb()-1.
         */
        void set_cell_vertex(
            index_t cell_id,
            index_t local_vertex_id,
            index_t vertex_id )
        {
            mesh_.mesh_->cells.set_vertex( cell_id, local_vertex_id, vertex_id ) ;
        }
        /*!
         * \brief Sets the vertex that a corner is incident to
         * \param[in] corner_index the corner, in 0..nb()-1
         * \param[in] vertex_index specifies the vertex that corner \p c is incident to
         */
        void set_cell_corner_vertex_index( index_t corner_index, index_t vertex_index ) {
            mesh_.mesh_->cell_corners.set_vertex( corner_index, vertex_index ) ;
        }
        /*!
         * \brief Sets the cell adjacent
         * \param[in] cell_index index of the cell
         * \param[in] facet_index local index of the cell facet
         * \param[in] cell_adjacent adjacent value to set
         */
        void set_cell_adjacent(
            index_t cell_index,
            index_t facet_index,
            index_t cell_adjacent )
        {
            mesh_.mesh_->cells.set_adjacent( cell_index, facet_index, cell_adjacent ) ;
        }
        /*!
         * @brief Retrieve the adjacencies
         */
        void connect_cells()
        {
            mesh_.mesh_->cells.connect() ;
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
        void cells_permute_elements( GEO::vector< index_t >& permutation )
        {
            mesh_.mesh_->cells.permute_elements( permutation ) ;
        }
        /*!
         * @brief Removes all the cells and attributes.
         * @param[in] keep_attributes if true, then all the existing attribute
         * names / bindings are kept (but they are cleared). If false, they are destroyed.
         * @param[in] keep_memory if true, then memory is kept and can be reused
         * by subsequent mesh entity creations.
         */
        void clear_cells( bool keep_attributes, bool keep_memory ) {
            mesh_.mesh_->cells.clear( keep_attributes, keep_memory ) ;
        }
        void permute_cells( GEO::vector<index_t>& permutation ) {
            mesh_.mesh_->cells.permute_elements( permutation ) ;
        }
        /*!
         * @brief Deletes a set of cells.
         * @param[in] to_delete     a vector of size nb(). If to_delete[c] is different from 0,
         * then cell c will be destroyed, else it will be kept. On exit, to_delete is modified
         * (it is used for internal bookkeeping).
         * @param[in] remove_isolated_vertices if true, then the vertices that are no longer incident to any entity are deleted.
         */
        void delete_cells(
            GEO::vector< index_t >& to_delete,
            bool remove_isolated_vertices )
        {
            mesh_.mesh_->cells.delete_elements( to_delete,
                remove_isolated_vertices ) ;
            delete_cell_aabb() ;
            delete_cell_colocater() ;
        }
        /*!
         * @brief Deletes the ColocaterANN on cells
         */
        void delete_cell_colocater()
        {
            if( mesh_.ann_[ColocaterANN::CELLS] ) {
                delete mesh_.ann_[ColocaterANN::CELLS] ;
                mesh_.ann_[ColocaterANN::CELLS] = nil ;
            }
            if( mesh_.ann_[ColocaterANN::CELL_FACETS] ) {
                delete mesh_.ann_[ColocaterANN::CELL_FACETS] ;
                mesh_.ann_[ColocaterANN::CELL_FACETS] = nil ;
            }
        }
        /*!
         * @brief Deletes the AABB on cells
         */
        void delete_cell_aabb()
        {
            if( mesh_.cells_aabb_ ) {
                delete mesh_.cells_aabb_;
                mesh_.cells_aabb_ = nil ;
            }
        }

        /*!
         * @}
         */

    private:
        Mesh& mesh_ ;
    } ;
}

#endif
