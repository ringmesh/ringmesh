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

#ifdef RINGMESH_WITH_GRAPHICS

#include <memory>

#include <geogram_gfx/glup_viewer/glup_viewer_gui.h>

#include <ringmesh/geomodel/geomodel_mesh_entity.h>

/*!
 * @file Classes for GeoModel visualization
 * @author Benjamin Chauvin and Arnaud Botella
 */

namespace RINGMesh {
    class GeoModel;
    class GeoModelGfx;
    class AttributeGfx;
    class PointSetGfx;
    class LineGfx;
    class SurfaceGfx;
    class VolumeGfx;
    class MeshEntityGfx;
    class MeshEntityGfx2;
}

namespace RINGMesh {

#define COMMON_GEOGRAM_GFX_IMPLEMENTATION \
    public:\
    virtual void draw_vertices() override \
    {\
        mesh_gfx_.draw_vertices();\
    }\
    virtual void set_vertices_color( float r, float g, float b ) override \
    {\
        mesh_gfx_.set_points_color( r, g, b );\
    }\
    virtual void set_vertices_size( index_t s ) override\
    {\
        mesh_gfx_.set_points_size( static_cast< float >( s ) );\
    }\
    virtual void set_scalar_attribute(\
        GEO::MeshElementsFlags subelements,\
        const std::string& name,\
        double attr_min,\
        double attr_max,\
        GLuint colormap_texture ) override\
    {\
        mesh_gfx_.set_scalar_attribute( subelements, name, attr_min, attr_max,\
            colormap_texture );\
    }\
    virtual void unset_scalar_attribute() override\
    {\
        mesh_gfx_.unset_scalar_attribute();\
    }\
private:\
    GEO::MeshGfx mesh_gfx_

    class GeogramPointSetGfx: public PointSetGfx {
    COMMON_GEOGRAM_GFX_IMPLEMENTATION;
    public:
        GeogramPointSetGfx( const GeoModelGfx& gfx, const Corner& corner )
            : PointSetGfx( gfx )
        {
            mesh_gfx_.set_mesh( &corner.low_level_mesh_storage().gfx_mesh() );
            set_vertices_color( 1, 0, 0 );
        }

    };

    class GeogramLineGfx: public LineGfx {
    COMMON_GEOGRAM_GFX_IMPLEMENTATION;
    public:
        GeogramLineGfx( const GeoModelGfx& gfx, const Line& line )
            : LineGfx( gfx )
        {
            mesh_gfx_.set_mesh( &line.low_level_mesh_storage().gfx_mesh() );
            set_vertices_color( 1, 1, 1 );
            set_edges_color( 1, 1, 1 );
        }

        virtual void draw_edges() override
        {
            mesh_gfx_.draw_edges();
        }
        virtual void set_edges_color( float r, float g, float b ) override
        {
            mesh_gfx_.set_mesh_color( r, g, b );
        }
        virtual void set_edges_width( index_t s ) override
        {
            mesh_gfx_.set_mesh_width( s );
        }

    };

    class GeogramSurfaceGfx: public SurfaceGfx {
    COMMON_GEOGRAM_GFX_IMPLEMENTATION;
    public:
        GeogramSurfaceGfx( const GeoModelGfx& gfx, const Surface& surface )
            : SurfaceGfx( gfx )
        {
            mesh_gfx_.set_mesh( &surface.low_level_mesh_storage().gfx_mesh() );
        }

        virtual void draw_surface() override
        {
            mesh_gfx_.draw_surface();
        }
        virtual void set_surface_color( float r, float g, float b ) override
        {
            mesh_gfx_.set_surface_color( r, g, b );
        }
        virtual void set_backface_surface_color( float r, float g, float b ) override
        {
            mesh_gfx_.set_backface_surface_color( r, g, b );
        }
        virtual void set_mesh_color( float r, float g, float b ) override
        {
            mesh_gfx_.set_mesh_color( r, g, b );
        }
        virtual void set_mesh_visibility( bool b ) override
        {
            mesh_gfx_.set_show_mesh( b );
        }
        virtual void set_mesh_width( index_t s ) override
        {
            mesh_gfx_.set_mesh_width( s );
        }
    };

    class GeogramVolumeGfx: public VolumeGfx {
    COMMON_GEOGRAM_GFX_IMPLEMENTATION;
    public:
        GeogramVolumeGfx( const GeoModelGfx& gfx, const Region& region )
            : VolumeGfx( gfx )
        {
            mesh_gfx_.set_mesh( &region.low_level_mesh_storage().gfx_mesh() );
        }

        virtual void draw_volume() override
        {
            mesh_gfx_.draw_volume();
        }
        virtual void set_draw_cells( GEO::MeshCellType type, bool x ) override
        {
            mesh_gfx_.set_draw_cells( type, x );
        }
        virtual void set_cells_colors_by_type() override
        {
            mesh_gfx_.set_cells_colors_by_type();
        }
        virtual void set_cells_color( float r, float g, float b ) override
        {
            mesh_gfx_.set_cells_color( r, g, b );
        }
        virtual void set_mesh_color( float r, float g, float b ) override
        {
            mesh_gfx_.set_mesh_color( r, g, b );
        }
        virtual void set_mesh_visibility( bool b ) override
        {
            mesh_gfx_.set_show_mesh( b );
        }
        virtual void set_mesh_width( index_t s ) override
        {
            mesh_gfx_.set_mesh_width( s );
        }
        virtual void set_shrink( double s ) override
        {
            mesh_gfx_.set_shrink( s );
        }
    };

}

#endif
