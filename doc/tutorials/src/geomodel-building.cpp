/*
 * Copyright (c) 2012-2017, Association Scientifique pour la Geologie et ses
 * Applications (ASGA). All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL ASGA BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *     http://www.ring-team.org
 *
 *     RING Project
 *     Ecole Nationale Superieure de Geologie - GeoRessources
 *     2 Rue du Doyen Marcel Roubault - TSA 70605
 *     54518 VANDOEUVRE-LES-NANCY
 *     FRANCE
 */

#include <ringmesh/basic/common.h>

#include <geogram/basic/command_line.h>
#include <geogram/basic/stopwatch.h>

#include <ringmesh/basic/command_line.h>
#include <ringmesh/geomodel/geomodel.h>
#include <ringmesh/geomodel/geomodel_api.h>
#include <ringmesh/geomodel/geomodel_builder.h>
#include <ringmesh/io/io.h>

/*!
 * @author Antoine Mazuyer
 * @author Francois Bonneau
 * @author Pierre Anquez
 */

/*!
 * Purpose of this main is to show the methods
 * to be used to build a GeoModel from scratch.
 *
 *
 *  !!!!!!!!!!!!!!!!!! WARNING !!!!!!!!!!!!!!!!!!!!!!!!!
 *  !!   This is the brute force way to build a       !!
 *  !!   GeoModel, as you will see, the code is       !!
 *  !!   pretty heavy. Purpose of this main is to     !!
 *  !!   present the method to build a GeoModel, and  !!
 *  !!   to understand the relations between each     !!
 *  !!   GeoModelEntities. One can implement loop     !!
 *  !!   to automate these actions by reading         !!
 *  !!   a file containing all the informations.      !!
 *  !!   That is what is done for instance, when      !!
 *  !!   RINGMesh reads a .ml file generated by       !!
 *  !!   Gocad                                        !!
 *  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 *
 * We will take the example of a very simple model with one horizon
 *
 *        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
 *       @                                                 @@
 *      @                                                 @ @
 *     @                                                 @  @
 *    @                                                 @   @
 *   @                                                 @    @
 *  @                                                 @    %@
 * @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@    % @
 * @                                                 @   %  @
 * @                                                 @  %   @
 * @                                                 @ %    @
 * @                                                 @%    @
 * @%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%@    @
 * @                                                 @   @
 * @                                                 @  @
 * @                                                 @ @
 * @                                                 @@
 * @%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%@
 *
 * @ : the boundaries
 * % : the horizons
 *
 *
 */

int main()
{
    using namespace RINGMesh;

    try
    {
        // This line stands for the initialization
        // of Geogram and the factories of RINGMesh
        // IT IS MANDATORY
        default_configure();

        // Say Hello
        print_header_information();
        Logger::div( "RINGMesh Training" );
        Logger::out( "", "Welcome to the training of RINGMesh !" );

        // Measure the time between the GEO::Stopwatch creation and its
        // destruction.
        GEO::Stopwatch total( "Total time" );

        // We instantiate the class GeoModel
        GeoModel3D geomodel;

        // To build the model, we have to use the class
        // GeoModelBuilder, which is a safety for the user
        // Indeed, the GeoModel can't be directly modified,
        // It has to be done using the GeoModelBuilder
        GeoModelBuilder3D builder( geomodel );

        //#############################
        // Declaration of the Entities#
        //#############################

        // For the next section, read the documentation to understand
        // the concept of Geological Entity and Mesh Entities
        // Let's to a sum up of the GeoModel we try to build:
        // For the Geological Entities (handle by the class
        // GeoModelGeologicalEntity):
        // 16 Contacts
        index_t nb_contacts = 16;
        // 1 horizons + 6 boundaries = 7 Interfaces
        index_t nb_interfaces = 7;
        // 2 Layers
        index_t nb_layers = 2;

        // For the Meshed Entities, (handle by the class GeoModelMeshEntity)
        // 12 Corners
        index_t nb_corners = 12;
        // 20 Lines
        index_t nb_lines = 20;
        // 11 Surfaces
        index_t nb_surfaces = 11;
        // 2  Regions
        index_t nb_regions = 2;

        // We first create the GeoModelGeoglogicalEntity
        // Create the contacts
        for( index_t contact = 0; contact < nb_contacts; contact++ )
        {
            builder.geology.create_geological_entity(
                Contact3D::type_name_static() );
            // the static method type_name_static() is available for each
            // GeoModelEntity. It returns an EntityType which is a string
            // corresponding to the Type of the entity.
        }

        // Create the Interfaces
        for( index_t interface_itr = 0; interface_itr < nb_interfaces;
             interface_itr++ )
        {
            builder.geology.create_geological_entity(
                Interface3D::type_name_static() );
        }

        // Create the Layers
        for( index_t layer = 0; layer < nb_layers; layer++ )
        {
            builder.geology.create_geological_entity(
                Layer3D::type_name_static() );
        }

        // Then we create the GeoModelMeshEntity
        // Create the Corners
        for( index_t corner = 0; corner < nb_corners; corner++ )
        {
            builder.topology.create_mesh_entity( Corner3D::type_name_static() );
        }

        // Create the Lines
        for( index_t lines = 0; lines < nb_lines; lines++ )
        {
            builder.topology.create_mesh_entity( Line3D::type_name_static() );
        }

        // Create the Surfaces
        for( index_t surface = 0; surface < nb_surfaces; surface++ )
        {
            builder.topology.create_mesh_entity(
                Surface3D::type_name_static() );
        }

        // Create the Regions
        for( index_t region = 0; region < nb_regions; region++ )
        {
            builder.topology.create_mesh_entity( Region3D::type_name_static() );
        }

        //#############################
        // Setting the Geometry       #
        //#############################

        // We declare the coordinates of the corners. We arrange the corner in a
        // table
        vec3 corners_table[12];
        corners_table[0] = vec3( 0, 0, 0 );
        corners_table[1] = vec3( 5, 0, 0 );
        corners_table[2] = vec3( 5, 5, 0 );
        corners_table[3] = vec3( 0, 5, 0 );
        corners_table[4] = vec3( 0, 0, -2 );
        corners_table[5] = vec3( 5, 0, -2 );
        corners_table[6] = vec3( 5, 5, -2 );
        corners_table[7] = vec3( 0, 5, -2 );
        corners_table[8] = vec3( 0, 0, -1 );
        corners_table[9] = vec3( 5, 0, -1 );
        corners_table[10] = vec3( 5, 5, -1 );
        corners_table[11] = vec3( 0, 5, -1 );

        // We associate the coordinates with the corners
        for( index_t corner = 0; corner < nb_corners; corner++ )
        {
            builder.geometry.set_corner( corner, corners_table[corner] );
        }

        // We associate the coordinates with the lines
        // We create a vector cur_coor_line containing the 2 vertices
        // for each line. Of course, you can have more vertices in a Line
        std::vector< vec3 > cur_coor_line( 2 );
        cur_coor_line[0] = corners_table[0];
        cur_coor_line[1] = corners_table[3];
        builder.geometry.set_line( 0, cur_coor_line );

        cur_coor_line[0] = corners_table[2];
        cur_coor_line[1] = corners_table[3];
        builder.geometry.set_line( 1, cur_coor_line );

        cur_coor_line[0] = corners_table[1];
        cur_coor_line[1] = corners_table[2];
        builder.geometry.set_line( 2, cur_coor_line );

        cur_coor_line[0] = corners_table[0];
        cur_coor_line[1] = corners_table[1];
        builder.geometry.set_line( 3, cur_coor_line );

        cur_coor_line[0] = corners_table[8];
        cur_coor_line[1] = corners_table[11];
        builder.geometry.set_line( 4, cur_coor_line );

        cur_coor_line[0] = corners_table[10];
        cur_coor_line[1] = corners_table[11];
        builder.geometry.set_line( 5, cur_coor_line );

        cur_coor_line[0] = corners_table[9];
        cur_coor_line[1] = corners_table[10];
        builder.geometry.set_line( 6, cur_coor_line );

        cur_coor_line[0] = corners_table[8];
        cur_coor_line[1] = corners_table[9];
        builder.geometry.set_line( 7, cur_coor_line );

        cur_coor_line[0] = corners_table[4];
        cur_coor_line[1] = corners_table[7];
        builder.geometry.set_line( 8, cur_coor_line );

        cur_coor_line[0] = corners_table[6];
        cur_coor_line[1] = corners_table[7];
        builder.geometry.set_line( 9, cur_coor_line );

        cur_coor_line[0] = corners_table[5];
        cur_coor_line[1] = corners_table[6];
        builder.geometry.set_line( 10, cur_coor_line );

        cur_coor_line[0] = corners_table[4];
        cur_coor_line[1] = corners_table[5];
        builder.geometry.set_line( 11, cur_coor_line );

        cur_coor_line[0] = corners_table[4];
        cur_coor_line[1] = corners_table[8];
        builder.geometry.set_line( 12, cur_coor_line );

        cur_coor_line[0] = corners_table[0];
        cur_coor_line[1] = corners_table[8];
        builder.geometry.set_line( 13, cur_coor_line );

        cur_coor_line[0] = corners_table[7];
        cur_coor_line[1] = corners_table[11];
        builder.geometry.set_line( 14, cur_coor_line );

        cur_coor_line[0] = corners_table[3];
        cur_coor_line[1] = corners_table[11];
        builder.geometry.set_line( 15, cur_coor_line );

        cur_coor_line[0] = corners_table[5];
        cur_coor_line[1] = corners_table[9];
        builder.geometry.set_line( 16, cur_coor_line );

        cur_coor_line[0] = corners_table[1];
        cur_coor_line[1] = corners_table[9];
        builder.geometry.set_line( 17, cur_coor_line );

        cur_coor_line[0] = corners_table[6];
        cur_coor_line[1] = corners_table[10];
        builder.geometry.set_line( 18, cur_coor_line );

        cur_coor_line[0] = corners_table[2];
        cur_coor_line[1] = corners_table[10];
        builder.geometry.set_line( 19, cur_coor_line );

        // We associate the coordinates with the Surfaces
        // We create a vector cur_coor_surface containing 4 vertices.
        // These 4 vertices delimits each surface so each surface
        // will contain one unique quad as a facet.
        // You can defined a more complicated mesh (for example a
        // triangular mesh) with these methods.
        std::vector< index_t > facet( 4, 0 );
        facet[0] = 0;
        facet[1] = 1;
        facet[2] = 2;
        facet[3] = 3;
        std::vector< index_t > facet_ptr( 2 );
        facet_ptr[0] = 0;
        facet_ptr[1] = 4;
        std::vector< vec3 > cur_coor_surface( 4 );
        cur_coor_surface[0] = corners_table[0];
        cur_coor_surface[1] = corners_table[1];
        cur_coor_surface[2] = corners_table[2];
        cur_coor_surface[3] = corners_table[3];
        builder.geometry.set_surface_geometry(
            0, cur_coor_surface, facet, facet_ptr );

        cur_coor_surface[0] = corners_table[8];
        cur_coor_surface[1] = corners_table[9];
        cur_coor_surface[2] = corners_table[10];
        cur_coor_surface[3] = corners_table[11];
        builder.geometry.set_surface_geometry(
            1, cur_coor_surface, facet, facet_ptr );

        cur_coor_surface[0] = corners_table[4];
        cur_coor_surface[1] = corners_table[5];
        cur_coor_surface[2] = corners_table[6];
        cur_coor_surface[3] = corners_table[7];
        builder.geometry.set_surface_geometry(
            2, cur_coor_surface, facet, facet_ptr );

        cur_coor_surface[0] = corners_table[8];
        cur_coor_surface[1] = corners_table[4];
        cur_coor_surface[2] = corners_table[7];
        cur_coor_surface[3] = corners_table[11];
        builder.geometry.set_surface_geometry(
            3, cur_coor_surface, facet, facet_ptr );

        cur_coor_surface[0] = corners_table[0];
        cur_coor_surface[1] = corners_table[8];
        cur_coor_surface[2] = corners_table[11];
        cur_coor_surface[3] = corners_table[3];
        builder.geometry.set_surface_geometry(
            4, cur_coor_surface, facet, facet_ptr );

        cur_coor_surface[0] = corners_table[5];
        cur_coor_surface[1] = corners_table[6];
        cur_coor_surface[2] = corners_table[10];
        cur_coor_surface[3] = corners_table[9];
        builder.geometry.set_surface_geometry(
            5, cur_coor_surface, facet, facet_ptr );

        cur_coor_surface[0] = corners_table[9];
        cur_coor_surface[1] = corners_table[10];
        cur_coor_surface[2] = corners_table[2];
        cur_coor_surface[3] = corners_table[1];
        builder.geometry.set_surface_geometry(
            6, cur_coor_surface, facet, facet_ptr );

        cur_coor_surface[0] = corners_table[8];
        cur_coor_surface[1] = corners_table[4];
        cur_coor_surface[2] = corners_table[5];
        cur_coor_surface[3] = corners_table[9];
        builder.geometry.set_surface_geometry(
            7, cur_coor_surface, facet, facet_ptr );

        cur_coor_surface[0] = corners_table[0];
        cur_coor_surface[1] = corners_table[8];
        cur_coor_surface[2] = corners_table[9];
        cur_coor_surface[3] = corners_table[1];
        builder.geometry.set_surface_geometry(
            8, cur_coor_surface, facet, facet_ptr );

        cur_coor_surface[0] = corners_table[6];
        cur_coor_surface[1] = corners_table[10];
        cur_coor_surface[2] = corners_table[11];
        cur_coor_surface[3] = corners_table[7];
        builder.geometry.set_surface_geometry(
            9, cur_coor_surface, facet, facet_ptr );

        cur_coor_surface[0] = corners_table[10];
        cur_coor_surface[1] = corners_table[2];
        cur_coor_surface[2] = corners_table[3];
        cur_coor_surface[3] = corners_table[11];
        builder.geometry.set_surface_geometry(
            10, cur_coor_surface, facet, facet_ptr );

        //###################################
        // Setting the Boundaries relations #
        //###################################

        // We set the Corners which are incident entities of the lines
        // The add_mesh_entity_boundary_relation method take as first argument
        // the
        // gme_t of the boundary and in second argument
        // the id of the GeoModelMeshentity bounded by the boundary
        // Remember :
        // Lines are bounded by Corners
        // Surfaces are bounded by Lines
        // Region are bounded by Surfaces

        // For corner 0
        // Corner 0 is a boundary of the lines: 0, 3, and 13.
        builder.topology.add_line_corner_boundary_relation( 0, 0 );
        builder.topology.add_line_corner_boundary_relation( 3 , 0 );
        builder.topology.add_line_corner_boundary_relation( 13, 0 );

        // For corner 1
        // Corner 1 is a boundary of the lines: 2, 3, and 17.
        builder.topology.add_line_corner_boundary_relation( 2, 1 );
        builder.topology.add_line_corner_boundary_relation( 3, 1 );
        builder.topology.add_line_corner_boundary_relation( 17, 1 );

        // For corner 2
        // Corner 2 is a boundary of the lines: 1, 2, and 19.
        builder.topology.add_line_corner_boundary_relation( 1, 2 );
        builder.topology.add_line_corner_boundary_relation( 2, 2 );
        builder.topology.add_line_corner_boundary_relation( 19, 2 );

        // For corner 3
        // Corner 3 is a boundary of the lines: 0, 1, and 15.
        builder.topology.add_line_corner_boundary_relation( 0, 3 );
        builder.topology.add_line_corner_boundary_relation( 1, 3 );
        builder.topology.add_line_corner_boundary_relation( 15, 3 );

        // For corner 4
        // Corner 4 is a boundary of the lines: 8, 11, and 12.
        builder.topology.add_line_corner_boundary_relation( 8, 4 );
        builder.topology.add_line_corner_boundary_relation( 11, 4 );
        builder.topology.add_line_corner_boundary_relation( 12, 4 );

        // For corner 5
        // Corner 5 is a boundary of the lines: 10, 11, and 16.
        builder.topology.add_line_corner_boundary_relation( 10, 5 );
        builder.topology.add_line_corner_boundary_relation( 11, 5 );
        builder.topology.add_line_corner_boundary_relation( 16, 5 );

        // For corner 6
        // Corner 6 is a boundary of the lines: 9, 10, and 18.
        builder.topology.add_line_corner_boundary_relation( 9, 6 );
        builder.topology.add_line_corner_boundary_relation( 10, 6 );
        builder.topology.add_line_corner_boundary_relation( 18, 6 );

        // For corner 7
        // Corner 7 is a boundary of the lines: 8, 9, and 14.
        builder.topology.add_line_corner_boundary_relation( 8, 7 );
        builder.topology.add_line_corner_boundary_relation( 9, 7 );
        builder.topology.add_line_corner_boundary_relation( 14, 7 );

        // For corner 8
        // Corner 8 is a boundary of the lines: 4, 7, 12, and 13.
        builder.topology.add_line_corner_boundary_relation( 4, 8 );
        builder.topology.add_line_corner_boundary_relation( 7, 8 );
        builder.topology.add_line_corner_boundary_relation( 12, 8 );
        builder.topology.add_line_corner_boundary_relation( 13, 8 );

        // For corner 9
        // Corner 9 is a boundary of the lines: 6, 7, 16, and 17.
        builder.topology.add_line_corner_boundary_relation( 6, 9 );
        builder.topology.add_line_corner_boundary_relation( 7, 9 );
        builder.topology.add_line_corner_boundary_relation( 16, 9 );
        builder.topology.add_line_corner_boundary_relation( 17, 9 );

        // For corner 10
        // Corner 10 is a boundary of the lines: 5, 6, 18, and 19.
        builder.topology.add_line_corner_boundary_relation( 5, 10 );
        builder.topology.add_line_corner_boundary_relation( 6, 10 );
        builder.topology.add_line_corner_boundary_relation( 18, 10 );
        builder.topology.add_line_corner_boundary_relation( 19, 10 );

        // For corner 11
        // Corner 11 is a boundary of the lines: 4, 5, 14, and 15.
        builder.topology.add_line_corner_boundary_relation( 4, 11 );
        builder.topology.add_line_corner_boundary_relation( 5, 11 );
        builder.topology.add_line_corner_boundary_relation( 14, 11 );
        builder.topology.add_line_corner_boundary_relation( 15, 11 );

        /////////////////////////////////////////////////////////

        // For line 0
        // Line 0 is a boundary of the surfaces: 0 and 4.
        builder.topology.add_surface_line_boundary_relation( 0, 0 );
        builder.topology.add_surface_line_boundary_relation( 4, 0 );

        // For line 1
        // Line 1 is a boundary of the surfaces: 0 and 10.
        builder.topology.add_surface_line_boundary_relation( 0, 1 );
        builder.topology.add_surface_line_boundary_relation( 10, 1 );

        // For line 2
        // Line 2 is a boundary of the surfaces: 0 and 6.
        builder.topology.add_surface_line_boundary_relation( 0, 2 );
        builder.topology.add_surface_line_boundary_relation( 6, 2 );

        // For line 3
        // Line 3 is a boundary of the surfaces: 0 and 8.
        builder.topology.add_surface_line_boundary_relation( 0, 3 );
        builder.topology.add_surface_line_boundary_relation( 8, 3 );

        // For line 4
        // Line 4 is a boundary of the surfaces: 1, 3 and 4.
        builder.topology.add_surface_line_boundary_relation( 1, 4 );
        builder.topology.add_surface_line_boundary_relation( 3, 4 );
        builder.topology.add_surface_line_boundary_relation( 4, 4 );

        // For line 5
        // Line 5 is a boundary of the surfaces: 1, 9 and 10.
        builder.topology.add_surface_line_boundary_relation( 1, 5 );
        builder.topology.add_surface_line_boundary_relation( 9, 5 );
        builder.topology.add_surface_line_boundary_relation( 10, 5 );

        // For line 6
        // Line 6 is a boundary of the surfaces: 1, 5 and 6.
        builder.topology.add_surface_line_boundary_relation( 1, 6 );
        builder.topology.add_surface_line_boundary_relation( 5, 6 );
        builder.topology.add_surface_line_boundary_relation( 6, 6 );

        // For line 7
        // Line 7 is a boundary of the surfaces: 1, 7 and 8.
        builder.topology.add_surface_line_boundary_relation( 1, 7 );
        builder.topology.add_surface_line_boundary_relation( 7, 7 );
        builder.topology.add_surface_line_boundary_relation( 8, 7 );

        // For line 8
        // Line 8 is a boundary of the surfaces: 2 and 3.
        builder.topology.add_surface_line_boundary_relation( 2, 8 );
        builder.topology.add_surface_line_boundary_relation( 3, 8 );

        // For line 9
        // Line 9 is a boundary of the surfaces: 2 and 9.
        builder.topology.add_surface_line_boundary_relation( 2, 9 );
        builder.topology.add_surface_line_boundary_relation( 9, 9 );

        // For line 10
        // Line 10 is a boundary of the surfaces: 2 and 5.
        builder.topology.add_surface_line_boundary_relation( 2, 10 );
        builder.topology.add_surface_line_boundary_relation( 5, 10 );

        // For line 11
        // Line 11 is a boundary of the surfaces: 2 and 7.
        builder.topology.add_surface_line_boundary_relation( 2, 11 );
        builder.topology.add_surface_line_boundary_relation( 7, 11 );

        // For line 12
        // Line 12 is a boundary of the surfaces: 3 and 7.
        builder.topology.add_surface_line_boundary_relation( 3, 12 );
        builder.topology.add_surface_line_boundary_relation( 7, 12 );

        // For line 13
        // Line 13 is a boundary of the surfaces: 4 and 8.
        builder.topology.add_surface_line_boundary_relation( 4, 13 );
        builder.topology.add_surface_line_boundary_relation( 8, 13 );

        // For line 14
        // Line 14 is a boundary of the surfaces: 3 and 9.
        builder.topology.add_surface_line_boundary_relation( 3, 14 );
        builder.topology.add_surface_line_boundary_relation( 9, 14 );

        // For line 15
        // Line 15 is a boundary of the surfaces: 4 and 10.
        builder.topology.add_surface_line_boundary_relation( 4, 15 );
        builder.topology.add_surface_line_boundary_relation( 10, 15 );

        // For line 16
        // Line 16 is a boundary of the surfaces: 5 and 7.
        builder.topology.add_surface_line_boundary_relation( 5, 16 );
        builder.topology.add_surface_line_boundary_relation( 7, 16 );

        // For line 17
        // Line 17 is a boundary of the surfaces: 6 and 8.
        builder.topology.add_surface_line_boundary_relation( 6, 17 );
        builder.topology.add_surface_line_boundary_relation( 8, 17 );

        // For line 18
        // Line 18 is a boundary of the surfaces: 5 and 9.
        builder.topology.add_surface_line_boundary_relation( 5, 18 );
        builder.topology.add_surface_line_boundary_relation( 9, 18 );

        // For line 19
        // Line 19 is a boundary of the surfaces: 6 and 10.
        builder.topology.add_surface_line_boundary_relation( 6, 19 );
        builder.topology.add_surface_line_boundary_relation( 10, 19 );

        /////////////////////////////////////////////////////////

        // For surface 0
        // Surface 0 is a boundary of the region 0.
        builder.topology.add_region_surface_boundary_relation( 0, 0, false );//TODO side ????

        // For surface 1
        // Surface 1 is a boundary of the region 0.
        builder.topology.add_region_surface_boundary_relation( 0, 1, false );//TODO side ????

        // For surface 2
        // Surface 2 is a boundary of the region 1.
        builder.topology.add_region_surface_boundary_relation( 1, 2, false );//TODO side ????

        // For surface 3
        // Surface 3 is a boundary of the region 1.
        builder.topology.add_region_surface_boundary_relation( 1, 3, false );//TODO side ????

        // For surface 4
        // Surface 4 is a boundary of the region 0.
        builder.topology.add_region_surface_boundary_relation( 0, 4, false );//TODO side ????

        // For surface 5
        // Surface 5 is a boundary of the region 1.
        builder.topology.add_region_surface_boundary_relation( 1, 5, false );//TODO side ????

        // For surface 6
        // Surface 6 is a boundary of the region 0.
        builder.topology.add_region_surface_boundary_relation( 0, 6, false );//TODO side ????

        // For surface 7
        // Surface 7 is a boundary of the region 1.
        builder.topology.add_region_surface_boundary_relation( 1, 7, false );//TODO side ????

        // For surface 8
        // Surface 8 is a boundary of the region 0.
        builder.topology.add_region_surface_boundary_relation( 0, 8, false );//TODO side ????

        // For surface 9
        // Surface 9 is a boundary of the region 1.
        builder.topology.add_region_surface_boundary_relation( 1, 9, false );//TODO side ????

        // For surface 10
        // Surface 10 is a boundary of the region 0.
        builder.topology.add_region_surface_boundary_relation( 0, 10, false );//TODO side ????

        //#####################################
        // Setting the parent/child relations #
        //#####################################

        // Remember :
        // Child of a Contact is a Line
        // Child of an Interface is a Surface
        // Child of a Layer is a Region

        // We use the method "add_parent_children_relation"
        // First argument is the parent (ie a GeoModelGeologicalEntity)
        // Second argument is the index of the child (ie a GeoModelMeshEntity)

        // For Contact 0
        builder.geology.add_parent_children_relation(
            gmge_id( Contact3D::type_name_static(), 0 ),
            gmme_id( Line3D::type_name_static(), 0 ) );

        // For Contact 1
        builder.geology.add_parent_children_relation(
            gmge_id( Contact3D::type_name_static(), 1 ),
            gmme_id( Line3D::type_name_static(), 1 ) );

        // For Contact 2
        builder.geology.add_parent_children_relation(
            gmge_id( Contact3D::type_name_static(), 2 ),
            gmme_id( Line3D::type_name_static(), 2 ) );

        // For Contact 3
        builder.geology.add_parent_children_relation(
            gmge_id( Contact3D::type_name_static(), 3 ),
            gmme_id( Line3D::type_name_static(), 3 ) );

        // For Contact 4
        builder.geology.add_parent_children_relation(
            gmge_id( Contact3D::type_name_static(), 4 ),
            gmme_id( Line3D::type_name_static(), 4 ) );

        // For Contact 5
        builder.geology.add_parent_children_relation(
            gmge_id( Contact3D::type_name_static(), 5 ),
            gmme_id( Line3D::type_name_static(), 5 ) );

        // For Contact 6
        builder.geology.add_parent_children_relation(
            gmge_id( Contact3D::type_name_static(), 6 ),
            gmme_id( Line3D::type_name_static(), 6 ) );

        // For Contact 7
        builder.geology.add_parent_children_relation(
            gmge_id( Contact3D::type_name_static(), 7 ),
            gmme_id( Line3D::type_name_static(), 7 ) );

        // For Contact 8
        builder.geology.add_parent_children_relation(
            gmge_id( Contact3D::type_name_static(), 8 ),
            gmme_id( Line3D::type_name_static(), 8 ) );

        // For Contact 9
        builder.geology.add_parent_children_relation(
            gmge_id( Contact3D::type_name_static(), 9 ),
            gmme_id( Line3D::type_name_static(), 9 ) );

        // For Contact 10
        builder.geology.add_parent_children_relation(
            gmge_id( Contact3D::type_name_static(), 10 ),
            gmme_id( Line3D::type_name_static(), 10 ) );

        // For Contact 11
        builder.geology.add_parent_children_relation(
            gmge_id( Contact3D::type_name_static(), 11 ),
            gmme_id( Line3D::type_name_static(), 11 ) );

        // For Contact 12
        builder.geology.add_parent_children_relation(
            gmge_id( Contact3D::type_name_static(), 12 ),
            gmme_id( Line3D::type_name_static(), 12 ) );
        builder.geology.add_parent_children_relation(
            gmge_id( Contact3D::type_name_static(), 12 ),
            gmme_id( Line3D::type_name_static(), 13 ) );

        // For Contact 13
        builder.geology.add_parent_children_relation(
            gmge_id( Contact3D::type_name_static(), 13 ),
            gmme_id( Line3D::type_name_static(), 14 ) );
        builder.geology.add_parent_children_relation(
            gmge_id( Contact3D::type_name_static(), 13 ),
            gmme_id( Line3D::type_name_static(), 15 ) );

        // For Contact 14
        builder.geology.add_parent_children_relation(
            gmge_id( Contact3D::type_name_static(), 14 ),
            gmme_id( Line3D::type_name_static(), 16 ) );
        builder.geology.add_parent_children_relation(
            gmge_id( Contact3D::type_name_static(), 14 ),
            gmme_id( Line3D::type_name_static(), 17 ) );

        // For Contact 15
        builder.geology.add_parent_children_relation(
            gmge_id( Contact3D::type_name_static(), 15 ),
            gmme_id( Line3D::type_name_static(), 19 ) );
        builder.geology.add_parent_children_relation(
            gmge_id( Contact3D::type_name_static(), 15 ),
            gmme_id( Line3D::type_name_static(), 18 ) );

        /////////////////////////////////////////////////

        // For Interface 0
        builder.geology.add_parent_children_relation(
            gmge_id( Interface3D::type_name_static(), 0 ),
            gmme_id( Surface3D::type_name_static(), 0 ) );

        // For Interface 1
        builder.geology.add_parent_children_relation(
            gmge_id( Interface3D::type_name_static(), 1 ),
            gmme_id( Surface3D::type_name_static(), 1 ) );

        // For Interface 2
        builder.geology.add_parent_children_relation(
            gmge_id( Interface3D::type_name_static(), 2 ),
            gmme_id( Surface3D::type_name_static(), 2 ) );

        // For Interface 3
        builder.geology.add_parent_children_relation(
            gmge_id( Interface3D::type_name_static(), 3 ),
            gmme_id( Surface3D::type_name_static(), 3 ) );
        builder.geology.add_parent_children_relation(
            gmge_id( Interface3D::type_name_static(), 3 ),
            gmme_id( Surface3D::type_name_static(), 4 ) );

        // For Interface 4
        builder.geology.add_parent_children_relation(
            gmge_id( Interface3D::type_name_static(), 4 ),
            gmme_id( Surface3D::type_name_static(), 5 ) );
        builder.geology.add_parent_children_relation(
            gmge_id( Interface3D::type_name_static(), 4 ),
            gmme_id( Surface3D::type_name_static(), 6 ) );

        // For Interface 5
        builder.geology.add_parent_children_relation(
            gmge_id( Interface3D::type_name_static(), 5 ),
            gmme_id( Surface3D::type_name_static(), 10 ) );
        builder.geology.add_parent_children_relation(
            gmge_id( Interface3D::type_name_static(), 5 ),
            gmme_id( Surface3D::type_name_static(), 9 ) );

        // For Interface 6
        builder.geology.add_parent_children_relation(
            gmge_id( Interface3D::type_name_static(), 6 ),
            gmme_id( Surface3D::type_name_static(), 8 ) );
        builder.geology.add_parent_children_relation(
            gmge_id( Interface3D::type_name_static(), 6 ),
            gmme_id( Surface3D::type_name_static(), 7 ) );

        ///////////////////////////////////////////////////

        // For Layer 0
        builder.geology.add_parent_children_relation(
            gmge_id( Layer3D::type_name_static(), 0 ),
            gmme_id( Region3D::type_name_static(), 0 ) );

        // For Layer 1
        builder.geology.add_parent_children_relation(
            gmge_id( Layer3D::type_name_static(), 1 ),
            gmme_id( Region3D::type_name_static(), 1 ) );

        // Then, we end the model building
        // This method will set the missing information for the boundaries
        // and parent/child relation. e. g., if you decide to use the
        // add_parent_children_relation (like above), the child has no
        // information of who
        // is his parent. This method deal with that by filling the missing
        // information
        builder.end_geomodel();

        // We save the builded model
        geomodel_save( geomodel, "builded_model.gm" );
    }
    catch( const RINGMeshException& e )
    {
        Logger::err( e.category(), e.what() );
        return 1;
    }
    catch( const std::exception& e )
    {
        Logger::err( "Exception", e.what() );
        return 1;
    }
    return 0;
}
