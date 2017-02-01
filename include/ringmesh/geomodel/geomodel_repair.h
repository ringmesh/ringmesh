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

#ifndef __RINGMESH_GEOMODEL_REPAIR__
#define __RINGMESH_GEOMODEL_REPAIR__

#include <ringmesh/basic/common.h>

#include <ringmesh/geomodel/geomodel_builder.h>

/*!
 * @file ringmesh/geomodel_repair.h
 * @brief Functions to repair GeoModel.
 * @author Jeanne Pellerin
 */

namespace RINGMesh {
    /*!
     * @brief Try repairing a supposedly invalid GeoModel
     * @details Remove colocated vertices in all GeoModelMeshEntity.
     *          Remove degenerated edges and facets in Surfaces and Lines.
     * @warning The Mesh of the geomodel is deleted.
     *          This function will by no mean fix all errors in a GeoModel
     *          It has been tested on a very small number of geomodels.
     *
     * @todo Convenience design to change. This allows an easy access for repair
     * to the internal meshes of the GeoModel. This class is otherwise artificial.
     */
//    class RINGMESH_API GeoModelRepair: public GeoModelBuilder {
//        ringmesh_disable_copy( GeoModelRepair ) ;
//    public:
//        /*!
//         * Enumeration of the different repair modes.
//         */
//        enum RepairMode {
//            ALL,
//            BASIC,
//            COLOCATED_VERTICES,
//            DEGENERATE_FACETS_EDGES,
//            LINE_BOUNDARY_ORDER
//        } ;
//    public:
//        GeoModelRepair( GeoModel& geomodel )
//            : GeoModelBuilder( geomodel )
//        {
//        }
//        virtual ~GeoModelRepair()
//        {
//        }
//        /*!
//         * @brief repair a GeoModel according a repair mode.
//         * @param[in] repair_mode repair mode to apply.
//         */
//        void repair( RepairMode repair_mode ) ;
//    private:
//        /*!
//         * All implemented repair for a GeoModel.
//         */
//        void geomodel_mesh_repair() ;
//        /*!
//         * Removes the colocated vertices in all the GeoModelMeshEntity within
//         * the GeoModel. GeoModelMeshEntity without any vertex anymore
//         * (after the removal of the vertices) are removed off the GeoModel.
//         */
//        void remove_colocated_entity_vertices_and_update_gm() ;
//        /*!
//         * Removes the degenerated facets in all the Surface and all the
//         * degenerated edges in all the Line within
//         * the GeoModel. Degeneration is due to colocated vertices.
//         * Surface and Line without any vertex anymore
//         * (after the removal of the vertices) are removed off the GeoModel.
//         */
//        void remove_degenerate_facets_and_edges_and_update_gm() ;
//        /*!
//         * @brief For all the lines in the geomodel, switch line boundaries
//         * if the way of their indices do not follow the way of the vertex indices.
//         */
//        void repair_line_boundary_vertex_order() ;
//        /*!
//         * @brief Detect and remove degenerated edges in a \param line.
//         * @return the number of degenerated edges that have been removed from the line.
//         */
//        index_t repair_line_mesh( Line& line ) ;
//        void line_detect_degenerate_edges(
//            const Line& L,
//            std::vector< bool >& e_is_degenerate,
//            std::vector< index_t >& colocated_vertices ) ;
//        /*!
//         * \note Copied and modified from geogram\mesh\mesh_repair.cpp
//         */
//        void surface_detect_degenerate_facets(
//            const Surface& S,
//            std::vector< index_t >& f_is_degenerate,
//            std::vector< index_t >& colocated_vertices ) ;
//        /*!
//         * \note Copied and modified from geogram\mesh\mesh_repair.cpp
//         *
//         * @brief Tests whether a facet is degenerate.
//         * @param[in] S the Surface that the facet belongs to
//         * @param[in] f the index of the facet in \p S
//         * @param[out] colocated_vertices contains the found colocated vertices
//         * in \p f if any.
//         * \return true if facet \p f has duplicated vertices,
//         *  false otherwise
//         */
//        bool facet_is_degenerate(
//            const Surface& S,
//            index_t f,
//            std::vector< index_t >& colocated_vertices ) ;
//
//        /*!
//         * @brief Detect and remove degenerated facets in a Surface
//         * @param[in,out] S Surface to check for potential degenerate facets.
//         * @return the number of degenerate facets in \p S.
//         */
//        index_t detect_degenerate_facets( Surface& S ) ;
//
//        /*!
//         * @brief Remove degenerate facets and edges from the Surface
//         *        and Line of the geomodel.
//         * @param[out] to_remove gme_t of the entities (Surface and Line)
//         * of the geomodel that are empty once degenerate entities are removed
//         * @pre Colocated vertices have already been removed
//         */
//        void remove_degenerate_facets_and_edges( std::set< gme_t >& to_remove ) ;
//
//        /*!
//         * @brief Remove colocated vertices of the geomodel.
//         * @param[out] to_remove gme_t of the entities of the geomodel that
//         *  are empty once degenerate entities are removed
//         */
//        void remove_colocated_entity_vertices( std::set< gme_t >& to_remove ) ;
//        /*!
//         * Get the indices of the duplicated vertices that are on an inside border.
//         * Only the vertex with the biggest index are added.
//         * @param[in] E_id GeoModelMeshEntity to check.
//         * @param[out] vertices vector of the vertex indexes on an inside boundary.
//         */
//        void vertices_on_inside_boundary(
//            const gme_t& E_id,
//            std::set< index_t >& vertices ) ;
//
//        /*!
//         * @brief Checks if an edge is degenerate.
//         *
//         * An edge is degenerate if both vertices are colocated.
//         *
//         * @param[in] L Line to check the edge \p e.
//         * @param[in] e edge index in Line \p L.
//         * @param[in] colocated_vertices contains the colocated mapping of the Line.
//         * @return true if the edge is degenerate. Else false.
//         */
//        bool edge_is_degenerate(
//            const Line& L,
//            index_t e,
//            const std::vector< index_t >& colocated_vertices )
//        {
//            index_t v1 = colocated_vertices[L.mesh_element_vertex_index( e, 0 )] ;
//            index_t v2 = colocated_vertices[L.mesh_element_vertex_index( e, 1 )] ;
//            return v1 == v2 ;
//        }
//    } ;

} //namespace RINGMesh

#endif 
