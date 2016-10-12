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

/*! \author Francois Bonneau */

#include <ringmesh/mesh/mesh.h>

#include <ringmesh/geomodel/geo_model_entity.h>
#include <ringmesh/geomodel/geo_model.h>
#include <ringmesh/mesh/mesh_builder.h>

namespace RINGMesh {

    Mesh0DBuilder* Mesh0D::get_mesh0d_builder()
    {
        return dynamic_cast< Mesh0DBuilder* >( get_mesh_builder_base() ) ;
    }
    Mesh1DBuilder* Mesh1D::get_mesh1d_builder()
    {
        return dynamic_cast< Mesh1DBuilder* >( get_mesh_builder_base() ) ;
    }
    Mesh2DBuilder* Mesh2D::get_mesh2d_builder()
    {
        return dynamic_cast< Mesh2DBuilder* >( get_mesh_builder_base() ) ;
    }
    Mesh3DBuilder* Mesh3D::get_mesh3d_builder()
    {
        return dynamic_cast< Mesh3DBuilder* >( get_mesh_builder_base() ) ;
    }
    MeshAllDBuilder* MeshAllD::get_meshalld_builder()
    {
        return dynamic_cast< MeshAllDBuilder* >( get_mesh_builder_base() ) ;
    }

    GeogramMeshBuilder* GeogramMesh::get_geogram_mesh_builder()
    {
        return dynamic_cast< GeogramMeshBuilder* >( get_mesh_builder_base() ) ;
    }
    MeshBaseBuilder* GeogramMesh::get_mesh_builder_base()
    {
        if( mesh_builder_ == NULL ) {
            mesh_builder_ = new GeogramMeshBuilder( *this ) ;
        }
        return mesh_builder_ ;
    }

    const GEO::MeshFacetsAABB& GeogramMesh::facets_aabb() const
    {
        GeoModel& M = const_cast< GeoModel& >( geo_model_ ) ;
        if( facets_aabb_ == nil ) {
            // Geogram triangulates the Mesh when creating the AABB tree
            ringmesh_assert( mesh_->facets.are_simplices() ) ;

            // Very bad side effect
            // The root cause of the problem is the duplication of many things
            // in our GeoModel structure [JP]
            if( M.mesh.vertices.is_initialized() ) {
                M.mesh.vertices.clear() ;
            }
            GeogramMeshBuilder* builder = get_geogram_mesh_builder() ;
            builder.clear_vertex_linked_objects() ;

            facets_aabb_ = new GEO::MeshFacetsAABB( *mesh_ ) ;
        }
        return *facets_aabb_ ;
    }

    const GEO::MeshCellsAABB& GeogramMesh::cells_aabb() const
    {
        GeoModel& M = const_cast< GeoModel& >( geo_model_ ) ;
        if( cells_aabb_ == nil ) {
            // Very bad side effect
            // The root cause of the problem is the duplication of many things
            // in our GeoModel structure [JP]
            if( M.mesh.vertices.is_initialized() ) {
                M.mesh.vertices.clear() ;
            }
            GeogramMeshBuilder* builder = get_geogram_mesh_builder() ;
            builder.clear_vertex_linked_objects() ;

            cells_aabb_ = new GEO::MeshCellsAABB( *mesh_ ) ;
        }
        return *cells_aabb_ ;
    }
} // namespace
