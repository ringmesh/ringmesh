/*
 * Copyright (c) 2012-2015, Association Scientifique pour la Geologie et ses Applications (ASGA)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the <organization> nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  Contacts:
 *     Arnaud.Botella@univ-lorraine.fr
 *     Antoine.Mazuyer@univ-lorraine.fr
 *     Jeanne.Pellerin@wias-berlin.de
 *
 *     http://www.gocad.org
 *
 *     GOCAD Project
 *     Ecole Nationale Superieure de Geologie - Georessources
 *     2 Rue du Doyen Marcel Roubault - TSA 70605
 *     54518 VANDOEUVRE-LES-NANCY
 *     FRANCE
 */

/*! \author Jeanne Pellerin and Arnaud Botella */

#ifndef __RINGMESH_GFX__
#define __RINGMESH_GFX__

#ifdef RINGMESH_WITH_GRAPHICS

#include <ringmesh/common.h>

#include <geogram_gfx/mesh/mesh_gfx.h>

namespace RINGMesh {
    class BoundaryModel ;
    class BoundaryModelMeshElement ;
    class Corner ;
    class Line ;
    class Surface ;
}

namespace RINGMesh {

    class RINGMESH_API BoundaryModelMeshElementGfx {
        ringmesh_disable_copy( BoundaryModelMeshElementGfx ) ;
    public:
        BoundaryModelMeshElementGfx( const BoundaryModelMeshElement& mesh ) ;

        GEO::MeshGfx& gfx() {
            return gfx_ ;
        }

        void set_vertices_visible( bool b ) { vertices_visible_ = b ; }
        bool get_vertices_visible() const { return vertices_visible_ ; }

    protected:
        GEO::MeshGfx gfx_ ;

        bool vertices_visible_ ;
    } ;

    class RINGMESH_API CornerGfx: public BoundaryModelMeshElementGfx {
    public:
        CornerGfx( const Corner& corner ) ;
    } ;

    class RINGMESH_API LineGfx: public BoundaryModelMeshElementGfx {
    public:
        LineGfx( const Line& line ) ;
    } ;

    class RINGMESH_API SurfaceGfx: public BoundaryModelMeshElementGfx {
    public:
        SurfaceGfx( const Surface& surface ) ;
    } ;



    class RINGMESH_API BoundaryModelGfx {
        ringmesh_disable_copy( BoundaryModelGfx ) ;
    public:
        BoundaryModelGfx() ;

        void set_boundary_model( const BoundaryModel& model ) ;
        void initialize() ;

        void draw_corners() ;
        void draw_lines() ;
        void draw_surfaces() ;

        void set_corners_color( float r, float g, float b ) ;
        void set_corner_color( index_t c, float r, float g, float b ) ;
        void set_corners_visibility( bool b ) ;
        void set_corner_visibility( index_t c, bool b ) ;
        void set_corners_size( index_t s ) ;
        void set_corner_size( index_t c, index_t s ) ;

    private:

        const BoundaryModel* model_ ;

        // Base manifold elements of a model
        std::vector< CornerGfx* > corners_ ;
        std::vector< LineGfx* > lines_ ;
        std::vector< SurfaceGfx* > surfaces_ ;

    } ;

}

#endif
#endif
