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
#include <ringmesh/mesh/geogram_mesh.h>

#include <ringmesh/geomodel/geo_model_entity.h>
#include <ringmesh/geomodel/geo_model.h>
#include <ringmesh/mesh/mesh_builder.h>
#include <ringmesh/mesh/geogram_mesh_builder.h>

namespace RINGMesh {
    MeshBase::~MeshBase()
    {
        if( mesh_builder_ ) delete mesh_builder_ ;
        if( vertices_ann_ ) delete vertices_ann_ ;
    }
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

    GeogramMesh0DBuilder* GeogramMesh0D::get_geogram_mesh_builder()
    {
        return dynamic_cast< GeogramMesh0DBuilder* >( get_mesh_builder_base() ) ;
    }
    MeshBaseBuilder* GeogramMesh0D::get_mesh_builder_base()
    {
        if( mesh_builder_ == nil ) {
            mesh_builder_ = new GeogramMesh0DBuilder( *this ) ;
        }
        return mesh_builder_ ;
    }


    GeogramMesh1DBuilder* GeogramMesh1D::get_geogram_mesh_builder()
    {
        return dynamic_cast< GeogramMesh1DBuilder* >( get_mesh_builder_base() ) ;
    }
    MeshBaseBuilder* GeogramMesh1D::get_mesh_builder_base()
    {
        if( mesh_builder_ == nil ) {
            mesh_builder_ = new GeogramMesh1DBuilder( *this ) ;
        }
        return mesh_builder_ ;
    }


    GeogramMesh2DBuilder* GeogramMesh2D::get_geogram_mesh_builder()
    {
        return dynamic_cast< GeogramMesh2DBuilder* >( get_mesh_builder_base() ) ;
    }
    MeshBaseBuilder* GeogramMesh2D::get_mesh_builder_base()
    {
        if( mesh_builder_ == nil ) {
            mesh_builder_ = new GeogramMesh2DBuilder( *this ) ;
        }
        return mesh_builder_ ;
    }

    GeogramMesh3DBuilder* GeogramMesh3D::get_geogram_mesh_builder()
    {
        return dynamic_cast< GeogramMesh3DBuilder* >( get_mesh_builder_base() ) ;
    }
    MeshBaseBuilder* GeogramMesh3D::get_mesh_builder_base()
    {
        if( mesh_builder_ == nil ) {
            mesh_builder_ = new GeogramMesh3DBuilder( *this ) ;
        }
        return mesh_builder_ ;
    }

    GeogramMeshAllDBuilder* GeogramMeshAllD::get_geogram_mesh_builder()
    {
        return dynamic_cast< GeogramMeshAllDBuilder* >( get_mesh_builder_base() ) ;
    }
    MeshBaseBuilder* GeogramMeshAllD::get_mesh_builder_base()
    {
        if( mesh_builder_ == nil ) {
            mesh_builder_ = new GeogramMeshAllDBuilder( *this ) ;
        }
        return mesh_builder_ ;
    }

    Mesh0D* Mesh0D::create_mesh( const MeshType type )
    {
        Mesh0D* mesh = Mesh0DFactory::create_object( type ) ;
        if( !mesh ) {
            Logger::warn( "Mesh0D" )
                << "Could not create mesh data structure: " << type
                << std::endl ;
            Logger::warn( "Mesh0D" ) << "Falling back to GeogramMesh0D data structure"
                << std::endl ;

            mesh = new GeogramMesh0D ;
        }
        return mesh ;
    }

    Mesh1D* Mesh1D::create_mesh( const MeshType type )
    {
        Mesh1D* mesh = Mesh1DFactory::create_object( type ) ;
        if( !mesh ) {
            Logger::warn( "Mesh1D" )
                << "Could not create mesh data structure: " << type
                << std::endl ;
            Logger::warn( "Mesh1D" ) << "Falling back to GeogramMesh1D data structure"
                << std::endl ;

            mesh = new GeogramMesh1D ;
        }
        return mesh ;
    }

    Mesh2D* Mesh2D::create_mesh( const MeshType type )
    {
        Mesh2D* mesh = Mesh2DFactory::create_object( type ) ;
        if( !mesh ) {
            Logger::warn( "Mesh2D" )
                << "Could not create mesh data structure: " << type
                << std::endl ;
            Logger::warn( "Mesh2D" ) << "Falling back to GeogramMesh2D data structure"
                << std::endl ;

            mesh = new GeogramMesh2D ;
        }
        return mesh ;
    }

    Mesh3D* Mesh3D::create_mesh( const MeshType type )
    {
        Mesh3D* mesh = Mesh3DFactory::create_object( type ) ;
        if( !mesh ) {
            Logger::warn( "Mesh3D" )
                << "Could not create mesh data structure: " << type
                << std::endl ;
            Logger::warn( "Mesh3D" ) << "Falling back to GeogramMesh3D data structure"
                << std::endl ;

            mesh = new GeogramMesh3D ;
        }
        return mesh ;
    }

    MeshAllD* MeshAllD::create_mesh( const MeshType type )
    {
        MeshAllD* mesh = MeshAllDFactory::create_object( type ) ;
        if( !mesh ) {
            Logger::warn( "MeshAllD" )
                << "Could not create mesh data structure: " << type
                << std::endl ;
            Logger::warn( "MeshAllD" ) << "Falling back to GeogramMeshAllD data structure"
                << std::endl ;

            mesh = new GeogramMeshAllD ;
        }
        return mesh ;
    }


} // namespace
