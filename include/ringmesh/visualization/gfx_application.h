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

#ifndef __RINGMESH_GFX_APPLICATION__
#define __RINGMESH_GFX_APPLICATION__

#include <ringmesh/basic/common.h>

#ifdef RINGMESH_WITH_GRAPHICS

#include <geogram_gfx/glup_viewer/glup_viewer_gui.h>

#include <ringmesh/basic/box3d.h>
#include <ringmesh/geomodel/geo_model.h>
#include <ringmesh/visualization/gfx.h>

/*!
 * @file Classes for GeoModel visualization
 * @author Benjamin Chauvin and Arnaud Botella
 */

namespace RINGMesh {
}

namespace RINGMesh {

    class RINGMESH_API RINGMeshApplication: public GEO::Application {
    public:
        enum ViewerType {
            GEOMODEL, MESH, NONE
        } ;

        RINGMeshApplication( int argc, char** argv ) ;
        ~RINGMeshApplication() ;

    protected:
        bool load_geogram( const std::string& filename ) ;
        bool load_geogram( GEO::Mesh& mesh ) ;
        virtual bool load( const std::string& filename ) ;


        bool can_load_geogram( const std::string& filename ) ;
        void browse_geogram( const std::string& path ) ;
        void browse_ringmesh( const std::string& path ) ;
        void update_region_of_interest() ;
    private:
        static RINGMeshApplication* instance() ;

        virtual std::string supported_read_file_extensions() ;
        std::string supported_geogram_read_file_extensions() ;
        virtual void init_graphics() ;
        virtual void draw_scene() ;
        virtual void draw_object_properties() ;
        virtual void draw_viewer_properties() ;
        virtual void draw_application_menus() ;
        virtual void draw_load_menu() ;


        static void increment_shrink() ;
        static void decrement_shrink() ;
        static void show_corners() ;
        static void show_lines() ;
        static void show_surface() ;
        static void show_volume() ;
        static void show_voi() ;
        static void mesh_visible() ;
        static void show_colormap() ;
        static void colored_cells() ;
        static void show_colored_regions() ;
        static void show_colored_layers() ;

    protected:
        class GeoModelViewer {
        public:
            GeoModelViewer( RINGMeshApplication& app, const std::string& filename ) ;

            void draw_scene() ;
            void draw_object_properties() ;
            void draw_viewer_properties() ;

            void draw_colormap() ;
            void toggle_colored_cells() ;
            void toggle_colored_regions() ;
            void toggle_colored_layers() ;

            void reset_attribute_name() ;
            void set_attribute_names( const GEO::AttributesManager& attributes ) ;
            void autorange() ;
        public:
            struct OldNewStatus {
                void operator=( bool value )
                {
                    old_status = value ;
                    new_status = value ;
                }
                bool need_to_update() const
                {
                    return old_status != new_status ;
                }
                void update()
                {
                    old_status = new_status ;
                }
                bool old_status ;
                bool new_status ;
            } ;

        public:
            RINGMeshApplication& app_ ;
            bool is_visible_ ;
            GeoModel GM_ ;
            GeoModelGfx GM_gfx_ ;
            Box3d bbox_ ;

            bool show_corners_ ;
            bool show_lines_ ;
            bool show_surface_ ;
            bool show_volume_ ;
            bool show_voi_ ;
            OldNewStatus colored_cells_ ;
            OldNewStatus show_colored_regions_ ;
            OldNewStatus show_colored_layers_ ;
            bool show_colormap_ ;

            bool show_hex_ ;
            bool show_prism_ ;
            bool show_pyramid_ ;
            bool show_tetra_ ;

            float shrink_ ;
            bool mesh_visible_ ;
            bool meshed_regions_ ;

            bool show_attributes_ ;
            float attribute_min_ ;
            float attribute_max_ ;
        } ;

        class MeshViewer {
        public:
            MeshViewer( RINGMeshApplication& app, const std::string& filename ) ;
            MeshViewer( RINGMeshApplication& app, GEO::Mesh& mesh ) ;

            void draw_object_properties() ;
            void draw_scene() ;

            void autorange() ;
            std::string attribute_names() ;
            void set_attribute( const std::string& attribute ) ;

        public:
            RINGMeshApplication& app_ ;
            bool is_visible_ ;
            GEO::Mesh mesh_ ;
            GEO::MeshGfx mesh_gfx_ ;
            Box3d bbox_ ;
            std::string name_ ;

            bool show_vertices_ ;
            float vertices_size_ ;

            bool show_surface_ ;
            bool show_surface_colors_ ;
            bool show_mesh_ ;
            bool show_surface_borders_ ;

            bool show_volume_ ;
            float cells_shrink_ ;
            bool show_colored_cells_ ;
            bool show_hexes_ ;

            bool show_attributes_ ;
            GLuint current_colormap_texture_ ;
            std::string attribute_ ;
            GEO::MeshElementsFlags attribute_subelements_ ;
            std::string attribute_name_ ;
            float attribute_min_ ;
            float attribute_max_ ;
        } ;

    protected:
        std::vector< GeoModelViewer* > models_ ;
        std::vector< MeshViewer* > meshes_ ;
        std::string ringmesh_file_extensions_ ;
        std::string geogram_file_extensions_ ;
        index_t current_viewer_ ;
        ViewerType current_viewer_type_ ;

    } ;
}

#endif
#endif
