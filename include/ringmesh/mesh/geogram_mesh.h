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

#ifndef __RINGMESH_GEOGRAM_MESH__
#define __RINGMESH_GEOGRAM_MESH__

#include <ringmesh/basic/common.h>

#include <geogram/basic/command_line.h>
#include <geogram/mesh/mesh.h>
#include <geogram/mesh/mesh_io.h>
#include <geogram/mesh/mesh_geometry.h>
#include <geogram/mesh/mesh_topology.h>

#include <ringmesh/basic/geometry.h>
#include <ringmesh/geogram_extension/geogram_extension.h>
#include <ringmesh/mesh/aabb.h>

namespace RINGMesh {
    class GeoModel ;
    class MeshBaseBuilder ;
    class Mesh0DBuilder ;
    class Mesh1DBuilder ;
    class Mesh2DBuilder ;
    class Mesh3DBuilder ;
    class MeshAllDBuilder ;
    class GeogramMeshBuilder ;
}

namespace RINGMesh {

    /*!
     * @brief class to encapsulate mesh structure in order to provide an API
     * on which we base the RINGMesh algorithm
     * @note For now, we encapsulate the GEO::Mesh class. We can develop the concept
     * using a factory to build several encapsulating classes.
     */
    class RINGMESH_API GeogramMesh: public MeshAllD {
    ringmesh_disable_copy( GeogramMesh ) ;

        friend class GeogramMeshBuilder ;

    public:
        /*!
         * @brief Mesh constructor.
         * @param[in] dimension dimension of the vertices.
         * @param[in] single_precision if true, vertices are stored in single precision (float),
         * else they are stored as double precision (double).
         */
        GeogramMesh(
            index_t dimension,
            bool single_precision )
            : MeshAllD()
        {
            mesh_ = new GEO::Mesh( dimension, single_precision ) ;
        }
        ~GeogramMesh()
        {
            delete mesh_ ;
        }
        /*!
         * \name MeshBase implementation
         * @{
         */
        void save_mesh(
            const std::string& filename,
            const GEO::MeshIOFlags& ioflags ) const
        {
            GEO::mesh_save( *mesh_, filename, ioflags ) ;
        }

        /*!
         * get access to GEO::MESH... only for GFX..
         * @todo Remove this function as soon as the GEO::MeshGFX is encapsulated
         */
        const GEO::Mesh& gfx_mesh() const
        {
            return *mesh_ ;
        }

        //TODO maybe reimplement the function with a RINGMesh::Mesh??
        void print_mesh_bounded_attributes() const
        {
            print_bounded_attributes( *mesh_ ) ;
        }

        /*
         * @brief Gets a point.
         * @param[in] v_id the vertex, in 0.. @function nb_vetices()-1.
         * @return const reference to the point that corresponds to the vertex.
         */
        const vec3& vertex( index_t v_id ) const
        {
            ringmesh_assert( v_id < nb_vertices() ) ;
            return mesh_->vertices.point( v_id ) ;
        }

        /*
         * @brief Gets the number of point in the Mesh.
         */
        index_t nb_vertices() const
        {
            return mesh_->vertices.nb() ;
        }
        GEO::AttributesManager& vertex_attribute_manager() const
        {
            return mesh_->vertices.attributes() ;
        }

        /*!@}
         * \name Mesh1D implementation
         * @{
         */

        /*
         * @brief Gets the index of an edge vertex.
         * @param[in] edge_id index of the edge.
         * @param[in] vertex_id local index of the vertex, in {0,1}
         * @return the global index of vertex \param vertex_id in edge \param edge_id.
         */
        index_t edge_vertex( index_t edge_id, index_t vertex_id ) const
        {
            return mesh_->edges.vertex( edge_id, vertex_id ) ;
        }
        /*!
         * @brief Gets the number of all the edges in the whole Mesh.
         */
        index_t nb_edges() const
        {
            return mesh_->edges.nb() ;
        }

        GEO::AttributesManager& edge_attribute_manager() const
        {
            return mesh_->edges.attributes() ;
        }

        /*!@}
         * \name Mesh2D implementation
         * @{
         */

        /*!
         * @brief Gets the vertex index by facet index and local vertex index.
         * @param[in] facet_id the facet index.
         * @param[in] vertex_id the local edge index in \param facet_id.
         * @return the global facet index adjacent to the \param edge_id of the facet \param facet_id.
         * @precondition  \param edge_id < number of edge of the facet \param facet_id .
         */
        index_t facet_vertex( index_t facet_id, index_t vertex_id ) const
        {
            return mesh_->facets.vertex( facet_id, vertex_id ) ;
        }
        /*!
         * @brief Gets the number of all facets in the whole Mesh.
         */
        index_t nb_facets() const
        {
            return mesh_->facets.nb() ;
        }
        /*!
         * @brief Gets the number of vertices in the facet \param facet_id.
         * @param[in] facet_id facet index
         */
        index_t nb_facet_vertices( index_t facet_id ) const
        {
            return mesh_->facets.nb_vertices( facet_id ) ;
        }

        /*!
         * @brief Gets an adjacent facet index by facet index and local edge index.
         * @param[in] facet_id the facet index.
         * @param[in] edge_id the local edge index in \param facet_id.
         * @return the global facet index adjacent to the \param edge_id of the facet \param facet_id.
         * @precondition  \param edge_id < number of edge of the facet \param facet_id .
         */
        index_t facet_adjacent( index_t facet_id, index_t edge_id ) const
        {
            return mesh_->facets.adjacent( facet_id, edge_id ) ;
        }
        GEO::AttributesManager& facet_attribute_manager() const
        {
            return mesh_->facets.attributes() ;
        }
        /*!
         * @brief Tests whether all the facets are triangles. when all the facets are triangles, storage and access is optimized.
         * @return True if all facets are triangles and False otherwise.
         */
        bool facets_are_simplicies() const
        {
            return mesh_->facets.are_simplices() ;
        }

        /*!
         * Computes the Mesh facet area
         * @param[in] facet_id the facet index
         * @return the facet area
         */
        double facet_area( index_t facet_id ) const
        {
            return GEO::Geom::mesh_facet_area( *mesh_, facet_id ) ;
        }

        /*!@}
         * \name Mesh3D implementation
         * @{
         */

        /*!
         * @brief Gets a vertex index by cell and local vertex index.
         * @param[in] cell_id the cell index.
         * @param[in] vertex_id the local vertex index in \param cell_id.
         * @return the global vertex index.
         * @precondition vertex_id<number of vertices of the cell.
         */
        index_t cell_vertex( index_t cell_id, index_t vertex_id ) const
        {
            return mesh_->cells.vertex( cell_id, vertex_id ) ;
        }
        /*!
         * @brief Gets a vertex index by cell and local edge and local vertex index.
         * @param[in] cell_id the cell index.
         * @param[in] edge_id the local edge index in \param cell_id.
         * @param[in] vertex_id the local vertex index in \param cell_id.
         * @return the global vertex index.
         * @precondition vertex_id<number of vertices of the cell.
         */
        index_t cell_edge_vertex(
            index_t cell_id,
            index_t edge_id,
            index_t vertex_id ) const
        {
            return mesh_->cells.edge_vertex( cell_id, edge_id, vertex_id ) ;
        }
        /*!
         * @brief Gets a vertex by cell facet and local vertex index.
         * @param[in] cell_id index of the cell
         * @param[in] facet_id index of the facet in the cell \param cell_id
         * @param[in] vertex_id index of the vertex in the facet \param facet_id
         * @return the global vertex index.
         * @precondition vertex_id < number of vertices in the facet \param facet_id
         * and facet_id number of facet in th cell \param cell_id
         */
        index_t cell_facet_vertex(
            index_t cell_id,
            index_t facet_id,
            index_t vertex_id ) const
        {
            return mesh_->cells.facet_vertex( cell_id, facet_id, vertex_id ) ;
        }
        /*!
         * @brief Gets a facet index by cell and local facet index.
         * @param[in] cell_id index of the cell
         * @param[in] facet_id index of the facet in the cell \param cell_id
         * @return the global facet index.
         */
        index_t cell_facet( index_t cell_id, index_t facet_id ) const
        {
            return mesh_->cells.facet( cell_id, facet_id ) ;
        }
        /*!
         * @brief Gets the number of facet in a cell
         * @param[in] cell_id index of the cell
         * @return the number of facet of the cell \param cell_id
         */
        index_t nb_cell_facets( index_t cell_id ) const
        {
            return mesh_->cells.nb_facets( cell_id ) ;
        }
        /*!
         * @brief Gets the total number of facet in all cell
         */
        index_t nb_cell_facets() const
        {
            return mesh_->cell_facets.nb() ;
        }
        /*!
         * @brief Gets the number of edges in a cell
         * @param[in] cell_id index of the cell
         * @return the number of facet of the cell \param cell_id
         */
        index_t nb_cell_edges( index_t cell_id ) const
        {
            return mesh_->cells.nb_edges( cell_id ) ;
        }
        /*!
         * @brief Gets the number of vertices of a facet in a cell
         * @param[in] cell_id index of the cell
         * @param[in] facet_id index of the facet in the cell \param cell_id
         * @return the number of vertices in the facet \param facet_id in the cell \param cell_id
         */
        index_t nb_cell_facet_vertices( index_t cell_id, index_t facet_id ) const
        {
            return mesh_->cells.facet_nb_vertices( cell_id, facet_id ) ;
        }
        /*!
         * @brief Gets the number of vertices of a cell
         * @param[in] cell_id index of the cell
         * @return the number of vertices in the cell \param cell_id
         */
        index_t nb_cell_vertices( index_t cell_id ) const
        {
            return mesh_->cells.nb_vertices( cell_id ) ;
        }
        /*!
         * @brief Gets the number of cells in the Mesh.
         */
        index_t nb_cells() const
        {
            return mesh_->cells.nb() ;
        }

        index_t cell_begin( index_t cell_id ) const
        {
            return mesh_->cells.corners_begin( cell_id ) ;
        }
        index_t cell_end( index_t cell_id ) const
        {
            return mesh_->cells.corners_end( cell_id ) ;
        }
        /*!
         * @return the index of the adjacent cell of \param cell_id along the facet \param facet_id
         */
        index_t cell_adjacent( index_t cell_id, index_t facet_id ) const
        {
            return mesh_->cells.adjacent( cell_id, facet_id ) ;
        }
        GEO::AttributesManager& cell_attribute_manager() const
        {
            return mesh_->cells.attributes() ;
        }
        GEO::AttributesManager& cell_facet_attribute_manager() const
        {
            return mesh_->cell_facets.attributes() ;
        }
        /*!
         * @brief Gets the type of a cell.
         * @param[in] cell_id the cell index, in 0..nb()-1
         */
        GEO::MeshCellType cell_type( index_t cell_id ) const
        {
            return mesh_->cells.type( cell_id ) ;
        }
        /*!
         * @brief Tests whether all the cells are tetrahedra. when all the cells are tetrahedra, storage and access is optimized.
         * @return True if all cells are tetrahedra and False otherwise.
         */
        bool cells_are_simplicies() const
        {
            return mesh_->cells.are_simplices() ;
        }

        /*!
         * @brief compute the volume of the cell \param cell_id.
         */
        double cell_volume( index_t cell_id ) const
        {
            return RINGMesh::mesh_cell_volume( *mesh_, cell_id ) ;
        }

        /*!
         * @}
         */

        GeogramMeshBuilder* get_geogram_mesh_builder() ;
    protected:
        virtual MeshBaseBuilder* get_mesh_builder_base() ;

    private:
        mutable GEO::Mesh* mesh_ ;

    } ;
}

#endif
