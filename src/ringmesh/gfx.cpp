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

/*! 
 * @file Implementation of visualization of GeoModelElements
 * @author Benjamin Chaunvin and Arnaud Botella
 */

#include <ringmesh/gfx.h>

#ifdef RINGMESH_WITH_GRAPHICS

#include <geogram/mesh/mesh_geometry.h>
//#include <geogram_gfx/third_party/freeglut/glut.h>
//#include <geogram_gfx/third_party/freeglut/freeglut_ext.h>

#include <ringmesh/geo_model.h>
#include <ringmesh/geo_model_element.h>

#include <geogram/basic/logger.h>

#define define_color( name, r, g, b )\
    class name: public GetColor {\
    public:\
        virtual Color get_color() {\
            return Color( r, g, b ) ;\
        }\
    }; \
    ringmesh_register_color_creator( name, #name ) \

namespace {
    using namespace RINGMesh ;

    std::string get_attribute_name_with_coordinate(
        const std::string& name,
        index_t coordinate )
    {
        return name + "[" + GEO::String::to_string( coordinate ) + "]" ;
    }

    void compute_attribute_range(
        GEO::ReadOnlyScalarAttributeAdapter& attribute,
        double& min,
        double& max )
    {
        if( attribute.is_bound() ) {
            for( index_t i = 0; i < attribute.size(); ++i ) {
                double value = attribute[i] ;
                min = GEO::geo_min( min, value ) ;
                max = GEO::geo_max( max, value ) ;
            }
        }
    }
}
namespace RINGMesh {

    define_color( yellow, 0xff, 0xff, 0x00 );
    define_color( violet, 0x7f, 0x00, 0x7f );
    define_color( indigo, 0xbf, 0x00, 0xbf );
    define_color( blue, 0x00, 0x00, 0xff );
    define_color( black, 0x00, 0x00, 0x00 );
    define_color( orange, 0xff, 0x7f, 0x00 );
    define_color( white, 0xff, 0xff, 0xff );
    define_color( red, 0xff, 0x00, 0x00 );
    define_color( green, 0x00, 0xff, 0x00 );
    define_color( brown, 0x66, 0x33, 0x00 );
    define_color( purple, 0xa0, 0x20, 0xf0 );
    define_color( pink, 0xff, 0x69, 0xb4 );

    class MeshElementGfx: public GEO::MeshGfx {
    ringmesh_disable_copy( MeshElementGfx ) ;
    public:
        MeshElementGfx(
            const GeoModelGfx& gfx,
            const GEO::Mesh& mesh,
            bool vertice_visible )
            :
                vertices_visible_( vertice_visible ),
                gfx_( gfx )
        {
            set_mesh( &mesh ) ;
        }
        virtual ~MeshElementGfx()
        {
        }
        void need_to_update()
        {
            buffer_objects_dirty_ = true ;
            attributes_buffer_objects_dirty_ = true ;
        }

        void draw_vertices()
        {
            GEO::MeshGfx::draw_vertices() ;
        }
        virtual void draw_edges()
        {
            index_t w = get_mesh_width() ;
            set_mesh_width( w + 1 ) ;
            GEO::MeshGfx::draw_edges() ;
            set_mesh_width( w ) ;
        }

        void set_vertices_visible( bool b )
        {
            vertices_visible_ = b ;
        }
        bool get_vertices_visible() const
        {
            return vertices_visible_ ;
        }

    protected:
        bool vertices_visible_ ;

        const GeoModelGfx& gfx_ ;

    } ;

    class CornerGfx: public MeshElementGfx {
    public:
        CornerGfx( const GeoModelGfx& gfx, const Corner& corner )
            : MeshElementGfx( gfx, corner.mesh(), true )
        {
            set_points_color( 1, 0, 0 ) ;
        }
    } ;

    class LineGfx: public MeshElementGfx {
    public:
        LineGfx( const GeoModelGfx& gfx, const Line& line )
            : MeshElementGfx( gfx, line.mesh(), false ), edges_visible_( true )
        {
            set_points_color( 1, 1, 1 ) ;
            set_mesh_color( 1, 1, 1 ) ;
        }
        void set_edges_visible( bool b )
        {
            edges_visible_ = b ;
        }
        bool get_edges_visible() const
        {
            return edges_visible_ ;
        }

    private:
        bool edges_visible_ ;

    } ;

    class SurfaceGfx: public MeshElementGfx {
    public:
        SurfaceGfx( const GeoModelGfx& gfx, const Surface& surface )
            : MeshElementGfx( gfx, surface.mesh(), false ), surface_visible_( true )
        {
//            set_mesh_border_width(50) ;
        }

        virtual void draw_surface()
        {
            GEO::MeshGfx::draw_surface() ;
//            GEO::MeshGfx::draw_surface_borders() ;
        }

        void set_surface_visible( bool b )
        {
            surface_visible_ = b ;
        }
        bool get_surface_visible() const
        {
            return surface_visible_ ;
        }
    private:
        bool surface_visible_ ;

    } ;

    class RegionGfx: public MeshElementGfx {
    public:
        RegionGfx( const GeoModelGfx& gfx, const Region& region )
            :
                MeshElementGfx( gfx, region.mesh(), false ),
                region_visible_( true ),
                surface_visible_( false ),
                edges_visible_( false )
        {
            set_points_color( 0.0, 0.0, 0.0 ) ;
        }
        void set_edges_visible( bool b )
        {
            edges_visible_ = b ;
        }
        bool get_edges_visible() const
        {
            return edges_visible_ ;
        }
        void set_surface_visible( bool b )
        {
            surface_visible_ = b ;
        }
        bool get_surface_visible() const
        {
            return surface_visible_ ;
        }
        void set_region_visible( bool b )
        {
            region_visible_ = b ;
        }
        bool get_region_visible() const
        {
            return region_visible_ ;
        }

    private:
        bool region_visible_ ;
        bool surface_visible_ ;
        bool edges_visible_ ;

    } ;

    GeoModelGfx::GeoModelGfx()
        : model_( nil ), corners_(), lines_(), surfaces_(), regions_()
    {
        attribute_min_ = 0. ;
        attribute_max_ = 0. ;
    }

    GeoModelGfx::~GeoModelGfx()
    {
        for( index_t c = 0; c < corners_.size(); c++ ) {
            delete corners_[c] ;
        }
        for( index_t l = 0; l < lines_.size(); l++ ) {
            delete lines_[l] ;
        }
        for( index_t s = 0; s < surfaces_.size(); s++ ) {
            delete surfaces_[s] ;
        }
        for( index_t r = 0; r < regions_.size(); r++ ) {
            delete regions_[r] ;
        }
    }

    /*!
     * Sets the GeoModel associated to the graphics
     * @param[in] model the GeoModel
     */
    void GeoModelGfx::set_geo_model( const GeoModel& model )
    {
        model_ = &model ;
        initialize() ;
    }

    /*!
     * Gets the GeoModel associated to the graphics
     * @return the GeoModel
     */
    const GeoModel* GeoModelGfx::geo_model() const
    {
        return model_ ;
    }

    /*!
     * Initializes the database according the GeoModel dimensions
     */
    void GeoModelGfx::initialize()
    {
        ringmesh_assert( model_ ) ;
        if( corners_.empty() && lines_.empty() && surfaces_.empty() ) {
            corners_.resize( model_->nb_corners(), nil ) ;
            lines_.resize( model_->nb_lines(), nil ) ;
            surfaces_.resize( model_->nb_surfaces(), nil ) ;
            regions_.resize( model_->nb_regions(), nil ) ;

            for( index_t c = 0; c < corners_.size(); c++ ) {
                corners_[c] = new CornerGfx( *this, model_->corner( c ) ) ;
            }
            for( index_t l = 0; l < lines_.size(); l++ ) {
                lines_[l] = new LineGfx( *this, model_->line( l ) ) ;
            }
            for( index_t s = 0; s < surfaces_.size(); s++ ) {
                surfaces_[s] = new SurfaceGfx( *this, model_->surface( s ) ) ;
            }
            for( index_t r = 0; r < model_->nb_regions(); r++ ) {
                regions_[r] = new RegionGfx( *this, model_->region( r ) ) ;
            }
        }
    }

    void GeoModelGfx::need_to_update()
    {
        for( index_t c = 0; c < corners_.size(); c++ ) {
            corners_[c]->need_to_update() ;
        }
        for( index_t l = 0; l < lines_.size(); l++ ) {
            lines_[l]->need_to_update() ;
        }
        for( index_t s = 0; s < surfaces_.size(); s++ ) {
            surfaces_[s]->need_to_update() ;
        }
        for( index_t r = 0; r < model_->nb_regions(); r++ ) {
            regions_[r]->need_to_update() ;
        }
    }

    void GeoModelGfx::compute_cell_vertex_attribute_range(
        index_t coordinate,
        const std::string& name )
    {
        attribute_min_ = max_float64() ;
        attribute_max_ = min_float64() ;
        std::string attribute_name = get_attribute_name_with_coordinate( name,
            coordinate ) ;
        for( index_t r = 0; r < regions_.size(); r++ ) {
            GEO::ReadOnlyScalarAttributeAdapter attribute(
                model_->region( r ).vertex_attribute_manager(), attribute_name ) ;
            compute_attribute_range( attribute, attribute_min_,
                attribute_max_ ) ;
        }
    }

    void GeoModelGfx::compute_cell_attribute_range(
        index_t coordinate,
        const std::string& name )
    {
        attribute_min_ = max_float64() ;
        attribute_max_ = min_float64() ;
        std::string attribute_name = get_attribute_name_with_coordinate( name,
            coordinate ) ;
        for( index_t r = 0; r < regions_.size(); r++ ) {
            GEO::ReadOnlyScalarAttributeAdapter attribute(
                model_->region( r ).cell_attribute_manager(), attribute_name ) ;
            compute_attribute_range( attribute, attribute_min_,
                attribute_max_ ) ;
        }
    }

    void GeoModelGfx::set_cell_vertex_attribute(
        const std::string& name,
        index_t coordinate,
        GLuint colormap_texture )
    {
        std::string attribute_name = get_attribute_name_with_coordinate( name,
            coordinate ) ;
        for( index_t r = 0; r < regions_.size(); r++ ) {
            regions_[r]->set_scalar_attribute( GEO::MESH_VERTICES, attribute_name, attribute_min_,
                attribute_max_, colormap_texture ) ;
        }

    }

    void GeoModelGfx::set_cell_attribute(
        const std::string& name,
        index_t coordinate,
        GLuint colormap_texture )
    {
        std::string attribute_name = get_attribute_name_with_coordinate( name,
            coordinate ) ;
        for( index_t r = 0; r < regions_.size(); r++ ) {
            regions_[r]->set_scalar_attribute( GEO::MESH_CELLS, attribute_name, attribute_min_,
                attribute_max_, colormap_texture ) ;
        }

    }

    /*!
     * Draws the corners
     */
    void GeoModelGfx::draw_corners()
    {
        for( index_t c = 0; c < corners_.size(); c++ ) {
            if( corners_[c]->get_vertices_visible() ) corners_[c]->draw_vertices() ;
        }
    }
    /*!
     * Sets the corner color to all the corners
     * @param[in] r the red component of the color in [0.0, 1.0]
     * @param[in] g the green component of the color in [0.0, 1.0]
     * @param[in] b the blue component of the color in [0.0, 1.0]
     */
    void GeoModelGfx::set_corners_color( float r, float g, float b )
    {
        for( index_t c = 0; c < corners_.size(); c++ ) {
            set_corner_color( c, r, g, b ) ;
        }
    }
    /*!
     * Sets the corner color
     * @param[in] c the corner index
     * @param[in] r the red component of the color in [0.0, 1.0]
     * @param[in] g the green component of the color in [0.0, 1.0]
     * @param[in] b the blue component of the color in [0.0, 1.0]
     */
    void GeoModelGfx::set_corner_color( index_t c, float r, float g, float b )
    {
        corners_[c]->set_points_color( r, g, b ) ;
    }
    /*!
     * Sets the corner visibility to all the corners
     * @param[in] b the visibility
     */
    void GeoModelGfx::set_corners_visibility( bool b )
    {
        for( index_t c = 0; c < corners_.size(); c++ ) {
            set_corner_visibility( c, b ) ;
        }
    }
    /*!
     * Sets the corner visibility
     * @param[in] c the corner index
     * @param[in] b the visibility
     */
    void GeoModelGfx::set_corner_visibility( index_t c, bool b )
    {
        corners_[c]->set_vertices_visible( b ) ;
    }
    /*!
     * Sets the corner size to all the corners
     * @param[in] s the size
     */
    void GeoModelGfx::set_corners_size( index_t s )
    {
        for( index_t c = 0; c < corners_.size(); c++ ) {
            set_corner_size( c, s ) ;
        }
    }
    /*!
     * Sets the corner size
     * @param[in] c the corner index
     * @param[in] s the size
     */
    void GeoModelGfx::set_corner_size( index_t c, index_t s )
    {
        corners_[c]->set_points_size( float( s ) ) ;
    }

    /*!
     * Draws the lines
     */
    void GeoModelGfx::draw_lines()
    {
        for( index_t l = 0; l < lines_.size(); l++ ) {
            if( lines_[l]->get_vertices_visible() ) lines_[l]->draw_vertices() ;
            if( lines_[l]->get_edges_visible() ) lines_[l]->draw_edges() ;
        }
    }
    /*!
     * Sets the line color to all the lines
     * @param[in] r the red component of the color in [0.0, 1.0]
     * @param[in] g the green component of the color in [0.0, 1.0]
     * @param[in] b the blue component of the color in [0.0, 1.0]
     */
    void GeoModelGfx::set_edge_lines_color( float r, float g, float b )
    {
        for( index_t k = 0; k < lines_.size(); k++ ) {
            set_edge_line_color( k, r, g, b ) ;
        }
    }
    /*!
     * Sets the line color
     * @param[in] l the line index
     * @param[in] r the red component of the color in [0.0, 1.0]
     * @param[in] g the green component of the color in [0.0, 1.0]
     * @param[in] b the blue component of the color in [0.0, 1.0]
     */
    void GeoModelGfx::set_edge_line_color( index_t l, float r, float g, float b )
    {
        lines_[l]->set_mesh_color( r, g, b ) ;
    }
    /*!
     * Sets the line visibility to all the lines
     * @param[in] b the visibility
     */
    void GeoModelGfx::set_edge_lines_visibility( bool b )
    {
        for( index_t l = 0; l < lines_.size(); l++ ) {
            set_edge_line_visibility( l, b ) ;
        }
    }
    /*!
     * Sets the line visibility
     * @param[in] l the line index
     * @param[in] b the visibility
     */
    void GeoModelGfx::set_edge_line_visibility( index_t l, bool b )
    {
        lines_[l]->set_edges_visible( b ) ;
    }
    /*!
     * Sets the edge line size to all the lines
     * @param[in] s the size
     */
    void GeoModelGfx::set_edge_lines_size( index_t s )
    {
        for( index_t l = 0; l < lines_.size(); l++ ) {
            set_edge_line_size( l, s ) ;
        }
    }
    /*!
     * Sets the edge line size
     * @param[in] l the line index
     * @param[in] s the size
     */
    void GeoModelGfx::set_edge_line_size( index_t l, index_t s )
    {
        lines_[l]->set_mesh_width( s ) ;
    }
    /*!
     * Sets the vertex line color to all the lines
     * @param[in] r the red component of the color in [0.0, 1.0]
     * @param[in] g the green component of the color in [0.0, 1.0]
     * @param[in] b the blue component of the color in [0.0, 1.0]
     */
    void GeoModelGfx::set_vertex_lines_color( float r, float g, float b )
    {
        for( index_t k = 0; k < lines_.size(); k++ ) {
            set_vertex_line_color( k, r, g, b ) ;
        }
    }
    /*!
     * Sets the vertex line color
     * @param[in] l the line index
     * @param[in] r the red component of the color in [0.0, 1.0]
     * @param[in] g the green component of the color in [0.0, 1.0]
     * @param[in] b the blue component of the color in [0.0, 1.0]
     */
    void GeoModelGfx::set_vertex_line_color( index_t l, float r, float g, float b )
    {
        lines_[l]->set_points_color( r, g, b ) ;
    }
    /*!
     * Sets the vertex line visibility to all the lines
     * @param[in] b the visibility
     */
    void GeoModelGfx::set_vertex_lines_visibility( bool b )
    {
        for( index_t l = 0; l < lines_.size(); l++ ) {
            set_vertex_line_visibility( l, b ) ;
        }
    }
    /*!
     * Sets the vertex line visibility
     * @param[in] l the line index
     * @param[in] b the visibility
     */
    void GeoModelGfx::set_vertex_line_visibility( index_t l, bool b )
    {
        lines_[l]->set_vertices_visible( b ) ;
    }
    /*!
     * Sets the vertex line size to all the lines
     * @param[in] s the size
     */
    void GeoModelGfx::set_vertex_lines_size( index_t s )
    {
        for( index_t l = 0; l < lines_.size(); l++ ) {
            set_vertex_line_size( l, s ) ;
        }
    }
    /*!
     * Sets the vertex line size
     * @param[in] l the line index
     * @param[in] s the size
     */
    void GeoModelGfx::set_vertex_line_size( index_t l, index_t s )
    {
        lines_[l]->set_points_size( float( s ) ) ;
    }

    /*!
     * Draws the surfaces
     */
    void GeoModelGfx::draw_surfaces()
    {
        for( index_t s = 0; s < surfaces_.size(); s++ ) {
            if( surfaces_[s]->get_vertices_visible() )
                surfaces_[s]->draw_vertices() ;
            if( surfaces_[s]->get_surface_visible() ) surfaces_[s]->draw_surface() ;
        }
    }
    void GeoModelGfx::set_surfaces_lighting( bool value )
    {
        for( index_t s = 0; s < surfaces_.size(); s++ ) {
            surfaces_[s]->set_lighting( value ) ;
        }
    }
    /*!
     * Sets the surface color to all the surfaces
     * @param[in] r the red component of the color in [0.0, 1.0]
     * @param[in] g the green component of the color in [0.0, 1.0]
     * @param[in] b the blue component of the color in [0.0, 1.0]
     */
    void GeoModelGfx::set_surfaces_color( float r, float g, float b )
    {
        for( index_t s = 0; s < surfaces_.size(); s++ ) {
            set_surface_color( s, r, g, b ) ;
        }
    }
    /*!
     * Sets the surface color
     * @param[in] s the surface index
     * @param[in] r the red component of the color in [0.0, 1.0]
     * @param[in] g the green component of the color in [0.0, 1.0]
     * @param[in] b the blue component of the color in [0.0, 1.0]
     */
    void GeoModelGfx::set_surface_color( index_t s, float r, float g, float b )
    {
        surfaces_[s]->set_surface_color( r, g, b ) ;
    }
    /*!
     * Sets the backface surface color to all the surfaces
     * @param[in] r the red component of the color in [0.0, 1.0]
     * @param[in] g the green component of the color in [0.0, 1.0]
     * @param[in] b the blue component of the color in [0.0, 1.0]
     */
    void GeoModelGfx::set_backface_surfaces_color( float r, float g, float b )
    {
        for( index_t s = 0; s < surfaces_.size(); s++ ) {
            set_backface_surface_color( s, r, g, b ) ;
        }
    }
    /*!
     * Sets the backsurface surface color
     * @param[in] s the surface index
     * @param[in] r the red component of the color in [0.0, 1.0]
     * @param[in] g the green component of the color in [0.0, 1.0]
     * @param[in] b the blue component of the color in [0.0, 1.0]
     */
    void GeoModelGfx::set_backface_surface_color(
        index_t s,
        float r,
        float g,
        float b )
    {
        surfaces_[s]->set_backface_surface_color( r, g, b ) ;
    }
    /*!
     * Sets the surface visibility to all the surfaces
     * @param[in] b the visibility
     */
    void GeoModelGfx::set_surfaces_visibility( bool b )
    {
        for( index_t s = 0; s < surfaces_.size(); s++ ) {
            set_surface_visibility( s, b ) ;
        }
    }
    /*!
     * Sets the surface visibility
     * @param[in] s the surface index
     * @param[in] b the visibility
     */
    void GeoModelGfx::set_surface_visibility( index_t s, bool b )
    {
        surfaces_[s]->set_surface_visible( b ) ;
    }
    /*!
     * Sets the mesh surface color to all the surfaces
     * @param[in] r the red component of the color in [0.0, 1.0]
     * @param[in] g the green component of the color in [0.0, 1.0]
     * @param[in] b the blue component of the color in [0.0, 1.0]
     */
    void GeoModelGfx::set_mesh_surfaces_color( float r, float g, float b )
    {
        for( index_t s = 0; s < surfaces_.size(); s++ ) {
            set_mesh_surface_color( s, r, g, b ) ;
        }
    }
    /*!
     * Sets the mesh surface color
     * @param[in] s the surface index
     * @param[in] r the red component of the color in [0.0, 1.0]
     * @param[in] g the green component of the color in [0.0, 1.0]
     * @param[in] b the blue component of the color in [0.0, 1.0]
     */
    void GeoModelGfx::set_mesh_surface_color( index_t s, float r, float g, float b )
    {
        surfaces_[s]->set_mesh_color( r, g, b ) ;
    }
    /*!
     * Sets the mesh surface visibility to all the surfaces
     * @param[in] b the visibility
     */
    void GeoModelGfx::set_mesh_surfaces_visibility( bool b )
    {
        for( index_t s = 0; s < surfaces_.size(); s++ ) {
            set_mesh_surface_visibility( s, b ) ;
        }
    }
    /*!
     * Sets the mesh surface visibility
     * @param[in] s the surface index
     * @param[in] b the visibility
     */
    void GeoModelGfx::set_mesh_surface_visibility( index_t s, bool b )
    {
        surfaces_[s]->set_show_mesh( b ) ;
    }
    /*!
     * Sets the mesh surface size to all the surfaces
     * @param[in] size the size
     */
    void GeoModelGfx::set_mesh_surfaces_size( index_t size )
    {
        for( index_t s = 0; s < surfaces_.size(); s++ ) {
            set_mesh_surface_size( s, size ) ;
        }
    }
    /*!
     * Sets the mesh surface size
     * @param[in] s the surface index
     * @param[in] size the size
     */
    void GeoModelGfx::set_mesh_surface_size( index_t s, index_t size )
    {
        surfaces_[s]->set_mesh_width( size ) ;
    }
    /*!
     * Sets the vertex surface color to all the surfaces
     * @param[in] r the red component of the color in [0.0, 1.0]
     * @param[in] g the green component of the color in [0.0, 1.0]
     * @param[in] b the blue component of the color in [0.0, 1.0]
     */
    void GeoModelGfx::set_vertex_surfaces_color( float r, float g, float b )
    {
        for( index_t s = 0; s < surfaces_.size(); s++ ) {
            set_vertex_surface_color( s, r, g, b ) ;
        }
    }
    /*!
     * Sets the vertex surface color
     * @param[in] s the surface index
     * @param[in] r the red component of the color in [0.0, 1.0]
     * @param[in] g the green component of the color in [0.0, 1.0]
     * @param[in] b the blue component of the color in [0.0, 1.0]
     */
    void GeoModelGfx::set_vertex_surface_color(
        index_t s,
        float r,
        float g,
        float b )
    {
        surfaces_[s]->set_points_color( r, g, b ) ;
    }
    /*!
     * Sets the vertex surface visibility to all the surfaces
     * @param[in] b the visibility
     */
    void GeoModelGfx::set_vertex_surfaces_visibility( bool b )
    {
        for( index_t s = 0; s < surfaces_.size(); s++ ) {
            set_vertex_surface_visibility( s, b ) ;
        }
    }
    /*!
     * Sets the vertex surface visibility
     * @param[in] s the surface index
     * @param[in] b the visibility
     */
    void GeoModelGfx::set_vertex_surface_visibility( index_t s, bool b )
    {
        surfaces_[s]->set_vertices_visible( b ) ;
    }
    /*!
     * Sets the vertex surface size to all the surfaces
     * @param[in] size the size
     */
    void GeoModelGfx::set_vertex_surfaces_size( index_t size )
    {
        for( index_t s = 0; s < surfaces_.size(); s++ ) {
            set_vertex_surface_size( s, size ) ;
        }
    }
    /*!
     * Sets the vertex surface size
     * @param[in] s the surface index
     * @param[in] size the size
     */
    void GeoModelGfx::set_vertex_surface_size( index_t s, index_t size )
    {
        surfaces_[s]->set_points_size( float( size ) ) ;
    }

    /*!
     * Draws the MacroMesh
     */
    void GeoModelGfx::draw_regions()
    {
        for( index_t m = 0; m < regions_.size(); m++ ) {
            if( regions_[m]->get_vertices_visible() ) regions_[m]->draw_vertices() ;
            if( regions_[m]->get_edges_visible() ) regions_[m]->draw_edges() ;
            if( regions_[m]->get_surface_visible() ) regions_[m]->draw_surface() ;
            if( regions_[m]->get_region_visible() ) regions_[m]->draw_volume() ;
        }
    }

    /*!
     * Sets the vertex region color to all the regions
     * @param[in] r the red component of the color in [0.0, 1.0]
     * @param[in] g the green component of the color in [0.0, 1.0]
     * @param[in] b the blue component of the color in [0.0, 1.0]
     */
    void GeoModelGfx::set_vertex_regions_color( float r, float g, float b )
    {
        for( index_t m = 0; m < regions_.size(); m++ ) {
            set_vertex_region_color( m, r, g, b ) ;
        }
    }

    /*!
     * Sets the vertex region color
     * @param[in] m the region index
     * @param[in] r the red component of the color in [0.0, 1.0]
     * @param[in] g the green component of the color in [0.0, 1.0]
     * @param[in] b the blue component of the color in [0.0, 1.0]
     */
    void GeoModelGfx::set_vertex_region_color( index_t m, float r, float g, float b )
    {
        ringmesh_assert( m < regions_.size() ) ;
        regions_[m]->set_points_color( r, g, b ) ;
    }

    /*!
     * Sets the vertex region visibility to all the regions
     * @param[in] b the visibility
     */
    void GeoModelGfx::set_vertex_regions_visibility( bool b )
    {
        for( index_t m = 0; m < regions_.size(); m++ ) {
            set_vertex_region_visibility( m, b ) ;
        }
    }

    /*!
     * Sets the vertex region visibility
     * @param[in] m the region index
     * @param[in] b the visibility
     */
    void GeoModelGfx::set_vertex_region_visibility( index_t m, bool b )
    {
        ringmesh_assert( m < regions_.size() ) ;
        regions_[m]->set_vertices_visible( b ) ;
    }

    /*!
     * Sets the vertex region size to all the regions
     * @param[in] s the size
     */
    void GeoModelGfx::set_vertex_regions_size( index_t s )
    {
        for( index_t m = 0; m < regions_.size(); m++ ) {
            set_vertex_region_size( m, s ) ;
        }
    }

    /*!
     * Sets the vertex region size
     * @param[in] m the region index
     * @param[in] s the size
     */
    void GeoModelGfx::set_vertex_region_size( index_t m, index_t s )
    {
        ringmesh_assert( m < regions_.size() ) ;
        regions_[m]->set_points_size( float( s ) ) ;
    }

    /*!
     * Sets the edge color to all the meshes
     * @param[in] r the red component of the color in [0.0, 1.0]
     * @param[in] g the green component of the color in [0.0, 1.0]
     * @param[in] b the blue component of the color in [0.0, 1.0]
     */
    void GeoModelGfx::set_edge_regions_color( float r, float g, float b )
    {
        for( index_t k = 0; k < regions_.size(); k++ ) {
            set_edge_region_color( k, r, g, b ) ;
        }
    }
    /*!
     * Sets the edge color
     * @param[in] m the mesh index
     * @param[in] r the red component of the color in [0.0, 1.0]
     * @param[in] g the green component of the color in [0.0, 1.0]
     * @param[in] b the blue component of the color in [0.0, 1.0]
     */
    void GeoModelGfx::set_edge_region_color( index_t m, float r, float g, float b )
    {
        ringmesh_assert( m < regions_.size() ) ;
        regions_[m]->set_mesh_color( r, g, b ) ; //TODO function not good?
    }
    /*!
     * Sets the edge visibility to all the meshes
     * @param[in] b the visibility
     */
    void GeoModelGfx::set_edge_regions_visibility( bool b )
    {
        for( index_t m = 0; m < regions_.size(); m++ ) {
            set_edge_region_visibility( m, b ) ;
        }
    }
    /*!
     * Sets the edge visibility
     * @param[in] m the mesh index
     * @param[in] b the visibility
     */
    void GeoModelGfx::set_edge_region_visibility( index_t m, bool b )
    {
        ringmesh_assert( m < regions_.size() ) ;
        regions_[m]->set_edges_visible( b ) ;
    }
    /*!
     * Sets the edge line size to all the lines
     * @param[in] s the size
     */
    void GeoModelGfx::set_edge_regions_size( index_t s )
    {
        for( index_t m = 0; m < regions_.size(); m++ ) {
            set_edge_region_size( m, s ) ;
        }
    }
    /*!
     * Sets the edge line size
     * @param[in] l the line index
     * @param[in] s the size
     */
    void GeoModelGfx::set_edge_region_size( index_t l, index_t s )
    {
        ringmesh_assert( l < regions_.size() ) ;
        regions_[l]->set_mesh_width( s ) ;
    }


    void GeoModelGfx::set_cell_regions_lighting( bool value )
    {
        for( index_t r = 0; r < regions_.size(); r++ ) {
            regions_[r]->set_lighting( value ) ;
        }
    }

    /*!
     * Sets the mesh region color to all the regions
     * @param[in] r the red component of the color in [0.0, 1.0]
     * @param[in] g the green component of the color in [0.0, 1.0]
     * @param[in] b the blue component of the color in [0.0, 1.0]
     */
    void GeoModelGfx::set_cell_mesh_regions_color( float r, float g, float b )
    {
        for( index_t m = 0; m < regions_.size(); m++ ) {
            set_cell_mesh_region_color( m, r, g, b ) ;
        }
    }

    /*!
     * Sets the mesh region color
     * @param[in] m the region index
     * @param[in] r the red component of the color in [0.0, 1.0]
     * @param[in] g the green component of the color in [0.0, 1.0]
     * @param[in] b the blue component of the color in [0.0, 1.0]
     */
    void GeoModelGfx::set_cell_mesh_region_color(
        index_t m,
        float r,
        float g,
        float b )
    {
        ringmesh_assert( m < regions_.size() ) ;
        regions_[m]->set_mesh_color( r, g, b ) ;
    }

    /*!
     * Toggles the cell region color per cell type to all the regions
     */
    void GeoModelGfx::set_cell_regions_color_type()
    {
        for( index_t m = 0; m < regions_.size(); m++ ) {
            set_cell_region_color_type( m ) ;
        }
    }

    /*!
     * Toggles the cell region color per cell type
     * @param[in] m the region index
     */
    void GeoModelGfx::set_cell_region_color_type( index_t m )
    {
        ringmesh_assert( m < regions_.size() ) ;
        regions_[m]->set_cells_colors_by_type() ;
    }

    /*!
     * Sets the mesh region visibility to all the regions
     * @param[in] b the visibility
     */
    void GeoModelGfx::set_cell_mesh_regions_visibility( bool b )
    {
        for( index_t m = 0; m < regions_.size(); m++ ) {
            set_cell_mesh_region_visibility( m, b ) ;
        }
    }

    /*!
     * Sets the mesh region visibility
     * @param[in] m the region index
     * @param[in] b the visibility
     */
    void GeoModelGfx::set_cell_mesh_region_visibility( index_t m, bool b )
    {
        ringmesh_assert( m < regions_.size() ) ;
        regions_[m]->set_show_mesh( b ) ;
    }

    /*!
     * Sets the mesh region size to all the regions
     * @param[in] s the size
     */
    void GeoModelGfx::set_cell_mesh_regions_size( index_t s )
    {
        for( index_t m = 0; m < regions_.size(); m++ ) {
            set_cell_mesh_region_size( m, s ) ;
        }
    }

    /*!
     * Sets the mesh region size
     * @param[in] m the region index
     * @param[in] s the size
     */
    void GeoModelGfx::set_cell_mesh_region_size( index_t m, index_t s )
    {
        ringmesh_assert( m < regions_.size() ) ;
        regions_[m]->set_mesh_width( s ) ;
    }

    /*!
     * Sets the cell region color to all the regions
     * @param[in] r the red component of the color in [0.0, 1.0]
     * @param[in] g the green component of the color in [0.0, 1.0]
     * @param[in] b the blue component of the color in [0.0, 1.0]
     */
    void GeoModelGfx::set_cell_regions_color( float r, float g, float b )
    {
        for( index_t m = 0; m < regions_.size(); m++ ) {
            set_cell_region_color( m, r, g, b ) ;
        }
    }

    /*!
     * Sets the cell region color
     * @param[in] m the region index
     * @param[in] r the red component of the color in [0.0, 1.0]
     * @param[in] g the green component of the color in [0.0, 1.0]
     * @param[in] b the blue component of the color in [0.0, 1.0]
     */
    void GeoModelGfx::set_cell_region_color( index_t m, float r, float g, float b )
    {
        ringmesh_assert( m < regions_.size() ) ;
        regions_[m]->set_cells_color( r, g, b ) ;
    }

    /*!
     * Sets the cell region visibility to all the regions
     * @param[in] b the visibility
     */
    void GeoModelGfx::set_cell_regions_visibility( bool b )
    {
        for( index_t m = 0; m < regions_.size(); m++ ) {
            set_cell_region_visibility( m, b ) ;
        }
    }

    /*!
     * Sets the cell region visibility to all the regions
     * @param[in] m the region index
     * @param[in] b the visibility
     */
    void GeoModelGfx::set_cell_region_visibility( index_t m, bool b )
    {
        ringmesh_assert( m < regions_.size() ) ;
        regions_[m]->set_region_visible( b ) ;
    }
    void GeoModelGfx::set_cell_regions_type_visibility( GEO::MeshCellType t, bool b )
    {
        for( index_t m = 0; m < regions_.size(); m++ ) {
            set_cell_region_type_visibility( m, t, b ) ;
        }
    }
    void GeoModelGfx::set_cell_region_type_visibility(
        index_t m,
        GEO::MeshCellType t,
        bool b )
    {
        ringmesh_assert( m < regions_.size() ) ;
        regions_[m]->set_draw_cells( t, b ) ;
    }

    /*!
     * Sets the cell region shrink to all the regions
     * @param[in] s the shrink
     */
    void GeoModelGfx::set_cell_regions_shrink( double s )
    {
        for( index_t m = 0; m < regions_.size(); m++ ) {
            set_cell_region_shrink( m, s ) ;
        }
    }

    /*!
     * Sets the cell region shrink
     * @param[in] m the region index
     * @param[in] s the shrink
     */
    void GeoModelGfx::set_cell_region_shrink( index_t m, double s )
    {
        ringmesh_assert( m < regions_.size() ) ;
        regions_[m]->set_shrink( s ) ;
    }

}

#endif
