/*
 *  Copyright (c) 2012-2014, Bruno Levy
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *  this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *  this list of conditions and the following disclaimer in the documentation
 *  and/or other materials provided with the distribution.
 *  * Neither the name of the ALICE Project-Team nor the names of its
 *  contributors may be used to endorse or promote products derived from this
 *  software without specific prior written permission.
 * 
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 *  If you modify this software, you should include a notice giving the
 *  name of the person performing the modification, the date of modification,
 *  and the reason for such modification.
 *
 *  Contact: Bruno Levy
 *
 *     Bruno.Levy@inria.fr
 *     http://www.loria.fr/~levy
 *
 *     ALICE Project
 *     LORIA, INRIA Lorraine, 
 *     Campus Scientifique, BP 239
 *     54506 VANDOEUVRE LES NANCY CEDEX 
 *     FRANCE
 *
 */

#ifndef __GEOGRAM_GFX_MESH_GFX__
#define __GEOGRAM_GFX_MESH_GFX__

#include <geogram_gfx/basic/common.h>
#include <geogram_gfx/GLUP/GLUP.h>
#include <geogram/mesh/mesh.h>

/**
 * \file geogram_gfx/mesh/mesh_gfx.h
 * \brief A class to display a mesh using OpenGL/GLUP.
 */

namespace GEO {

    class MeshGfxImpl;
    
    /**
     * \brief Draws a mesh using OpenGL.
     */
    class GEOGRAM_GFX_API MeshGfx {
    public:

        /**
         * \brief MeshGfx constructor.
         */
        MeshGfx();

        /**
         * \brief MeshGfx destructor.
         */
        ~MeshGfx();

        /**
         * \brief Draws the vertices of the mesh.
         * \details If a vertices selection is set, only the
         *  vertices in the selection are drawn
         * \see set_vertices_selection()
         */
        void draw_vertices();

        /**
         * \brief Draws the edges of the mesh.
         */
        void draw_edges();
        
        /**
         * \brief Draws the surfacic part of the mesh.
         */
        void draw_surface();

        /**
         * \brief Draws the borders of the surfacic 
         *  part of the mesh.
         */
        void draw_surface_borders();
        
        /**
         * \brief Draws the volumetric part of the mesh.
         */
        void draw_volume();
        
        /**
         * \brief Gets the mesh visibility flag.
         * \details The mesh visibility flags specifies
         *  whether mesh edges should be drawn. The used
         *  color can be specified by set_mesh_color()
         * \retval true if mesh edges should be displayed
         * \retval false otherwise
         */
        bool get_show_mesh() const {
            return show_mesh_;
        }

        /**
         * \brief Sets the mesh visibility flag.
         * \param[in] x the new value of the mesh visibility flag.
         * \details The mesh visibility flags specifies
         *  whether mesh edges should be drawn. The used
         *  color can be specified by set_mesh_color()
         * \note For now, regions are only implemented for
         *   triangulated meshes and tetrahedralized meshes
         *   (not implemented yet for hybrid surfacic and 
         *    volumetric meshes).
         */
        void set_show_mesh(bool x) {
            show_mesh_ = x;
        }

        /**
         * \brief Gets the mesh width
         * \details The mesh width is taken into account 
         *   when the mesh visibility flag is set 
         *   (by set_show_mesh()), when drawing facets
         *   and cells.
         * \return the mesh width
         */
        index_t get_mesh_width() const {
            return mesh_width_;
        }

        /**
         * \brief Sets the mesh width
         * \details The mesh width is taken into account 
         *   when the mesh visibility flag is set 
         *   (by set_show_mesh()), when drawing facets
         *   and cells.
         * \param[in] x the mesh width (minimum is 1)
         */
        void set_mesh_width(index_t x) {
            mesh_width_ = x;
        }

        /**
         * \brief Gets the mesh border width
         * \details The mesh border width is the one used
         *   by draw_surface_borders()
         * \return the mesh border width
         */
        index_t get_mesh_border_width() const {
            return mesh_border_width_;
        }

        /**
         * \brief Sets the mesh border width
         * \details The mesh border width is the one used
         *   by draw_surface_borders()
         * \param[in] x the mesh width (minimum is 1)
         */
        void set_mesh_border_width(index_t x) {
            mesh_border_width_ = x;
        }
        
        /**
         * \brief Gets the cells shrink coefficient.
         * \details The cells shrink coefficient is used 
         *  to display cells slighly smaller than what they are.
         *  Cells shrinking is only supported in GLSL mode.
         * \return the cells shrink coefficient, betwe 0.0 (no shrink) 
         *  and 1.0 (full shrink)
         */
        double get_shrink() const {
            return shrink_;
        }

        /**
         * \brief Sets the cells shrink coefficient.
         * \details The cells shrink coefficient is used 
         *  to display cells slighly smaller than what they are.
         *  Cells shrinking is only supported in GLSL mode.
         * \param[in] x the cells shrink coefficient, betwe 0.0 (no shrink) 
         *  and 1.0 (full shrink)
         */
        void set_shrink(double x) {
            shrink_ = x;
            if(shrink_ < 0.0) {
                shrink_ = 0.0;
            } else if(shrink_ > 1.0) {
                shrink_ = 1.0;
            }
        }


        /**
         * \brief Gets the animate flag
         * \details When animate mode is activated and the
         *  mesh has 6d vertices, then an animation is displayed.
         *  The first three coordinates correspond to the vertex
         *  position at initial time (t=0). The last three coordinates 
         *  correspond to the vertex position at final time (t=1).
         * \retval true if animation is used
         * \retval false otherwise
         * \see get_time(), set_time()
         */
        bool get_animate() const {
            return animate_;
        }

        /**
         * \brief Gets the animate flag
         * \details When animate mode is activated and the
         *  mesh has 6d vertices, then an animation is displayed.
         *  The first three coordinates correspond to the vertex
         *  position at initial time (t=0). The last three coordinates 
         *  correspond to the vertex position at final time (t=1).
         * \param[in] x true if animation should be used, false otherwise
         * \see get_time(), set_time()
         */
        void set_animate(bool x) {
            animate_ = x;
        }
        
        /**
         * \brief Gets the time of the animation.
         * \details Used if animate mode is set.
         * \return the time of the animation, betwe 0.0 (initial) 
         *  and 1.0 (final)
         * \see get_animate(), set_animate()
         */
        double get_time() const {
            return time_;
        }

        /**
         * \brief Gets the time of the animation.
         * \details Used if animate mode is set.
         * \param[in] x the time of the animation, betwe 0.0 (initial) 
         *  and 1.0 (final)
         * \see get_animate(), set_animate()
         */
        void set_time(double x) {
            time_ = x;
            if(time_ < 0.0) {
                time_ = 0.0;
            } else if(time_ > 1.0) {
                time_ = 1.0;
            }
        }
        
        /**
         * \brief Gets the cell visibility flag.
         * \details It is possible to specify cell visibility
         *  flags for each individual cell type.
         * \param[in] type one of MESH_TET, MESH_HEX, MESH_PRISM, MESH_PYRAMID
         * \retval true if the cells of \p type should be displayed
         * \retval false otherwise
         */
        bool get_draw_cells(MeshCellType type) const {
            return draw_cells_[type];
        }


        /**
         * \brief Sets the cell visibility flag.
         * \details It is possible to specify cell visibility
         *  flags for each individual cell type.
         * \param[in] type one of MESH_TET, MESH_HEX, MESH_PRISM, MESH_PYRAMID
         * \param[in] x true if mesh cells of type \p type should be displayed,
         *  false otherwise.
         */
        void set_draw_cells(MeshCellType type, bool x) {
            draw_cells_[type] = x;
        }

        /**
         * \brief Sets the points color
         * \details Specifies the color used to display points
         * \param[in] r,g,b the components of the points color,
         *  in (0.0 .. 1.0)
         * \see draw_points()
         */
        void set_points_color(float r, float g, float b) {
            points_color_[0] = r;
            points_color_[1] = g;
            points_color_[2] = b;
        }

        /**
         * \brief Gets the points color
         * \param[out] r,g,b the components of the points color,
         *  in (0.0 .. 1.0)
         * \see draw_points()
         */
        void get_points_color(float& r, float& g, float& b) const {
            r = points_color_[0];
            g = points_color_[1];
            b = points_color_[2];
        }

        /**
         * \brief Sets the point size
         * \param[in] x the point size (minimum 1)
         * \see draw_points()
         */
        void set_points_size(float x) {
            points_size_ = x;
        }

        /**
         * \brief Gets the point size
         * \return the point size
         * \see draw_points()
         */
        float get_points_size() const {
            return points_size_;
        }
        
        /**
         * \brief Sets the mesh color
         * \details Specifies the mesh color to be used if 
         *  mesh edges should be displayed.
         * \param[in] r,g,b the components of the mesh color,
         *  in (0.0 .. 1.0)
         * \see set_show_mesh(), draw_surface(), draw_volume()
         */
        void set_mesh_color(float r, float g, float b) {
            mesh_color_[0] = r;
            mesh_color_[1] = g;
            mesh_color_[2] = b;
        }

        /**
         * \brief Gets the mesh color
         * \param[out] r,g,b the components of the mesh color,
         *  in (0.0 .. 1.0)
         * \see set_show_mesh(), draw_surface(), draw_volume()
         */
        void get_mesh_color(float& r, float& g, float& b) const {
            r = mesh_color_[0];
            g = mesh_color_[1];
            b = mesh_color_[2];
        }
        
        /**
         * \brief Sets the surface color
         * \details Specifies the color used to display the
         *  surfacic part of the mesh. It specifies the color 
         *  of both frontfacing and backfacing faces.
         * \param[in] r,g,b the components of the surface color,
         *  in (0.0 .. 1.0)
         * \see draw_surface(), set_backface_surface_color()
         */
        void set_surface_color(float r, float g, float b) {
            surface_color_[0] = r;
            surface_color_[1] = g;
            surface_color_[2] = b;
            backface_surface_color_[0] = r;
            backface_surface_color_[1] = g;
            backface_surface_color_[2] = b;
        }

        /**
         * \brief Gets the surface color
         * \param[out] r,g,b the components of the surface color,
         *  in (0.0 .. 1.0)
         * \see draw_surface()
         */
        void get_surface_color(float& r, float& g, float& b) const {
            r = surface_color_[0];
            g = surface_color_[1];
            b = surface_color_[2];
        }
        
        /**
         * \brief Sets the surface color for backfacing faces.
         * \details Specifies the color used to display the
         *  backfaces of the surfacic part of the mesh. 
         * \param[in] r,g,b the components of the surface color,
         *  in (0.0 .. 1.0)
         * \see set_show_mesh(), draw_surface(), draw_volume()
         */
        void set_backface_surface_color(float r, float g, float b) {
            backface_surface_color_[0] = r;
            backface_surface_color_[1] = g;
            backface_surface_color_[2] = b;
        }

        /**
         * \brief Sets the color used to display mesh cells.
         * \param[in] r,g,b the components of the cells color,
         *  in (0.0 .. 1.0)
         * \see set_cells_colors_by_type(), draw_volume()
         */
        void set_cells_color(float r, float g, float b) {
            for(index_t i=0; i<MESH_NB_CELL_TYPES; ++i) {
                cells_color_[i][0] = r;
                cells_color_[i][1] = g;
                cells_color_[i][2] = b;
            }
        }

        /**
         * \brief Gets the cells color
         * \param[out] r,g,b the components of the cells color,
         *  in (0.0 .. 1.0)
         * \see set_cells_colors_by_type(), draw_volume()
         */
        void get_cells_color(float& r, float& g, float& b) const {
            r = cells_color_[0][0];
            g = cells_color_[0][1];
            b = cells_color_[0][2];
        }

        /**
         * \brief Sets a different color for each mesh cell type
         * \details it uses the following colors:
         *  - tets: red
         *  - hexes: white
         *  - prisms: green
         *  - pyramids: blue
         * \see set_cells_color(), draw_volume()
         */
        void set_cells_colors_by_type() {
            cells_colors_by_type_ = true;
            set_cells_color(MESH_TET,       1.0f, 0.0f, 0.0f);
            set_cells_color(MESH_HEX,       0.9f, 0.9f, 0.9f);
            set_cells_color(MESH_PRISM,     0.0f, 1.0f, 0.0f);
            set_cells_color(MESH_PYRAMID,   0.0f, 0.0f, 1.0f);
            set_cells_color(MESH_CONNECTOR, 1.0f, 0.8f, 0.0f);            
        }
        
        /**
         * \brief Gets the lighing flag
         * \retval true if lighting should be used
         * \retval false otherwise
         */
        bool get_lighting() const {
            return lighting_;
        }

        /**
         * \brief Sets the lighting flag
         * \param[in] x true if lighting should be used, false
         *  otherwise.
         */
        void set_lighting(bool x) {
            lighting_ = x;
        }

        /**
         * \brief Sets the mesh
         * \param[in] M a pointer to the mesh that should be
         *  displayed.
         */
        void set_mesh(const Mesh* M);
        
        /**
         * \brief Gets the mesh
         * \return a pointer to the mesh that will be displayed.
         */
        const Mesh* mesh() const {
            return mesh_;
        }

        /**
         * \brief Sets picking mode.
         * \details If picking mode is MESH_NONE, then normal drawing
         *  is activated, else the color is replaced with the index of
         *  the elements.
         * \param[in] what a bitwise or ('|') combination of 
         *  MESH_VERTICES, MESH_EDGES, MESH_FACETS, MESH_CELLS,
         *  or MESH_NONE if picking mode should be deactivated
         * \note Picking mode is currently only implemented with
         *  GLSL support at least v1.5, and only works for points,
         *  segments, pure triangle meshes and pure tetrahedral meshes
         *  (facet picking for polygonal meshes and cell picking for 
         *   hybrid meshes are not implemented yet).
         */
        void set_picking_mode(MeshElementsFlags what) {
            picking_mode_ = what;
        }

        /**
         * \brief Gets the current picking mode.
         * \return a bitwise or ('|') combination of 
         *  MESH_VERTICES, MESH_EDGES, MESH_FACETS, MESH_CELLS,
         *  or MESH_NONE if picking mode is deactivated
         */
        MeshElementsFlags get_picking_mode() const {
            return picking_mode_;
        }
        

        /**
         * \brief Sets the object-wide picking id.
         * \details When the object-wide picking id is set,
         *  it is used to draw all primitives in the picking buffer.
         * \param[in] id the object-wide picking id, or index_t(-1) if
         *  normal per-primitive picking should be used.
         */
        void set_object_picking_id(index_t id) {
            object_picking_id_ = id;
        }

        /**
         * \brief Gets the object-wide picking id.
         * \return the object-wide picking id, or index_t(-1) if
         *  normal per-primitive picking should be used.
         */
        index_t get_object_picking_id() const {
            return object_picking_id_;
        }

        /**
         * \brief Sets the vertices selection.
         * \details If set, the vertices selection is used to determine
         *  which vertices should be drawn by draw_vertices().
         * \param[in] name the name of a Property<bool> bound to
         *  the vertices of the mesh.
         * \see draw_vertices()
         */
        void set_vertices_selection(const std::string& name) {
            vertices_selection_ = name;
        }

        /**
         * \brief Gets the vertices selection.
         * \return the name of the vertices selection, or an
         *  empty string if no vertices selection is set.
         */
        const std::string& get_vertices_selection() const {
            return vertices_selection_;
        }

        /**
         * \brief Sets the parameters for displaying a
         *  scalar attribute using texture mapping.
         * \param[in] subelement one of MESH_VERTICES, MESH_FACETS,
         *  MESH_FACET_CORNERS, MESH_CELLS, MESH_CELL_CORNERS,
         *  MESH_CELL_FACETS
         * \param[in] name name of the attribute with an optional index,
         *   for instance, "foobar[5]" refers to the 5th coordinate of
         *   the "foobar" vector attribute.
         * \param[in] attr_min value of the attribute that is bound to 
         *   the leftmost color in the colormap
         * \param[in] attr_max value of the attribute that is bound to
         *   the rightmost color in the colormap
         * \param[in] colormap_texture the texture to be used to display
         *   the attribute colors
         */
        void set_scalar_attribute(
            MeshElementsFlags subelements,
            const std::string& name,
            double attr_min, double attr_max,
            GLuint colormap_texture
        );

        /**
         * \brief Unsets scalar attribute display.
         */
        void unset_scalar_attribute() {
            attribute_subelements_ = MESH_NONE;
            attribute_min_ = 0.0;
            attribute_max_ = 0.0;
            attribute_colormap_texture_ = 0;
        }
        
    protected:

        void set_cells_color(MeshCellType type, float r, float g, float b) {
            cells_color_[type][0] = r;
            cells_color_[type][1] = g;
            cells_color_[type][2] = b;            
        }

        void draw_vertex_with_attribute(index_t vertex) {
            if(
                picking_mode_ == MESH_NONE &&
                attribute_subelements_ == MESH_VERTICES
            ) {
                glupTexCoord1d(attribute_[vertex]);
            }
            draw_vertex(vertex);
        }

        void draw_surface_vertex_with_attribute(
            index_t vertex, index_t facet, index_t corner
        ) {
            if(picking_mode_ == MESH_NONE) {
                switch(attribute_subelements_) {
                case MESH_VERTICES:
                    glupTexCoord1d(attribute_[vertex]);
                    break;
                case MESH_FACETS:
                    glupTexCoord1d(attribute_[facet]);
                    break;
                case MESH_FACET_CORNERS:
                    glupTexCoord1d(attribute_[corner]);                
                    break;
                default:
                    break;
                }
            }
            draw_vertex(vertex);
        }

        void draw_volume_vertex_with_attribute(
            index_t vertex, index_t cell, index_t cell_corner
        ) {
            if(picking_mode_ == MESH_NONE) {
                switch(attribute_subelements_) {
                case MESH_VERTICES:
                    glupTexCoord1d(attribute_[vertex]);
                    break;
                case MESH_CELLS:
                    glupTexCoord1d(attribute_[cell]);
                    break;
                case MESH_CELL_CORNERS:
                    glupTexCoord1d(attribute_[cell_corner]);
                    break;
                default:
                    break;
                }
            }
            draw_vertex(vertex);
        }

        void draw_vertex(index_t v) {
            if(do_animation_) {
                if(mesh_->vertices.single_precision()) {
                    const GLUPfloat* p =
                        mesh_->vertices.single_precision_point_ptr(v);
                    float t = float(time_);
                    float s = 1.0f - float(time_);
                    glupVertex3f(
                        s*p[0] + t*p[3],
                        s*p[1] + t*p[4],
                        s*p[2] + t*p[5]
                    );
                } else {
                    const GLUPdouble* p = mesh_->vertices.point_ptr(v);
                    double s = 1.0 - time_;
                    glupVertex3d(
                        s*p[0] + time_*p[3],
                        s*p[1] + time_*p[4],
                        s*p[2] + time_*p[5]
                    );
                }
            } else {
                if(mesh_->vertices.single_precision()) {
                    glupVertex3fv(
                        mesh_->vertices.single_precision_point_ptr(v)
                    );
                } else {
                    glupVertex3dv(
                        mesh_->vertices.point_ptr(v)
                    );
                }
            }
        }

        void draw_surface_mesh_with_lines();
        
        /**
         * \brief Sets GLUP drawing parameters.
         */
        void set_GLUP_parameters();

        /**
         * \brief Sets GLUP picking mode for drawing primitives
         *  of a given type, or deactivates GLUP picking if MeshGfx picking mode
         *  is deactivated.
         * \param[in] one of MESH_VERTICES, MESH_EDGES, MESH_FACETS, MESH_CELLS.
         */
        void set_GLUP_picking(MeshElementsFlags what);


        /**
         * \brief Encodes an id as the current vertex color.
         * \details This is required for drawing polygons, that
         *  cannot use standard GLUP primitives and picking.
         */
        void set_GLUP_vertex_color_from_picking_id(index_t id);


        /**
         * \brief Updates the Vertex Buffer Objects and Vertex Array
         *  Objects.
         * \details The buffer objects are updated if buffer_objects_dirty_
         *  is set, then buffer_objects_dirty_ is reset. If 
         *  buffer_objects_dirty_ is not set, it checks whether the sizes
         *  of the buffer objects match the size of the mesh arrays.
         */
        void update_buffer_objects_if_needed();


        /**
         * \brief Updates the buffer objects used to display attributes.
         * \details The buffer objects are updated if 
         *  attribute_buffer_objects_dirty_
         *  is set, then attribute_buffer_objects_dirty_ is reset. If 
         *  attribute_buffer_objects_dirty_ is not set, it checks whether 
         *  the sizes of the buffer objects match the size of the mesh arrays.
         */
        void update_attribute_buffer_objects_if_needed();
        
        /**
         * \brief Binds the vertices VBO to the current VAO.
         */
        void bind_vertices_VBO();

        /**
         * \brief Setups drawing for attributes.
         * \details If no attribute is bound, does nothing.
         */
        void begin_attributes();

        /**
         * \brief Deactivates drawing for attributes.
         */
        void end_attributes();


        /**
         * \brief Tests whether array mode can be used
         *  to draw a specified GLUP primitive.
         * \param[in] prim the GLUP primitive
         */
        bool can_use_array_mode(GLUPprimitive prim) const;
        
        /**
         * \brief Forbids MeshGfx copy..
         */
        MeshGfx(const MeshGfx& rhs);
        
        /**
         * \brief Forbids MeshGfx copy..
         */
        MeshGfx& operator=(const MeshGfx& rhs);


    protected:
        bool show_mesh_;
        index_t mesh_width_;
        index_t mesh_border_width_;
        double shrink_;
        bool animate_;
        double time_;
        bool draw_cells_[MESH_NB_CELL_TYPES];
        float points_size_;
        
        float points_color_[3];
        float mesh_color_[3];
        float surface_color_[3];
        float backface_surface_color_[3];
        float cells_color_[MESH_NB_CELL_TYPES][3];
        bool cells_colors_by_type_;

        bool lighting_;

        MeshElementsFlags picking_mode_;
        index_t object_picking_id_;

        std::string vertices_selection_;

        bool do_animation_;
        
        const Mesh* mesh_;
        bool triangles_and_quads_;

        bool buffer_objects_dirty_;
        bool attributes_buffer_objects_dirty_;

        GLuint vertices_VAO_;
        GLuint edges_VAO_;
        GLuint facets_VAO_;
        GLuint cells_VAO_;
        
        GLuint vertices_VBO_;
        GLuint edge_indices_VBO_;
        GLuint facet_indices_VBO_;
        GLuint cell_indices_VBO_;
        GLuint vertices_attribute_VBO_;
        
        MeshElementsFlags attribute_subelements_;
        std::string attribute_name_;
        double attribute_min_;
        double attribute_max_;
        GLuint attribute_colormap_texture_;
        ReadOnlyScalarAttributeAdapter attribute_;
    };

}

#endif
