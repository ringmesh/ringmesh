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

#include <geogram_gfx/third_party/glew/glew.h>
#include <geogram_gfx/glut_viewer/glut_viewer.h>
#include <geogram/mesh/mesh.h>
#include <geogram/mesh/mesh_io.h>
#include <geogram/mesh/mesh_geometry.h>
#include <geogram/mesh/mesh_private.h>
#include <geogram/basic/command_line.h>
#include <geogram/basic/command_line_args.h>
#include <geogram/basic/file_system.h>

#include <stdarg.h>

namespace {

    GEO::SinglePrecisionMesh M;

    // Vertex buffer objects
    GLuint vertices_VBO = 0;
    GLuint facet_indices_VBO = 0;
    GLuint cell_indices_VBO = 0;

    GLboolean show_mesh = GL_TRUE;
    GLboolean show_colors = GL_TRUE;
    GLboolean show_borders = GL_FALSE;
    GLboolean white_bg = GL_TRUE;
    GLboolean show_surface = GL_TRUE;
    GLboolean show_volume = GL_FALSE;
    GLboolean show_vertices = GL_FALSE;
    GLboolean lighting = GL_TRUE;
    GLfloat   shrink = 0.0;
    GLboolean colored_cells = GL_FALSE;
    
    /** \brief true if shaders are deactivated. */
    bool plain_mode = false;

    /** \brief true if surface mesh has only triangles and quads. */
    bool triangles_and_quads = false;

    /** \brief true if displaying optimal transport results. */
    bool OTM_mode = false;

    /** \brief time for the animation display of optimal transport results. */
    float OTM_time = 0.0;
    
    /** 
     * \brief true if tessellation shaders are supported 
     *  (OpenGL/GLSL ver. >= 4.0). 
     */
    bool has_tessellation_shaders = false;
    
    /**
     * \brief Symbolic constants referring to a GPU program in the
     *  array programs[].
     */
    enum {
        PRG_POINTS        =0,
        PRG_TRI           =1,
        PRG_QUAD          =2,
        PRG_TET           =3,
        PRG_HEX           =4,
        PRG_PRISM         =5,
        PRG_PYRAMID       =6,
        PRG_NB            =7
    } ;

    /**
     * \brief GPU programs are grouped in an array, so that
     *  setting the same uniform variable in all of them can
     *  be done easily with a for() loop.
     */
    GLuint programs[PRG_NB] = {
        0, 0, 0, 0, 0, 0, 0
    } ;

    /**
     * \brief Default color to be used with a given program.
     * \details The color should be passed by:
     *  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, colors[PRG]);
     */
    GLfloat colors[PRG_NB][4] =
    {
        {0.9f, 0.9f, 0.9f, 1.0f},
        {0.9f, 0.9f, 0.9f, 1.0f},
        {0.9f, 0.9f, 0.9f, 1.0f},
        {0.9f, 0.9f, 0.9f, 1.0f},
        {0.9f, 0.9f, 0.9f, 1.0f},
        {0.9f, 0.9f, 0.9f, 1.0f},
        {0.9f, 0.9f, 0.9f, 1.0f}        
    } ;

    /**
     * \brief Defines the default color for one of the programs.
     * \param[in] index index of the program, in 0..PRG_NB - 1
     * \param[in] r the red component, in 0.0f..1.0f
     * \param[in] g the green component, in 0.0f..1.0f
     * \param[in] b the blue component, in 0.0f..1.0f
     */
    inline void set_color(GLuint index, float r, float g, float b) {
        colors[index][0] = r;
        colors[index][1] = g;
        colors[index][2] = b;        
    }
    
    /************************************************************************/
    //
    // Shaders for automatic generation of per-facet normals and
    //  fragment-level mesh rendering.

    // Note: GLSL version 150 is the lowest version that supports
    //  geometry shaders (we use it for largest compatibility).
    // Latest GLSL version is 430 (to be used if we use SSBOs to implement
    //  irregular mesh rendering)


    /**
     * \brief The fragment shader for polygons.
     * \details Used by all polygons and cells programs. 
     *  Does front/back shading in different colors and 
     *  fragment-level mesh display.
     */
    const char* fshader_source =
        "#version 150 compatibility                                         \n"
        "flat in float diffuse;                                             \n"
        "flat in float specular;                                            \n"
        "flat in vec3  edge_mask;                                           \n"
        "in vec2 bary;                                                      \n"
        "uniform bool mesh = true ;                                         \n"
        "uniform vec3 mesh_color = vec3(0.0, 0.0, 0.0) ;                    \n"
        "uniform bool lighting = true ;                                     \n"
        "out vec4 frag_color ;                                              \n"
        "float edge_factor(){                                               \n"
        "    vec3 bary3 = vec3(bary.x, bary.y, 1.0-bary.x-bary.y) ;         \n"
        "    vec3 d = fwidth(bary3);                                        \n"
        "    vec3 a3 = smoothstep(vec3(0.0,0.0,0.0), d*1.3, bary3);         \n"
        "    a3 = vec3(1.0, 1.0, 1.0) - edge_mask + edge_mask*a3;           \n"
        "    return min(min(a3.x, a3.y), a3.z);                             \n"
        "}                                                                  \n"
        "void main() {                                                      \n"
        "    float s = (lighting && gl_FrontFacing) ? 1.0 : -1.0 ;          \n"
        "    vec4 Kdiff = gl_FrontFacing ?                                  \n"
        "         gl_FrontMaterial.diffuse : gl_BackMaterial.diffuse ;      \n"
        "    float sdiffuse = s * diffuse ;                                 \n"
        "    vec4 result = vec4(0.1, 0.1, 0.1, 1.0);                        \n"
        "    if(sdiffuse > 0.0) {                                           \n"
        "       result += sdiffuse*Kdiff +                                  \n"
        "                 specular*gl_FrontMaterial.specular;               \n"
        "    }                                                              \n"
        "    frag_color = mesh ?                                            \n"
        "                  mix(vec4(mesh_color,1.0),result,edge_factor()) : \n"
        "                  result ;                                         \n"
        "}                                                                  \n";

    /**
     * \brief The fragment shader for points.
     * \details Uses vshader_pass_through. Makes the points appear 
     *  as small spheres.
     */
    const char* points_fshader_source =
        "#version 150 compatibility                                         \n"
        "out vec4 frag_color ;                                              \n"
        "void main() {                                                      \n"
        "   vec2 V = gl_TexCoord[0].xy - vec2(0.5, 0.5);                    \n"
        "   float d = 1.0-4.0*dot(V,V);                                     \n"
        "   if(d < 0.0) {                                                   \n"
        "      discard;                                                     \n"
        "   }                                                               \n"
        "   frag_color = d*gl_Color;                                        \n"
        "}                                                                  \n";
    
    /** 
     * \brief Some utility functions for the geometry shaders.
     * \details Provides functions for clipping, projection, and
     *  for generating shaded polygons.
     *  - flat_shaded_triangle(p1,p2,p3,pp1,pp2,pp3,do_clip) where
     *   (p1,p2,p3) are the coordinates in world space, (pp1,pp2,pp3) the
     *   transformed coordinates in clip space and do_clip specifies whether
     *   the triangle should be clipped.
     *  - flat_shaded_quad(p1,p2,p3,p4,pp1,pp2,pp3,pp4,do_clip,edges)
     */
    const char* gshader_utils_source =
        "#version 150 compatibility                                         \n"
        "flat out float diffuse;                                            \n"
        "flat out float specular;                                           \n"
        "flat out vec3 edge_mask;                                           \n"
        "uniform bool lighting=true;                                        \n"
        "uniform bool clipping=true;                                        \n"
        "out float gl_ClipDistance[1];                                      \n"
        "out vec2 bary;                                                     \n"
        "                                                                   \n"
        "vec4 project(vec4 V) {                                             \n"
        "   return gl_ModelViewProjectionMatrix * V ;                       \n"
        "}                                                                  \n"
        "float clip(vec4 V, bool do_clip) {                                 \n"
        "  return do_clip ? dot(gl_ModelViewMatrix*V,gl_ClipPlane[0]) :0.0; \n"
        "}                                                                  \n"
        "float cosangle(vec3 N, vec3 L) {                                   \n"
        "   float s = inversesqrt(dot(N,N)*dot(L,L)) ;                      \n"
        "   return s*dot(N,L) ;                                             \n"
        "}                                                                  \n"
        "void flat_shaded_triangle(                                         \n"
        "     vec4 p1,  vec4 p2,  vec4 p3,                                  \n"
        "     vec4 pp1, vec4 pp2, vec4 pp3,                                 \n"
        "     bool do_clip                                                  \n"
        "  ) {                                                              \n"
        "   if(lighting) {                                                  \n"
        "      vec3 N = gl_NormalMatrix * cross((p2-p1).xyz,(p3-p1).xyz) ;  \n"
        "      vec3 L = gl_LightSource[0].position.xyz ;                    \n"
        "      diffuse = cosangle(N,L) ;                                    \n"
        "      float NdotH = cosangle(N,gl_LightSource[0].halfVector.xyz) ; \n"
        "      specular = pow(abs(NdotH),gl_FrontMaterial.shininess);       \n"
        "   } else {                                                        \n"
        "       diffuse = -1.0 ; specular = 0.0 ;                           \n"
        "   }                                                               \n"
        "   edge_mask = vec3(1.0,1.0,1.0);                                  \n"
        "   gl_ClipDistance[0] = clip(p1, do_clip);                         \n"
        "   gl_Position=pp1; bary = vec2(1.0,0.0) ; EmitVertex();           \n"
        "   gl_ClipDistance[0] = clip(p2, do_clip);                         \n"
        "   gl_Position=pp2; bary = vec2(0.0,1.0) ; EmitVertex();           \n"
        "   gl_ClipDistance[0] = clip(p3, do_clip);                         \n"
        "   gl_Position=pp3; bary = vec2(0.0,0.0) ; EmitVertex();           \n"
        "   EndPrimitive();                                                 \n"
        "}                                                                  \n"
        "                                                                   \n"
        "void flat_shaded_quad(                                             \n"
        "     vec4 p1,  vec4 p2,  vec4 p3, vec4 p4,                         \n"
        "     vec4 pp1, vec4 pp2, vec4 pp3, vec4 pp4,                       \n"
        "     bool do_clip                                                  \n"
        "  ) {                                                              \n"
        "   if(lighting) {                                                  \n"
        "      vec3 N = gl_NormalMatrix * (                                 \n" 
        "           cross((p2-p1).xyz,(p4-p1).xyz) +                        \n"
        "           cross((p4-p3).xyz,(p2-p3).xyz)                          \n"
        "           );                                                      \n"
        "      vec3 L = gl_LightSource[0].position.xyz ;                    \n"
        "      diffuse = cosangle(N,L) ;                                    \n"
        "      float NdotH = cosangle(N,gl_LightSource[0].halfVector.xyz) ; \n"
        "      specular = pow(abs(NdotH),gl_FrontMaterial.shininess);       \n"
        "   } else {                                                        \n"
        "       diffuse = -1.0 ; specular = 0.0 ;                           \n"
        "   }                                                               \n"
        "   float cl1 = clip(p1,do_clip);                                   \n"
        "   float cl2 = clip(p2,do_clip);                                   \n"
        "   float cl3 = clip(p3,do_clip);                                   \n"
        "   float cl4 = clip(p4,do_clip);                                   \n"
        "   edge_mask = vec3(0.0, 1.0, 1.0);                                \n"
        "   gl_ClipDistance[0]=cl1; gl_Position=pp1; bary=vec2(1.0,0.0);    \n"
        "   EmitVertex();                                                   \n"
        "   gl_ClipDistance[0]=cl2; gl_Position=pp2; bary=vec2(0.0,1.0);    \n"
        "   EmitVertex();                                                   \n"
        "   gl_ClipDistance[0]=cl4; gl_Position=pp4; bary=vec2(0.0,0.0);    \n"
        "   EmitVertex();                                                   \n"
        "   edge_mask = vec3(0.0, 1.0, 1.0);                                \n"
        "   gl_ClipDistance[0]=cl3; gl_Position=pp3; bary=vec2(1.0,0.0);    \n"
        "   EmitVertex();                                                   \n"
        "   EndPrimitive();                                                 \n"
        "}                                                                  \n"
        ;
    
    /**
     * \brief The pass-through vertex shader.
     * \details Used by points, quads, tets, prisms
     */
    const char* vshader_pass_through_source =
        "#version 150 compatibility                                         \n"
        " void main(void) {                                                 \n"
        "    gl_Position = gl_Vertex;                                       \n"
        " }                                                                 \n";


    /**
     * \brief The tesselation evaluation shader for hexes.
     * \details Gathers the eight vertices into the array passed
     *  to the geometry shader.
     *  Each hex is generated twice, for the second one, the output
     *  boolean discardme is set to one, to indicate to the geometry
     *  shader that the hex should be discarded (sounds stupid, but
     *  I did not find a smarter way...).
     */
    const char* teshader_hex_source =
        "#version 400                                                       \n"
        "layout(isolines, point_mode) in;                                   \n"
        "const int nb_vertices = 8;                                         \n"
        "out Data {                                                         \n"
        "    vec3 p[nb_vertices];                                           \n"
        "    bool discardme;                                                \n"
        "} DataOut;                                                         \n"
        "void main() {                                                      \n"
        "   if(gl_TessCoord.x == 0.0) {                                     \n"
        "      DataOut.discardme = true;                                    \n"
        "      return;                                                      \n"
        "   }                                                               \n"
        "   DataOut.discardme = false;                                      \n"
        "   for(int i=0; i<nb_vertices; ++i) {                              \n"
        "     DataOut.p[i] = gl_in[i].gl_Position.xyz;                      \n"
        "   }                                                               \n"
        "}                                                                  \n";
    
    /**
     * \brief The vertex shader for hexes if tesselation shader cannot be used.
     * \details For hexes, the pass-through vertex shader
     *  cannot be used, since there is no standard OpenGL primitive
     *  with eight vertices (maximum is 6). We use GL_POINTS and 
     *  pass the 8 coordinates as a generic attribute.
     */
    const char* vshader_hex_source =
        "#version 150 compatibility                                         \n"
        "#extension GL_ARB_explicit_attrib_location : enable                \n"
        " const int nb_vertices = 8;                                        \n"
        " layout(location=0) in vec3 p_in[nb_vertices];                     \n"
        " out Data {                                                        \n"
        "     vec3 p[nb_vertices];                                          \n"
        "     bool discardme;                                               \n"
        " } DataOut;                                                        \n"
        " void main(void) {                                                 \n"
        "   DataOut.discardme = false;                                      \n"
        "   for(int i=0; i<nb_vertices; ++i) {                              \n"
        "       DataOut.p[i] = p_in[i];                                     \n"
        "   }                                                               \n"
        " }                                                                 \n";

    /**
     * \brief The geometry shader for hexes.
     * \details Uses vshader_hex and gshader_utils 
     */
    const char* gshader_hex_source = 
        "layout(points) in;                                                 \n"
        "layout(triangle_strip, max_vertices = 24) out;                     \n"
        " in Data {                                                         \n"
        "     vec3 p[8];                                                    \n"
        "     bool discardme;                                               \n"
        " } DataIn[] ;                                                      \n"
        "uniform float shrink = 0.0;                                        \n"
        "                                                                   \n"
        "void draw_hex(                                                     \n"
        "   vec4 p0, vec4 p1, vec4 p2, vec4 p3,                             \n"
        "   vec4 p4, vec4 p5, vec4 p6, vec4 p7                              \n"
        ") {                                                                \n"
        "    if(clipping) {                                                 \n"
        "       if(                                                         \n"
        "           clip(p0,true) < 0.0 && clip(p1,true) < 0.0 &&           \n"
        "           clip(p2,true) < 0.0 && clip(p3,true) < 0.0 &&           \n"
        "           clip(p4,true) < 0.0 && clip(p5,true) < 0.0 &&           \n"
        "           clip(p6,true) < 0.0 && clip(p7,true) < 0.0              \n"
        "       ) {                                                         \n"
        "            return ;                                               \n"
        "       }                                                           \n"
        "    }                                                              \n"
        "   if(shrink != 0.0) {                                             \n"
        "      vec4 g = (1.0/8.0)*(p0+p1+p2+p3+p4+p5+p6+p7);                \n"
        "      p0 = shrink*g + (1.0-shrink)*p0;                             \n"
        "      p1 = shrink*g + (1.0-shrink)*p1;                             \n"
        "      p2 = shrink*g + (1.0-shrink)*p2;                             \n"
        "      p3 = shrink*g + (1.0-shrink)*p3;                             \n"
        "      p4 = shrink*g + (1.0-shrink)*p4;                             \n"
        "      p5 = shrink*g + (1.0-shrink)*p5;                             \n"
        "      p6 = shrink*g + (1.0-shrink)*p6;                             \n"
        "      p7 = shrink*g + (1.0-shrink)*p7;                             \n"
        "   }                                                               \n"
        "   vec4 pp0 = project(p0);                                         \n"
        "   vec4 pp1 = project(p1);                                         \n"
        "   vec4 pp2 = project(p2);                                         \n"
        "   vec4 pp3 = project(p3);                                         \n"
        "   vec4 pp4 = project(p4);                                         \n"
        "   vec4 pp5 = project(p5);                                         \n"
        "   vec4 pp6 = project(p6);                                         \n"
        "   vec4 pp7 = project(p7);                                         \n"
        "   flat_shaded_quad(p0,p2,p6,p4,pp0,pp2,pp6,pp4,false);            \n"
        "   flat_shaded_quad(p3,p1,p5,p7,pp3,pp1,pp5,pp7,false);            \n"
        "   flat_shaded_quad(p1,p0,p4,p5,pp1,pp0,pp4,pp5,false);            \n"
        "   flat_shaded_quad(p2,p3,p7,p6,pp2,pp3,pp7,pp6,false);            \n"
        "   flat_shaded_quad(p1,p3,p2,p0,pp1,pp3,pp2,pp0,false);            \n"
        "   flat_shaded_quad(p4,p6,p7,p5,pp4,pp6,pp7,pp5,false);            \n"
        "}                                                                  \n"
        "                                                                   \n"
        "void main() {                                                      \n"
        "   if(DataIn[0].discardme) { return; }                             \n"
        "   draw_hex(                                                       \n"
        "      vec4(DataIn[0].p[0],1.0),                                    \n"
        "      vec4(DataIn[0].p[1],1.0),                                    \n"
        "      vec4(DataIn[0].p[2],1.0),                                    \n"
        "      vec4(DataIn[0].p[3],1.0),                                    \n"
        "      vec4(DataIn[0].p[4],1.0),                                    \n"
        "      vec4(DataIn[0].p[5],1.0),                                    \n"
        "      vec4(DataIn[0].p[6],1.0),                                    \n"
        "      vec4(DataIn[0].p[7],1.0)                                     \n"
        "   );                                                              \n"
        "}                                                                  \n";


    /**
     * \brief The tesselation evaluation shader for pyramids.
     * \details Gathers the eight vertices into the array passed
     *  to the geometry shader.
     *  Each pyramid is generated twice, for the second one, the output
     *  boolean discardme is set to one, to indicate to the geometry
     *  shader that the hex should be discarded (sounds stupid, but
     *  I did not find a smarter way...).
     */
    const char* teshader_pyramid_source =
        "#version 400                                                       \n"
        "layout(isolines, point_mode) in;                                   \n"
        "const int nb_vertices = 5;                                         \n"
        "out Data {                                                         \n"
        "    vec3 p[nb_vertices];                                           \n"
        "    bool discardme;                                                \n"
        "} DataOut;                                                         \n"
        "void main() {                                                      \n"
        "   if(gl_TessCoord.x == 0.0) {                                     \n"
        "      DataOut.discardme = true;                                    \n"
        "      return;                                                      \n"
        "   }                                                               \n"
        "   DataOut.discardme = false;                                      \n"
        "   for(int i=0; i<nb_vertices; ++i) {                              \n"
        "     DataOut.p[i] = gl_in[i].gl_Position.xyz;                      \n"
        "   }                                                               \n"
        "}                                                                  \n";

    
    /**
     * \brief The vertex shader for pyramids if tesselation canot be used.
     * \details For pyramids, the pass-through vertex shader
     *  cannot be used, since there is no standard OpenGL primitive
     *  with five vertices. We use GL_POINTS and 
     *  pass the 5 points as generic attributes.
     */
    const char* vshader_pyramid_source =
        "#version 150 compatibility                                         \n"
        "#extension GL_ARB_explicit_attrib_location : enable                \n"
        " const int nb_vertices = 5;                                        \n"
        " layout(location=0) in vec3 p_in[nb_vertices];                     \n"
        " out Data {                                                        \n"
        "     vec3 p[nb_vertices];                                          \n"
        "     bool discardme;                                               \n"
        " } DataOut ;                                                       \n"
        " void main(void) {                                                 \n"
        "   DataOut.discardme = false;                                      \n"
        "   for(int i=0; i<nb_vertices; ++i) {                              \n"
        "       DataOut.p[i] = p_in[i];                                     \n"
        "   }                                                               \n"
        " }                                                                 \n";
    
    /**
     * \brief The geometry shader for pyramids.
     * \details Uses vshader_pyramid and gshader_utils 
     */
    const char* gshader_pyramid_source = 
        "layout(points) in;                                                 \n"
        "layout(triangle_strip, max_vertices = 28) out;                     \n"
        " in Data {                                                         \n"
        "     vec3 p[5];                                                    \n"
        "     bool discardme;                                               \n"
        " } DataIn[];                                                       \n"
        " uniform float shrink = 0.0;                                       \n"
        "                                                                   \n"
        "void draw_pyramid(                                                 \n"
        "   vec4 p0, vec4 p1, vec4 p2, vec4 p3, vec4 p4                     \n"
        ") {                                                                \n"
        "    if(clipping) {                                                 \n"
        "       if(                                                         \n"
        "           clip(p0,true) < 0.0 &&  clip(p1,true) < 0.0 &&          \n"
        "           clip(p2,true) < 0.0 &&  clip(p3,true) < 0.0 &&          \n"
        "           clip(p4,true) < 0.0                                     \n"
        "       ) {                                                         \n"
        "            return ;                                               \n"
        "       }                                                           \n"
        "    }                                                              \n"
        "   if(shrink != 0.0) {                                             \n"
        "      vec4 g = (1.0/5.0)*(p0+p1+p2+p3+p4);                         \n"
        "      p0 = shrink*g + (1.0-shrink)*p0;                             \n"
        "      p1 = shrink*g + (1.0-shrink)*p1;                             \n"
        "      p2 = shrink*g + (1.0-shrink)*p2;                             \n"
        "      p3 = shrink*g + (1.0-shrink)*p3;                             \n"
        "      p4 = shrink*g + (1.0-shrink)*p4;                             \n"
        "   }                                                               \n"
        "   vec4 pp0 = project(p0);                                         \n"
        "   vec4 pp1 = project(p1);                                         \n"
        "   vec4 pp2 = project(p2);                                         \n"
        "   vec4 pp3 = project(p3);                                         \n"
        "   vec4 pp4 = project(p4);                                         \n"
        "   flat_shaded_quad(p0,p1,p2,p3,pp0,pp1,pp2,pp3,false);            \n"
        "   flat_shaded_triangle(p0,p4,p1,pp0,pp4,pp1,false);               \n"
        "   flat_shaded_triangle(p0,p3,p4,pp0,pp3,pp4,false);               \n"
        "   flat_shaded_triangle(p2,p4,p3,pp2,pp4,pp3,false);               \n"
        "   flat_shaded_triangle(p2,p1,p4,pp2,pp1,pp4,false);               \n"
        "}                                                                  \n"
        "                                                                   \n"
        "void main() {                                                      \n"
        "   if(DataIn[0].discardme) { return; }                             \n"
        "   draw_pyramid(                                                   \n"
        "      vec4(DataIn[0].p[0],1.0),                                    \n"
        "      vec4(DataIn[0].p[1],1.0),                                    \n"
        "      vec4(DataIn[0].p[2],1.0),                                    \n"
        "      vec4(DataIn[0].p[3],1.0),                                    \n"
        "      vec4(DataIn[0].p[4],1.0)                                     \n"
        "   );                                                              \n"
        "}                                                                  \n";

    
    /**
     * \brief The geometry shader for triangles.
     * \details Uses vshader_pass_through and gshader_utils.
     */
    const char* gshader_tri_source =
        "layout(triangles) in;                                              \n"
        "layout(triangle_strip, max_vertices = 3) out;                      \n"
        "                                                                   \n"
        "void draw_triangle(vec4 p1, vec4 p2, vec4 p3) {                    \n"
        "   flat_shaded_triangle(                                           \n"
        "     p1,p2,p3,                                                     \n"
        "     project(p1), project(p2), project(p3),                        \n"
        "     true                                                          \n"
        "   );                                                              \n"
        "}                                                                  \n"
        "                                                                   \n"
        "void main() {                                                      \n"
        "    draw_triangle(                                                 \n"
        "       gl_in[0].gl_Position,                                       \n"
        "       gl_in[1].gl_Position,                                       \n"
        "       gl_in[2].gl_Position                                        \n"
        "    );                                                             \n"
        "}                                                                  \n";


    /**
     * \brief The geometry shader for quads.
     * \details Uses v_shader_pass_through and gshader_utils.
     */
    const char* gshader_quad_source =
        "layout(lines_adjacency) in;                                        \n"
        "layout(triangle_strip, max_vertices = 4) out;                      \n"
        "                                                                   \n"
        "void draw_quad(vec4 p1, vec4 p2, vec4 p3, vec4 p4) {               \n"
        "   flat_shaded_quad(                                               \n"
        "     p1,p2,p3,p4,                                                  \n"
        "     project(p1), project(p2), project(p3), project(p4),           \n"
        "     true                                                          \n"
        "   );                                                              \n"
        "}                                                                  \n"
        "                                                                   \n"
        "void main() {                                                      \n"
        "    draw_quad(                                                     \n"
        "       gl_in[0].gl_Position,                                       \n"
        "       gl_in[1].gl_Position,                                       \n"
        "       gl_in[2].gl_Position,                                       \n"
        "       gl_in[3].gl_Position                                        \n"
        "    );                                                             \n"
        "}                                                                  \n";
    
    /**
     * \brief The geometry shader for tetrahedra.
     * \details Uses v_shader_pass_through and gshader_utils.
     */
    const char* gshader_tet_source =
        "layout(lines_adjacency) in;                                        \n"
        "layout(triangle_strip, max_vertices = 12) out;                     \n"
        "uniform float shrink=0.0;                                          \n"
        "                                                                   \n"
        "void draw_tet(vec4 p0, vec4 p1, vec4 p2, vec4 p3) {                \n"
        "    if(clipping) {                                                 \n"
        "       if(                                                         \n"
        "           clip(p0,true) < 0.0 && clip(p1,true) < 0.0 &&           \n"
        "           clip(p2,true) < 0.0 && clip(p3,true) < 0.0              \n"
        "       ) {                                                         \n"
        "            return ;                                               \n"
        "       }                                                           \n"
        "    }                                                              \n"
        "   if(shrink != 0.0) {                                             \n"
        "      vec4 g = (1.0/4.0)*(p0+p1+p2+p3);                            \n"
        "      p0 = shrink*g + (1.0-shrink)*p0;                             \n"
        "      p1 = shrink*g + (1.0-shrink)*p1;                             \n"
        "      p2 = shrink*g + (1.0-shrink)*p2;                             \n"
        "      p3 = shrink*g + (1.0-shrink)*p3;                             \n"
        "   }                                                               \n"
        "   vec4 pp0 = project(p0);                                         \n"
        "   vec4 pp1 = project(p1);                                         \n"
        "   vec4 pp2 = project(p2);                                         \n"
        "   vec4 pp3 = project(p3);                                         \n"
        "   flat_shaded_triangle(p0,p1,p2,pp0,pp1,pp2,false);               \n"
        "   flat_shaded_triangle(p1,p0,p3,pp1,pp0,pp3,false);               \n"
        "   flat_shaded_triangle(p0,p2,p3,pp0,pp2,pp3,false);               \n"
        "   flat_shaded_triangle(p2,p1,p3,pp2,pp1,pp3,false);               \n"
        "}                                                                  \n"
        "                                                                   \n"
        "void main() {                                                      \n"
        "    draw_tet(                                                      \n"
        "       gl_in[0].gl_Position,                                       \n"
        "       gl_in[1].gl_Position,                                       \n"
        "       gl_in[2].gl_Position,                                       \n"
        "       gl_in[3].gl_Position                                        \n"
        "    );                                                             \n"
        "}                                                                  \n";


    /**
     * \brief The geometry shader for prisms
     * \details Uses vshader_pass_through and gshader_utils.
     */
    const char* gshader_prism_source =
        "layout(triangles_adjacency) in;                                    \n"
        "layout(triangle_strip, max_vertices = 18) out;                     \n"
        "uniform float shrink = 0.0;                                        \n"
        "                                                                   \n"
        "void draw_prism(                                                   \n"
        "   vec4 p0, vec4 p1, vec4 p2,                                      \n"
        "   vec4 p3, vec4 p4, vec4 p5                                       \n"
        ") {                                                                \n"
        "    if(clipping) {                                                 \n"
        "       if(                                                         \n"
        "           clip(p0,true) < 0.0 && clip(p1,true) < 0.0 &&           \n"
        "           clip(p2,true) < 0.0 && clip(p3,true) < 0.0 &&           \n"
        "           clip(p4,true) < 0.0 && clip(p5,true) < 0.0              \n"
        "       ) {                                                         \n"
        "            return ;                                               \n"
        "       }                                                           \n"
        "    }                                                              \n"
        "   if(shrink != 0.0) {                                             \n"
        "      vec4 g = (1.0/6.0)*(p0+p1+p2+p3+p4+p5);                      \n"
        "      p0 = shrink*g + (1.0-shrink)*p0;                             \n"
        "      p1 = shrink*g + (1.0-shrink)*p1;                             \n"
        "      p2 = shrink*g + (1.0-shrink)*p2;                             \n"
        "      p3 = shrink*g + (1.0-shrink)*p3;                             \n"
        "      p4 = shrink*g + (1.0-shrink)*p4;                             \n"
        "      p5 = shrink*g + (1.0-shrink)*p5;                             \n"        
        "   }                                                               \n"
        "   vec4 pp0 = project(p0);                                         \n"
        "   vec4 pp1 = project(p1);                                         \n"
        "   vec4 pp2 = project(p2);                                         \n"
        "   vec4 pp3 = project(p3);                                         \n"
        "   vec4 pp4 = project(p4);                                         \n"
        "   vec4 pp5 = project(p5);                                         \n"
        "   flat_shaded_triangle(p0,p1,p2,pp0,pp1,pp2,false);               \n"
        "   flat_shaded_triangle(p3,p5,p4,pp3,pp5,pp4,false);               \n"
        "   flat_shaded_quad(p0,p3,p4,p1,pp0,pp3,pp4,pp1,false);            \n"
        "   flat_shaded_quad(p0,p2,p5,p3,pp0,pp2,pp5,pp3,false);            \n"
        "   flat_shaded_quad(p1,p4,p5,p2,pp1,pp4,pp5,pp2,false);            \n"
        "}                                                                  \n"
        "                                                                   \n"
        "void main() {                                                      \n"
        "    draw_prism(                                                    \n"
        "       gl_in[0].gl_Position,                                       \n"
        "       gl_in[1].gl_Position,                                       \n"
        "       gl_in[2].gl_Position,                                       \n"
        "       gl_in[3].gl_Position,                                       \n"
        "       gl_in[4].gl_Position,                                       \n"
        "       gl_in[5].gl_Position                                        \n"
        "    );                                                             \n"
        "}                                                                  \n";

    /************************************************************************/

    /**
     * \brief Compiles a shader for a specific target.
     * \details One may use \p source1 for library functions common 
     *  to different shaders (then the code of the shader is in \p source2). 
     *  It may seem more natural to generate a shader object with library 
     *  functions, but OpenGL documentation does not recommend
     *  to do so (and it did not seem to work). Errors are detected and 
     *  displayed to std::err.
     * \param[in] target the OpenGL shader target ()
     * \param[in] source1 the source of the shader (ASCII string)
     * \param[in] source2 an optional additional source string or 0 if unused
     * \return the OpenGL opaque Id of the created shader object.
     */
    GLuint compile_shader(
        GLenum target, const char* source1, const char* source2=0
    ) {
        GLuint s_handle = glCreateShader(target);
        if(s_handle == 0) {
            GEO::Logger::err("GLSL") << "Could not create shader"
                << std::endl;
            exit(1);
        }
        const char* sources[2];
        sources[0] = source1;
        sources[1] = source2;
        glShaderSource(s_handle, (source2 == 0) ? 1 : 2, sources, 0);
        glCompileShader(s_handle);
        GLint compile_status;
        glGetShaderiv(s_handle, GL_COMPILE_STATUS, &compile_status);
        if(!compile_status) {
            GLchar compiler_message[4096];
            glGetShaderInfoLog(
                s_handle, sizeof(compiler_message), 0, compiler_message
            );
            std::cerr << "GLSL compiler status :"
                << compile_status << std::endl;
            std::cerr << "GLSL compiler message:"
                << compiler_message << std::endl;
            glDeleteShader(s_handle);
            s_handle = 0;
        }
        return s_handle;
    }

    /**
     * \brief Creates a program from a zero-terminated list of shaders
     * \details Errors are detected and displayed to std::cerr.
     * \param[in] shader the first shader of the list
     * \return the id of the program
     */
    GLuint setup_program(GLuint shader, ...) {
        GLuint program = glCreateProgram();
        va_list args;
        va_start(args,shader);
        while(shader != 0) {
            glAttachShader(program, shader);
            shader = va_arg(args, GLuint);
        }
        va_end(args);
        glLinkProgram(program);
        GLint link_status;
        glGetProgramiv(program, GL_LINK_STATUS, &link_status);
        if(!link_status) {
            GLchar linker_message[4096];
            glGetProgramInfoLog(
                program, sizeof(linker_message), 0, linker_message
            );
            std::cerr << "GLSL linker status :" << link_status << std::endl;
            std::cerr << "GLSL linker message:" << linker_message << std::endl;
            glDeleteProgram(program);
            program = 0;
        }
        return program;
    }

    /**
     * \brief Creates all the shaders used by vorpaview.
     * \details Compiles the three shaders (vertex, geometry and fragment),
     *  links them into a program, and displays the potential error
     *  messages to std::cerr. There is one program for displaying
     *  triangles and one program for displaying tetrahedra.
     */
    void setup_shaders() {
        if(GEO::CmdLine::get_arg_bool("plain")) {
            GEO::Logger::out("Shaders")
                << "Deactivated (plain mode specified on command line)"
                << std::endl;
            plain_mode = true;
            return;
        }
        const char* shading_language_ver_str = (const char*)glGetString(
            GL_SHADING_LANGUAGE_VERSION
        );
        GEO::Logger::out("Shaders") << "GLSL version = "
                                    << shading_language_ver_str << std::endl;
        double shading_language_ver = atof(shading_language_ver_str);
        if(shading_language_ver < 1.5) {
            GEO::Logger::out("Shaders")
                << "Deactivated (requires GLSL version >= 1.50)"
                << std::endl;
            plain_mode = true;
            return;
        } else {
            GEO::Logger::out("Shaders") << "Using GLSL shaders" << std::endl;
        }

        has_tessellation_shaders =
            GEO::CmdLine::get_arg_bool("tessellation") &&
            (shading_language_ver >= 4.0);

        if(has_tessellation_shaders) {
            GEO::Logger::out("Shaders")
                << "Using Tessellation shaders" << std::endl;
        } else {
            GEO::Logger::out("Shaders")
                << "Deactivated Tessellation shaders" << std::endl;            
        }
        
        GLuint vshader_pass_through = compile_shader(
            GL_VERTEX_SHADER, vshader_pass_through_source
        );
        GLuint gshader_tri = compile_shader(
            GL_GEOMETRY_SHADER, gshader_utils_source, gshader_tri_source
        );
        GLuint gshader_tet = compile_shader(
            GL_GEOMETRY_SHADER, gshader_utils_source, gshader_tet_source
        );
        GLuint gshader_quad = compile_shader(
            GL_GEOMETRY_SHADER, gshader_utils_source, gshader_quad_source
        );
        GLuint gshader_prism = compile_shader(
            GL_GEOMETRY_SHADER, gshader_utils_source, gshader_prism_source
        );
        GLuint fshader = compile_shader(
            GL_FRAGMENT_SHADER, fshader_source
        );
        GLuint points_fshader = compile_shader(
            GL_FRAGMENT_SHADER, points_fshader_source
        );

        GLuint gshader_hex = compile_shader(
            GL_GEOMETRY_SHADER, gshader_utils_source, gshader_hex_source
        );
        GLuint gshader_pyramid = compile_shader(
            GL_GEOMETRY_SHADER, gshader_utils_source, gshader_pyramid_source
        );
            
        programs[PRG_POINTS] = setup_program(points_fshader, 0);
        programs[PRG_TRI] = setup_program(
            vshader_pass_through, gshader_tri, fshader, 0
        );
        programs[PRG_QUAD] = setup_program(
            vshader_pass_through, gshader_quad, fshader, 0
        );
        programs[PRG_TET] = setup_program(
            vshader_pass_through, gshader_tet, fshader, 0
        );
        programs[PRG_PRISM] = setup_program(
            vshader_pass_through, gshader_prism, fshader, 0
        );
        
        if(has_tessellation_shaders) {
            GLuint teshader_hex = compile_shader(
                GL_TESS_EVALUATION_SHADER, teshader_hex_source
            );
            GLuint teshader_pyramid = compile_shader(
                GL_TESS_EVALUATION_SHADER, teshader_pyramid_source
            );
            programs[PRG_HEX] = setup_program(
                vshader_pass_through, teshader_hex, gshader_hex, fshader, 0
            );
            programs[PRG_PYRAMID] = setup_program(
                vshader_pass_through, teshader_pyramid,
                gshader_pyramid, fshader, 0
            );                        
        } else {
            GLuint vshader_hex = compile_shader(
                GL_VERTEX_SHADER, vshader_hex_source
            );            
            GLuint vshader_pyramid = compile_shader(
                GL_VERTEX_SHADER, vshader_pyramid_source
            );
            programs[PRG_HEX] = setup_program(
                vshader_hex, gshader_hex, fshader, 0
            );
            programs[PRG_PYRAMID] = setup_program(
                vshader_pyramid, gshader_pyramid, fshader, 0
            );            
        }
    }

    /**
     * \brief Zooms in.
     * \details Zooming factor is 1.1x.
     */
    void zoom_in() {
        *glut_viewer_float_ptr(GLUT_VIEWER_ZOOM) *= 1.1f;
    }

    /**
     * \brief Zooms out.
     * \details De-zooming factor is (1/1.1)x.
     */
    void zoom_out() {
        *glut_viewer_float_ptr(GLUT_VIEWER_ZOOM) /= 1.1f;
    }

    /**
     * \brief Toggles black or white background color.
     */
    void toggle_background() {
        white_bg = !white_bg;
        if(white_bg) {
            glut_viewer_set_background_color(1.0, 1.0, 1.0);
        } else {
            glut_viewer_set_background_color(0.0, 0.0, 0.0);
        }
        // Iterate over all programs
        // except PRG_POINTS (programs[1] .. programs[PRG_NB-1])
        for(int i=1; i<PRG_NB; ++i) {
            if(programs[i] != 0) {
                GLint loc = glGetUniformLocation(programs[i], "mesh_color");
                glUseProgram(programs[i]);
                if(white_bg) {
                    glUniform3f(loc, 0.0, 0.0, 0.0);
                } else {
                    glUniform3f(loc, 1.0, 1.0, 1.0);
                }
                glUseProgram(0);
            }
        }
    }

    /**
     * \brief Toggles mesh display.
     */
    void toggle_mesh() {
        show_mesh = !show_mesh;
        // Iterate over all programs
        // except PRG_POINTS (programs[1] .. programs[PRG_NB-1])
        for(int i=1; i<PRG_NB; ++i) {
            if(programs[i] != 0) {
                glUseProgram(programs[i]);                
                GLint loc = glGetUniformLocation(programs[i], "mesh");
                glUniform1i(loc, show_mesh);
                glUseProgram(0);
            }
        }
    }

    /**
     * \brief Updates the shrinking factor.
     */
    void update_shrink() {
        for(int i=1; i<PRG_NB; ++i) {
            if(programs[i] != 0) {
                glUseProgram(programs[i]);                
                GLint loc = glGetUniformLocation(programs[i], "shrink");
                glUniform1f(loc,shrink);
                glUseProgram(0);
            }
        }
    }

    /**
     * \brief Increases cell shrinking factor.
     */
    void inc_shrink() {
        shrink += 0.1f;
        if(shrink > 1.0f) {
            shrink = 1.0f;
        }
        update_shrink();
    }

    /**
     * \brief Decreases cell shrinking factor.
     */
    void dec_shrink() {
        shrink -= 0.1f;
        if(shrink < 0.0f) {
            shrink = 0.0f;
        }
        update_shrink();        
    }
    
    
    /**
     * \brief Toggles color / BW display.
     */
    void toggle_colors() {
        show_colors = !show_colors;
    }

    /**
     * \brief Toggles lighting / constant color mode.
     */
    void toggle_lighting() {
        lighting = !lighting;
        // Iterate over all programs
        // except PRG_POINTS (programs[1] .. programs[PRG_NB-1])
        for(int i=1; i<PRG_NB; ++i) {
            if(programs[i] != 0) {
                glUseProgram(programs[i]);
                GLint loc = glGetUniformLocation(programs[i], "lighting");
                glUniform1i(loc, lighting);
                glUseProgram(0);
            }
        }
    }

    /**
     * \brief Increments the time of the Optimal Transport animation.
     */
    void increment_OTM_time() {
        OTM_time += 0.05f;
        OTM_time = GEO::geo_min(OTM_time, 1.0f);
    }

    /**
     * \brief Decrements the time of the Optimal Transport animation.
     */
    void decrement_OTM_time() {
        OTM_time -= 0.05f;
        OTM_time = GEO::geo_max(OTM_time, 0.0f);        
    }
    
    /**
     * \brief Initializes OpenGL objects.
     * \details Specifed as glut_viewer_set_init_func() callback.
     */
    void init() {
        glewInit();
        setup_shaders();
        
        // In plain mode, there is no shading, therefore we need the mesh
        // in order to see something...
        if(plain_mode) {
            show_mesh = GL_TRUE;
        }
        
        glut_viewer_set_background_color(1.0, 1.0, 1.0);
        glut_viewer_add_toggle(
            'T', glut_viewer_is_enabled_ptr(GLUT_VIEWER_TWEAKBARS),
            "Toggle tweakbars"
        );
        glut_viewer_add_key_func('b', toggle_background, "Toggle background");
        glut_viewer_add_key_func('c', toggle_colors, "colors");
        glut_viewer_add_toggle('B', &show_borders, "borders");
        glut_viewer_add_key_func('z', zoom_in, "Zoom in");
        glut_viewer_add_key_func('Z', zoom_out, "Zoom out");
        glut_viewer_add_key_func('r', decrement_OTM_time, "Decrement OTM time");
        glut_viewer_add_key_func('t', increment_OTM_time, "Increment OTM time");
        glut_viewer_disable(GLUT_VIEWER_TWEAKBARS);
        glut_viewer_disable(GLUT_VIEWER_BACKGROUND);
        glut_viewer_add_key_func('m', toggle_mesh, "mesh");
    }

    /**
     * \brief Creates OpenGL-side arrays (VertexBufferObjects).
     * \details The first call creates the VertexBufferObjects
     *  that store the point coordinates and the triangle
     *  corner indices in the graphic board.
     */
    void create_VBOs_if_needed() {
        if(vertices_VBO == 0 && M.nb_vertices() != 0) {
            glGenBuffers(1, &vertices_VBO);
            glBindBuffer(GL_ARRAY_BUFFER, vertices_VBO);
            glBufferData(
                GL_ARRAY_BUFFER,
                GLsizeiptr(M.nb_vertices() * M.dimension() * sizeof(float)),
                M.vertex_ptr(0), GL_STATIC_DRAW
            );
            glEnableClientState(GL_VERTEX_ARRAY);
            unsigned int stride =
                (unsigned int) (M.dimension() * sizeof(float));
            geo_assert(M.dimension() == 3);
            glVertexPointer(3, GL_FLOAT, GLsizei(stride), 0);
        }
        if(facet_indices_VBO == 0 && M.nb_facets() != 0) {
            glGenBuffers(1, &facet_indices_VBO);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, facet_indices_VBO);
            glBufferData(
                GL_ELEMENT_ARRAY_BUFFER,
                GLsizeiptr(M.nb_corners() * sizeof(int)),
                M.corner_vertex_index_ptr(0), GL_STATIC_DRAW
            );
        }
        typedef GEOGen::MeshMutator<GEO::SinglePrecisionMesh> MeshMutator;
        if(cell_indices_VBO == 0 && M.nb_cells() != 0) {
            glGenBuffers(1, &cell_indices_VBO);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cell_indices_VBO);
            glBufferData(
                GL_ELEMENT_ARRAY_BUFFER,
                GLsizeiptr(MeshMutator::cell_vertices(M).size() * sizeof(int)),
                MeshMutator::cell_vertices(M).data(), GL_STATIC_DRAW
            );
        }
    }

    /**
     * \brief Binds the triangles array as current Vertex Buffer Object.
     * \details Creates the triangles Vertex Buffer Object if need be.
     */
    void bind_facets_VBO() {
        create_VBOs_if_needed();
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, facet_indices_VBO);
    }

    /**
     * \brief Binds the tetrahedra array as current Vertex Buffer Object.
     * \details Creates the tetrahedra Vertex Buffer Object if need be.
     */
    void bind_cells_VBO() {
        create_VBOs_if_needed();
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cell_indices_VBO);
    }

    /**
     * \brief Deletes all the previously created Vertex Buffer Objects.
     */
    void delete_VBOs_if_needed() {
        if(vertices_VBO != 0) {
            glDeleteBuffers(1, &vertices_VBO);
            vertices_VBO = 0;
        }
        if(facet_indices_VBO != 0) {
            glDeleteBuffers(1, &facet_indices_VBO);
            facet_indices_VBO = 0;
        }
        if(cell_indices_VBO != 0) {
            glDeleteBuffers(1, &cell_indices_VBO);
            cell_indices_VBO = 0;
        }
    }

    /**
     * \brief Draws a triangle mesh using VertexBufferObjects.
     * \details The implementation uses a Geometry Shader (does
     *   not work if the graphic board is too old).
     */
    void draw_triangles_VBOs() {
        if(M.nb_facets() == 0) {
            return;
        }
        bind_facets_VBO();
        glUseProgram(programs[PRG_TRI]);
        // Note: the fourth argument (0) corresponds to the bound VBO.        
        glDrawElements(
            GL_TRIANGLES, GLsizei(M.nb_corners()), GL_UNSIGNED_INT, 0
        );
        glUseProgram(0);
    }

    /**
     * \brief Draws a tetrahedral mesh using VertexBufferObjects.
     * \details The implementation uses a Geometry Shader (does
     *  not work if the graphic board is too old).
     */
    void draw_tets_with_shaders() {
        if(M.nb_tets() == 0) {
            return;
        }
        bind_cells_VBO();
        glUseProgram(programs[PRG_TET]);        
        // Note: the fourth argument (0) corresponds to the bound VBO.
        glDrawElements(
            GL_LINES_ADJACENCY, GLsizei(M.nb_tets() * 4), GL_UNSIGNED_INT, 0
        );
        glUseProgram(0);                
    }

    /**
     * \brief Draws the volumetric cells of a mesh using the shaders.
     */
    
    void draw_cells_with_shaders() {
        glCullFace(GL_FRONT);
        glEnable(GL_CULL_FACE);
        bind_cells_VBO();

        // Draw the tets, using GL_LINE_ADJACENCY (sends
        // 4 vertices per primitive)
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, colors[PRG_TET]);        
        glUseProgram(programs[PRG_TET]);
        for(GEO::index_t c=0; c<M.nb_cells(); ++c) {
            if(M.cell_type(c) == GEO::MESH_TET) {
                glDrawElements(
                    GL_LINES_ADJACENCY, 4, GL_UNSIGNED_INT,
                    (void*)(M.cell_vertices_begin(c) * sizeof(int))
                );
            }
        }

        // Draw the prisms, using GL_TRIANGLES_ADJACENCY (sends
        // 6 vertices per primitive)
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, colors[PRG_PRISM]);
        glUseProgram(programs[PRG_PRISM]);
        for(GEO::index_t c=0; c<M.nb_cells(); ++c) {
            if(M.cell_type(c) == GEO::MESH_PRISM) {
                glDrawElements(
                    GL_TRIANGLES_ADJACENCY, 6, GL_UNSIGNED_INT,
                    (void*)(M.cell_vertices_begin(c) * sizeof(int))
                );
            }
        }

        // Draw the hexes and pyramids using a tesselation shader
        // to lookup the vertices. No standard OpenGL primitive has
        // 8 or 5 vertices, but GL_PATCH (used by tesselation shader)
        // has a configurable number of vertices !!

        if(has_tessellation_shaders) {
            static float levels[4] = {1.0, 1.0, 0.0, 0.0};
            glPatchParameterfv(GL_PATCH_DEFAULT_OUTER_LEVEL, levels);

            glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, colors[PRG_HEX]);                    
            glPatchParameteri(GL_PATCH_VERTICES,8);            
            glUseProgram(programs[PRG_HEX]);
            for(GEO::index_t c=0; c<M.nb_cells(); ++c) {
                if(M.cell_type(c) == GEO::MESH_HEX) {
                    glDrawElements(
                        GL_PATCHES, 8, GL_UNSIGNED_INT,
                        (void*)(M.cell_vertices_begin(c) * sizeof(int))
                    );
                }
            }

            glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, colors[PRG_PYRAMID]);                                
            glPatchParameteri(GL_PATCH_VERTICES,5);            
            glUseProgram(programs[PRG_PYRAMID]);
            for(GEO::index_t c=0; c<M.nb_cells(); ++c) {
                if(M.cell_type(c) == GEO::MESH_PYRAMID) {
                    glDrawElements(
                        GL_PATCHES, 5, GL_UNSIGNED_INT,
                        (void*)(M.cell_vertices_begin(c) * sizeof(int))
                    );
                }
            }
            
        } else {

            // If tessellation shaders are not available,
            // use GL_POINT primitives with 8 vec3's for hexes
            // and 5 vec3's for pyramids.
            
            // Draw the hexes, using GL_POINTS
            // with 8 generic attributes (unfortunately,
            // there is no standard OpenGL primitive with 8 vertices).
            glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, colors[PRG_HEX]);                                            
            glUseProgram(programs[PRG_HEX]);
            glBegin(GL_POINTS);
            for(GEO::index_t c=0; c<M.nb_cells(); ++c) {
                if(M.cell_type(c) == GEO::MESH_HEX) {
                    for(GEO::index_t i=1; i<8; ++i) {
                        glVertexAttrib3fv(
                            i, M.vertex_ptr(M.cell_vertex_index(c,i))
                        );
                    }
                    glVertexAttrib3fv(
                        0, M.vertex_ptr(M.cell_vertex_index(c,0))
                    );
                }
            }
            glEnd();

            // Draw the pyramids, using GL_POINTS
            // with 5 generic attributes (unfortunately,
            // there is no standard OpenGL primitive with 5 vertices).
            glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, colors[PRG_PYRAMID]);                                                        
            glUseProgram(programs[PRG_PYRAMID]);
            glBegin(GL_POINTS);
            for(GEO::index_t c=0; c<M.nb_cells(); ++c) {
                if(M.cell_type(c) == GEO::MESH_PYRAMID) {
                    for(GEO::index_t i=1; i<5; ++i) {
                        glVertexAttrib3fv(
                            i, M.vertex_ptr(M.cell_vertex_index(c,i))
                        );
                    }
                    glVertexAttrib3fv(
                        0, M.vertex_ptr(M.cell_vertex_index(c,0))
                    );
                }
            }
            glEnd();
        }

        glUseProgram(0);
        glDisable(GL_CULL_FACE);
    }

    /**
     * \brief Sends the normal to a triangle to OpenGL
     * \param[in] M a const reference to the mesh
     * \param[in] pi a pointer to the three float coordinates 
     *  of the first vertex
     * \param[in] pj a pointer to the three float coordinates 
     *  of the second vertex
     * \param[in] pk a pointer to the three float coordinates 
     *  of the third vertex
     */
    void glTriangleNormal(const float* pi, const float* pj, const float* pk) {
        float N[3];
        N[0] = -(pi[1]-pj[1])*(pk[2]-pj[2]) + (pi[2]-pj[2])*(pk[1]-pj[1]);
        N[1] = -(pi[2]-pj[2])*(pk[0]-pj[0]) + (pi[0]-pj[0])*(pk[2]-pj[2]);
        N[2] = -(pi[0]-pj[0])*(pk[1]-pj[1]) + (pi[1]-pj[1])*(pk[0]-pj[0]);
        glNormal3fv(N);
    }

    
    /**
     * \brief Sends the normal to a triangle to OpenGL
     * \param[in] M a const reference to the mesh
     * \param[in] i index of the first vertex of the triangle
     * \param[in] j index of the second vertex of the triangle
     * \param[in] k index of the third vertex of the triangle
     */
    void glMeshTriangleNormal(
        const GEO::SinglePrecisionMesh& M,
        GEO::index_t i, GEO::index_t j, GEO::index_t k
    ) {
        const float* pi = M.vertex_ptr(i);
        const float* pj = M.vertex_ptr(j);
        const float* pk = M.vertex_ptr(k);
        glTriangleNormal(pi,pj,pk);
    }

    /**
     * \brief Draws the cells of a hybrid mesh.
     */
    void draw_cells() {

        static bool cell_type_has_triangles[GEO::MESH_NB_CELL_TYPES] = {
            true,  // tets
            false, // hexes
            true,  // prisms
            true,  // pyramids
            true   // connectors
        };

        static bool cell_type_has_quads[GEO::MESH_NB_CELL_TYPES] = {
            false,  // tets
            true, // hexes
            true,  // prisms
            true,  // pyramids
            true   // connectors
        };

        glCullFace(GL_FRONT);
        glEnable(GL_CULL_FACE);
        
        // Try using loaded vertex array with glDrawElements() 
        // Try using a shader, and activating by-cells clipping
        //   (we may need a shader per cell type, or a shader per
        //    facet type)

        glBegin(GL_TRIANGLES);
        for(GEO::index_t c=0; c<M.nb_cells(); ++c) {
            GEO::MeshCellType type = M.cell_type(c);
            if(!cell_type_has_triangles[type]) {
                continue;
            }
            for(GEO::index_t f=0; f<M.cell_nb_facets(c); ++f) {
                if(M.cell_facet_nb_vertices(c,f) == 3) {
                    GEO::index_t i = M.cell_facet_vertex_index(c,f,0);
                    GEO::index_t j = M.cell_facet_vertex_index(c,f,1);
                    GEO::index_t k = M.cell_facet_vertex_index(c,f,2);
                    glMeshTriangleNormal(M,i,j,k);
                    glVertex3fv(M.vertex_ptr(i));
                    glVertex3fv(M.vertex_ptr(j));
                    glVertex3fv(M.vertex_ptr(k));                    
                }
            }
        }
        glEnd();
        
        glBegin(GL_QUADS);
        for(GEO::index_t c=0; c<M.nb_cells(); ++c) {
            GEO::MeshCellType type = M.cell_type(c);            
            if(!cell_type_has_quads[type]) {
                continue;
            }
            for(GEO::index_t f=0; f<M.cell_nb_facets(c); ++f) {
                if(M.cell_facet_nb_vertices(c,f) == 4) {
                    GEO::index_t i = M.cell_facet_vertex_index(c,f,0);
                    GEO::index_t j = M.cell_facet_vertex_index(c,f,1);
                    GEO::index_t k = M.cell_facet_vertex_index(c,f,2);
                    GEO::index_t l = M.cell_facet_vertex_index(c,f,3);
                    glMeshTriangleNormal(M,i,j,k);
                    glVertex3fv(M.vertex_ptr(i));
                    glVertex3fv(M.vertex_ptr(j));
                    glVertex3fv(M.vertex_ptr(k));
                    glVertex3fv(M.vertex_ptr(l));
                }
            }
        }
        glEnd();

        glDisable(GL_CULL_FACE);        
    }

    /**
     * \brief Draws a polygon mesh using VertexBufferObjects.
     *  This function works for meshes that have only triangles
     * and quads.
     */
    void draw_triangles_and_quads_VBOs() {
        if(M.nb_facets() == 0) {
            return;
        }
        create_VBOs_if_needed();

        glUseProgram(programs[PRG_TRI]);        
        for(unsigned int f = 0; f < M.nb_facets(); f++) {
            unsigned int b = M.facet_begin(f);
            unsigned int e = M.facet_end(f);
            // Note: the fourth argument (void*)(b*sizeof(int))
            // is relative to the address of the bound
            // VBO.
            if(e-b == 3) {
                glDrawElements(
                    GL_TRIANGLES, GLsizei(e - b),
                    GL_UNSIGNED_INT, (void*) (b * sizeof(int))
                );
            }
        }

        glUseProgram(programs[PRG_QUAD]);
        for(unsigned int f = 0; f < M.nb_facets(); f++) {
            unsigned int b = M.facet_begin(f);
            unsigned int e = M.facet_end(f);
            // Note: the fourth argument (void*)(b*sizeof(int))
            // is relative to the address of the bound
            // VBO.
            if(e-b == 4) {
                glDrawElements(
                    GL_LINES_ADJACENCY, GLsizei(e - b),
                    GL_UNSIGNED_INT, (void*) (b * sizeof(int))
                );
            }
        }
        glUseProgram(0);
    }
    
    /**
     * \brief Draws a polygon mesh using VertexBufferObjects.
     *
     * \note This function needs optimization (for now, it issues
     * one OpenGL call per polygon, there is probably a means
     * of avoiding that...).
     */
    void draw_polygons_VBOs() {
        if(M.nb_facets() == 0) {
            return;
        }
        create_VBOs_if_needed();
        for(unsigned int f = 0; f < M.nb_facets(); f++) {
            unsigned int b = M.facet_begin(f);
            unsigned int e = M.facet_end(f);
            // Note: the fourth argument (void*)(b*sizeof(int))
            // is relative to the address of the bound
            // VBO.
            glDrawElements(
                GL_POLYGON, GLsizei(e - b),
                GL_UNSIGNED_INT, (void*) (b * sizeof(int))
            );
        }
    }

    void draw_surface_OTM();
    
    /**
     * \brief Draws a surfacic mesh.
     * \details This function is optimized for triangle meshes, and
     *   much much slower for polygon meshes.
     */
    void draw_surface() {
        if(M.nb_facets() == 0) {
            return;
        }
        if(OTM_mode) {
            draw_surface_OTM();
            return;
        }
        bind_facets_VBO();
        if(M.is_triangulated()) {
            draw_triangles_VBOs();
        } else if(!plain_mode && triangles_and_quads) {
            draw_triangles_and_quads_VBOs();
        } else {
            draw_polygons_VBOs();
        }
    }

    /**
     * \brief Draws an optimal transport mesh animation.
     * \details The mesh is in 6d, the first three coordinates
     *  correspond to vertex location at time t=0; and the last
     *  three coordinates correspond to vertex location at time 
     *  t=1.
     */
    void draw_surface_OTM() {
        geo_assert(M.is_triangulated());

        GLint polymode[2];
        glGetIntegerv(GL_POLYGON_MODE, polymode);
        bool surface_mode = (polymode[0] == GL_FILL);
        
        if(plain_mode && surface_mode) {
            if(lighting) {
                glEnable(GL_LIGHTING);
                glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
            }
        } else {
            glUseProgram(programs[PRG_TRI]);
        }
        
        float p[3][3];
        glBegin(GL_TRIANGLES);
        for(GEO::index_t t=0; t < M.nb_facets(); ++t) {
            for(GEO::index_t i=0; i<3; ++i) {
                const float* p_t0 = M.corner_vertex_ptr(M.facet_begin(t)+i);
                const float* p_t1 = p_t0+3;
                for(GEO::coord_index_t c=0; c<3; ++c) {
                    p[i][c] = OTM_time * p_t1[c] + (1.0f - OTM_time) * p_t0[c];
                }
            }
            if(plain_mode) {
                glTriangleNormal(p[2],p[1],p[0]);
            }
            glVertex3fv(p[2]);
            glVertex3fv(p[1]);
            glVertex3fv(p[0]);
        }
        glEnd();

        if(plain_mode && surface_mode) {
            if(lighting) {
                glDisable(GL_LIGHTING);
                glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);
            }
        } else {
            glUseProgram(0);
        }
    }
    
    /**
     * \brief Draws the borders of a surface, and the borders of
     *   the facet regions if present.
     * \details This function is optimized for triangle meshes, and
     *   much much slower for polygon meshes.
     */
    void draw_surface_borders() {
        if(M.nb_facets() == 0) {
            return;
        }
        glDisable(GL_LIGHTING);
        glLineWidth(3);
        if(white_bg) {
            glColor3f(0.0, 0.0, 0.0);
        } else {
            glColor3f(1.0, 1.0, 1.0);
        }
        glBegin(GL_LINES);
        for(GEO::index_t f = 0; f < M.nb_facets(); f++) {
            for(GEO::index_t c1 = M.facet_begin(f);
                c1 < M.facet_end(f); ++c1
            ) {
                if(M.corner_adjacent_facet(c1) == -1) {
                    unsigned int c2 = M.next_around_facet(f, c1);
                    glVertex3fv(M.vertex_ptr(M.corner_vertex_index(c1)));
                    glVertex3fv(M.vertex_ptr(M.corner_vertex_index(c2)));
                }
            }
        }
        glEnd();

        if(!M.has_attribute(GEO::MESH_FACET_REGION)) {
            return;
        }

        glLineWidth(2);
        glBegin(GL_LINES);
        for(GEO::index_t f = 0; f < M.nb_facets(); f++) {
            GEO::signed_index_t f_rgn = M.facet_region(f);
            for(GEO::index_t c1 = M.facet_begin(f);
                c1 < M.facet_end(f); ++c1
            ) {
                GEO::signed_index_t adj_f = M.corner_adjacent_facet(c1);
                if(
                    adj_f != -1 &&
                    M.facet_region(GEO::index_t(adj_f)) != f_rgn
                ) {
                    unsigned int c2 = M.next_around_facet(f, c1);
                    glVertex3fv(M.vertex_ptr(M.corner_vertex_index(c1)));
                    glVertex3fv(M.vertex_ptr(M.corner_vertex_index(c2)));
                }
            }
        }
        glEnd();
    }

    void draw_volume_OTM();
    
    /**
     * \brief Draws a volumetric mesh.
     */
    void draw_volume() {
        if(M.nb_cells() == 0) {
            return;
        }
        if(OTM_mode) {
            draw_volume_OTM();
            return;
        }
        if(plain_mode) {
            glEnable(GL_LIGHTING);
            draw_cells();
            glDisable(GL_LIGHTING);
        } else {
            glCullFace(GL_FRONT);
            glEnable(GL_CULL_FACE);
            if(M.is_tetrahedralized()) {
                draw_tets_with_shaders();
            } else {
                draw_cells_with_shaders();                
            }
            glDisable(GL_CULL_FACE);
        }
    }


    /**
     * \brief Draws an optimal transport mesh animation.
     * \details The mesh is in 6d, the first three coordinates
     *  correspond to vertex location at time t=0; and the last
     *  three coordinates correspond to vertex location at time 
     *  t=1.
     */
    void draw_volume_OTM() {
        geo_assert(M.is_tetrahedralized());
        
        if(plain_mode) {
        } else {
            
            glUseProgram(programs[PRG_TET]);
            
            float p[4][3];
            glBegin(GL_LINES_ADJACENCY);
            for(GEO::index_t t=0; t < M.nb_cells(); ++t) {
                for(GEO::index_t i=0; i<4; ++i) {
                    const float* p_t0 = M.vertex_ptr(M.cell_vertex_index(t,i));
                    const float* p_t1 = p_t0+3;
                    for(GEO::coord_index_t c=0; c<3; ++c) {
                        p[i][c] =
                            OTM_time * p_t1[c] + (1.0f - OTM_time) * p_t0[c];
                    }
                }
                glVertex3fv(p[0]);
                glVertex3fv(p[1]);
                glVertex3fv(p[2]);
                glVertex3fv(p[3]);            
            }
            glEnd();
            glUseProgram(0);
        }
    }

    
    /**
     * \brief Draws all the vertices of the mesh from the Vertex Buffer Object.
     */
    void draw_points_VBO() {
        create_VBOs_if_needed();
        glDrawArrays(GL_POINTS, 0, GLsizei(M.nb_vertices()));
    }

    /**
     * \brief Draws all the vertices of the mesh.
     */
    void draw_points() {
        glDisable(GL_LIGHTING);
        glPointSize(15);
        glEnable(GL_POINT_SPRITE);
        glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
        glColor3f(0.0f, 1.0f, 0.0f);
        glUseProgram(programs[PRG_POINTS]);
        draw_points_VBO();
        glUseProgram(0);
    }

    /**
     * \brief Draws the current mesh according to current rendering mode.
     * \details Specifed as glut_viewer_set_display_func() callback.
     */
    void display() {
        
        if(show_borders) {
            draw_surface_borders();
        }

        GLfloat shininess = 20.0f;
        glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, &shininess);
        if(show_colors) {
            if(M.nb_cells() == 0) {
                static float diff_front[4] = {0.5f, 0.75f, 1.0f, 1.0f};
                static float diff_back[4] = {1.0f, 0.0f, 0.0f, 1.0f};
                glMaterialfv(GL_FRONT, GL_DIFFUSE, diff_front);
                glMaterialfv(GL_BACK, GL_DIFFUSE, diff_back);
                static float spec[4] = {0.6f, 0.6f, 0.6f, 1.0f};
                glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spec);
            } else {
                static float diff_back[4] = {1.0f, 1.0f, 0.0f, 0.7f};
                static float diff_front[4] = {0.7f, 0.0f, 0.0f, 1.0f};
                glMaterialfv(GL_FRONT, GL_DIFFUSE, diff_front);
                glMaterialfv(GL_BACK, GL_DIFFUSE, diff_back);
                static float spec[4] = {0.6f, 0.6f, 0.6f, 1.0f};
                glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spec);
            }
        } else {
            static float spec[4] = {0.6f, 0.6f, 0.6f, 1.0f};
            glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spec);
            if(white_bg) {
                static float diff[4] = {0.9f, 0.9f, 0.9f, 1.0f};
                glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diff);
            } else {
                static float diff[4] = {0.1f, 0.1f, 0.1f, 1.0f};
                glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diff);
            }
        }

        if(show_surface) {
            glColor3f(1.0f, 1.0f, 1.0f);
            draw_surface();
        }

        if(show_volume) {
            static float tet_color[4] = {1.0f, 1.0f, 1.0f, 1.0f};
            glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, tet_color);
            draw_volume();
        }

        // If the surface is triangulated and
        //   and shaders are used (!plain_mode), 
        //   then the mesh is drawn by the fragment shader (that
        //  changes the color of the fragments near
        //  the edges of the triangles),
        // If the surface has polygons, then the mesh
        //  is drawn "the standard way" below:
        if(
            show_mesh &&
            (plain_mode || (!M.is_triangulated() && !triangles_and_quads))
        ) {
            glDisable(GL_LIGHTING);
            glLineWidth(1);
            if(white_bg) {
                glColor3f(0.0, 0.0, 0.0);
            } else {
                glColor3f(1.0, 1.0, 1.0);
            }
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            draw_surface();
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }


        if(plain_mode && show_mesh && show_volume && !M.is_tetrahedralized()) {
            glDisable(GL_LIGHTING);
            glLineWidth(1);
            if(white_bg) {
                glColor3f(0.0, 0.0, 0.0);
            } else {
                glColor3f(1.0, 1.0, 1.0);
            }
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            draw_cells();
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
        
        if(show_vertices && !OTM_mode) {
            draw_points();
        }
    }

    /**
     * \brief Gets the bounding box of a mesh animation.
     * \details The mesh animation is stored as a mesh with
     *  6d coordinates, that correspond to the geometric location
     *  at the vertices at time 0 and at time 1.
     * \param[in] M the mesh
     * \param[out] xyzmin a pointer to the three minimum coordinates
     * \param[out] xyzmax a pointer to the three maximum coordinates
     */
    void get_bbox6(
        const GEO::SinglePrecisionMesh& M, double* xyzmin, double* xyzmax
    ) {
        geo_assert(M.dimension() >= 6);
        for(GEO::index_t c = 0; c < 3; c++) {
            xyzmin[c] = GEO::Numeric::max_float64();
            xyzmax[c] = GEO::Numeric::min_float64();
        }
        for(GEO::index_t v = 0; v < M.nb_vertices(); v++) {
            const float* p = M.vertex_ptr(v);
            for(GEO::coord_index_t c = 0; c < 3; c++) {
                xyzmin[c] = GEO::geo_min(xyzmin[c], double(p[c]));
                xyzmin[c] = GEO::geo_min(xyzmin[c], double(p[c+3]));
                xyzmax[c] = GEO::geo_max(xyzmax[c], double(p[c]));
                xyzmax[c] = GEO::geo_max(xyzmax[c], double(p[c+3]));
            }
        }
    }

    
    /**
     * \brief Loads a mesh from a file.
     */
    void load_mesh(const std::string& filename) {

        delete_VBOs_if_needed();
        GEO::MeshIOFlags flags;
        flags.set_attribute(GEO::MESH_FACET_REGION);
        flags.set_element(GEO::MESH_CELLS);
        if(!GEO::mesh_load(filename, M, flags)) {
            // We need to specify dimension, else
            // we got an invalid mesh that triggers
            // an assertion fail later.
            M.set_dimension(3);
            return;
        }
        if(M.nb_facets() == 0 && M.nb_cells() > 0) {
            M.compute_cells_boundaries();
        }

        if(
            GEO::FileSystem::extension(filename) == "obj6" ||
            GEO::FileSystem::extension(filename) == "tet6"
        ) {
            GEO::Logger::out("Vorpaview")
                << "Displaying optimal transport" << std::endl;
            OTM_mode = true;
            double xyzmin[3];
            double xyzmax[3];
            get_bbox6(M, xyzmin, xyzmax);
            glut_viewer_set_region_of_interest(
                float(xyzmin[0]), float(xyzmin[1]), float(xyzmin[2]),
                float(xyzmax[0]), float(xyzmax[1]), float(xyzmax[2])
            );
        } else {
            M.set_dimension(3);
            double xyzmin[3];
            double xyzmax[3];
            GEO::get_bbox(M, xyzmin, xyzmax);
            glut_viewer_set_region_of_interest(
                float(xyzmin[0]), float(xyzmin[1]), float(xyzmin[2]),
                float(xyzmax[0]), float(xyzmax[1]), float(xyzmax[2])
            );
        }
        
        triangles_and_quads = true;
        for(GEO::index_t f=0; f<M.nb_facets(); ++f) {
            if(M.facet_size(f) != 3 && M.facet_size(f) != 4) {
                triangles_and_quads = false;
                break;
            }
        }
    }

    /**
     * \brief Loads a mesh from a file icon dropped into the window.
     * \details Specifed as glut_viewer_set_drag_drop_func() callback.
     */
    void dropped_file_cb(char* filename) {
        load_mesh(std::string(filename));
    }

    /**
     * \brief Inverts the normals of a mesh.
     * \details In color mode, this swaps the red and the blue sides.
     */
    void invert_normals() {
        delete_VBOs_if_needed();
        for(GEO::index_t f = 0; f < M.nb_facets(); ++f) {
            GEOGen::MeshMutator<GEO::SinglePrecisionMesh>::flip_facet(M, f);
        }
    }

    void toggle_colored_cells() {
        colored_cells = !colored_cells;
        if(colored_cells) {
            set_color(PRG_TET,     1.0f, 0.0f, 0.0f);
            set_color(PRG_HEX,     1.0f, 1.0f, 0.0f);
            set_color(PRG_PRISM,   0.0f, 1.0f, 0.0f);
            set_color(PRG_PYRAMID, 0.0f, 0.0f, 1.0f);            
        } else {
            set_color(PRG_TET,     0.9f, 0.9f, 0.9f);
            set_color(PRG_HEX,     0.9f, 0.9f, 0.9f);
            set_color(PRG_PRISM,   0.9f, 0.9f, 0.9f);
            set_color(PRG_PYRAMID, 0.9f, 0.9f, 0.9f);            
        }
    }
}

int main(int argc, char** argv) {

    GEO::initialize();
    GEO::Logger::instance()->set_quiet(false);

    GEO::CmdLine::import_arg_group("standard");
    GEO::CmdLine::import_arg_group("algo");
    GEO::CmdLine::declare_arg("full_screen",false,"full screen mode");
    GEO::CmdLine::declare_arg("plain",false,"plain mode (no shaders)");
    GEO::CmdLine::declare_arg(
        "tessellation",true,"use tessellation shaders (if possible)"
    );
    
    std::vector<std::string> filenames;
    if(!GEO::CmdLine::parse(argc, argv, filenames, "<filename>")) {
        return 1;
    }

    if(filenames.size() == 1) {
        load_mesh(filenames[0]);
    } else {
        M.set_dimension(3);
    }

    if(M.nb_cells() != 0) {
        show_volume = GL_TRUE;
        show_mesh = GL_TRUE;
    } else if(M.nb_facets() == 0) {
        show_vertices = GL_TRUE;
    }

    glut_viewer_set_window_title(
        (char*) "[ \\V (O |R |P /A |L |I |N |E ]-[ viewer ]"
    );
    glut_viewer_set_init_func(init);
    glut_viewer_set_display_func(display);
    glut_viewer_set_drag_drop_func(dropped_file_cb);
    glut_viewer_add_toggle('V', &show_volume, "volume");
    glut_viewer_add_toggle('p', &show_vertices, "vertices");
    glut_viewer_add_toggle('S', &show_surface, "surface");
    glut_viewer_add_key_func('L', toggle_lighting, "toggle lighting");
    glut_viewer_add_key_func('n', invert_normals, "invert normals");
    glut_viewer_add_key_func('x', dec_shrink, "unshrink cells");
    glut_viewer_add_key_func('w', inc_shrink, "shrink cells");
    glut_viewer_add_key_func('C', toggle_colored_cells, "toggle colored cells");        
    
    if(GEO::CmdLine::get_arg_bool("full_screen")) {
       glut_viewer_enable(GLUT_VIEWER_FULL_SCREEN);
    }
      
    glut_viewer_main_loop(argc, argv);

    // Note: when 'q' is pressed, exit() is called
    // because there is no simple way of exiting from
    // glut's event loop, therefore this line is not
    // reached and memory is not properly freed on exit.
    // TODO: add a function in freeglut to exit event loop.

    return 0;
}

