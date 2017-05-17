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

    try {

        // This line stands for the initialization
        // of Geogram and the factories of RINGMesh
        // IT IS MANDATORY
        default_configure();

        // Say Hello
        print_header_information();
        Logger::div( "RINGMesh Training" );
        Logger::out( "", "Welcome to the training of RINGMesh !" );

        // Measure the time between the GEO::Stopwatch creation and its destruction.
        GEO::Stopwatch total( "Total time" );

        // We instantiate the class GeoModel
        GeoModel geomodel;

        // To build the model, we have to use the class
        // GeoModelBuilder, which is a safety for the user
        // Indeed, the GeoModel can't be directly modified,
        // It has to be done using the GeoModelBuilder
        GeoModelBuilder builder( geomodel );

        //#############################
        // Declaration of the Entities#
        //#############################

        // For the next section, read the documentation to understand
        // the concept of Geological Entity and Mesh Entities
        // Let's to a sum up of the GeoModel we try to build:
        // For the Geological Entities (handle by the class GeoModelGeologicalEntity):
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

        //We first create the GeoModelGeoglogicalEntity
        // Create the contacts
        for( index_t contact = 0; contact < nb_contacts; contact++ ) {
            builder.geology.create_geological_entity( Contact::type_name_static() );
            // the static method type_name_static() is available for each
            // GeoModelEntity. It returns an EntityType which is a string
            // corresponding to the Type of the entity.
        }

        // Create the Interfaces
        for( index_t interface_itr = 0; interface_itr < nb_interfaces;
            interface_itr++ ) {
            builder.geology.create_geological_entity(
                Interface::type_name_static() );
        }

        // Create the Layers
        for( index_t layer = 0; layer < nb_layers; layer++ ) {
            builder.geology.create_geological_entity( Layer::type_name_static() );
        }

        // Then we create the GeoModelMeshEntity
        // Create the Corners
        for( index_t corner = 0; corner < nb_corners; corner++ ) {
            builder.topology.create_mesh_entity< Corner >();
        }

        // Create the Lines
        for( index_t lines = 0; lines < nb_lines; lines++ ) {
            builder.topology.create_mesh_entity< Line >();
        }

        // Create the Surfaces
        for( index_t surface = 0; surface < nb_surfaces; surface++ ) {
            builder.topology.create_mesh_entity< Surface >();

        }

        // Create the Regions
        for( index_t region = 0; region < nb_regions; region++ ) {
            builder.topology.create_mesh_entity< Region >();
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
        for( index_t corner = 0; corner < nb_corners; corner++ ) {
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
        builder.geometry.set_surface_geometry( 0, cur_coor_surface, facet,
            facet_ptr );

        cur_coor_surface[0] = corners_table[8];
        cur_coor_surface[1] = corners_table[9];
        cur_coor_surface[2] = corners_table[10];
        cur_coor_surface[3] = corners_table[11];
        builder.geometry.set_surface_geometry( 1, cur_coor_surface, facet,
            facet_ptr );

        cur_coor_surface[0] = corners_table[4];
        cur_coor_surface[1] = corners_table[5];
        cur_coor_surface[2] = corners_table[6];
        cur_coor_surface[3] = corners_table[7];
        builder.geometry.set_surface_geometry( 2, cur_coor_surface, facet,
            facet_ptr );

        cur_coor_surface[0] = corners_table[8];
        cur_coor_surface[1] = corners_table[4];
        cur_coor_surface[2] = corners_table[7];
        cur_coor_surface[3] = corners_table[11];
        builder.geometry.set_surface_geometry( 3, cur_coor_surface, facet,
            facet_ptr );

        cur_coor_surface[0] = corners_table[0];
        cur_coor_surface[1] = corners_table[8];
        cur_coor_surface[2] = corners_table[11];
        cur_coor_surface[3] = corners_table[3];
        builder.geometry.set_surface_geometry( 4, cur_coor_surface, facet,
            facet_ptr );

        cur_coor_surface[0] = corners_table[5];
        cur_coor_surface[1] = corners_table[6];
        cur_coor_surface[2] = corners_table[10];
        cur_coor_surface[3] = corners_table[9];
        builder.geometry.set_surface_geometry( 5, cur_coor_surface, facet,
            facet_ptr );

        cur_coor_surface[0] = corners_table[9];
        cur_coor_surface[1] = corners_table[10];
        cur_coor_surface[2] = corners_table[2];
        cur_coor_surface[3] = corners_table[1];
        builder.geometry.set_surface_geometry( 6, cur_coor_surface, facet,
            facet_ptr );

        cur_coor_surface[0] = corners_table[8];
        cur_coor_surface[1] = corners_table[4];
        cur_coor_surface[2] = corners_table[5];
        cur_coor_surface[3] = corners_table[9];
        builder.geometry.set_surface_geometry( 7, cur_coor_surface, facet,
            facet_ptr );

        cur_coor_surface[0] = corners_table[0];
        cur_coor_surface[1] = corners_table[8];
        cur_coor_surface[2] = corners_table[9];
        cur_coor_surface[3] = corners_table[1];
        builder.geometry.set_surface_geometry( 8, cur_coor_surface, facet,
            facet_ptr );

        cur_coor_surface[0] = corners_table[6];
        cur_coor_surface[1] = corners_table[10];
        cur_coor_surface[2] = corners_table[11];
        cur_coor_surface[3] = corners_table[7];
        builder.geometry.set_surface_geometry( 9, cur_coor_surface, facet,
            facet_ptr );

        cur_coor_surface[0] = corners_table[10];
        cur_coor_surface[1] = corners_table[2];
        cur_coor_surface[2] = corners_table[3];
        cur_coor_surface[3] = corners_table[11];
        builder.geometry.set_surface_geometry( 10, cur_coor_surface, facet,
            facet_ptr );

        //###################################
        // Setting the Boundaries relations #
        //###################################

        //We set the Corners which are in boundary of the lines
        // The add_mesh_entity_boundary_relation method take as first argument the
        // gme_t of the boundary and in second argument
        // the id of the GeoModelMeshentity bounded by the boundary
        // Remember :
        // Lines are bounded by Corners
        // Surfaces are bounded by Lines
        // Region are bounded by Surfaces

        // For corner 0
        gmme_id corner0( Corner::type_name_static(), 0 );
        builder.topology.add_mesh_entity_boundary_relation(
            gmme_id( Line::type_name_static(), 0 ), corner0 );
        builder.topology.add_mesh_entity_boundary_relation(
            gmme_id( Line::type_name_static(), 13 ), corner0 );
        builder.topology.add_mesh_entity_boundary_relation(
            gmme_id( Line::type_name_static(), 3 ), corner0 );

        // For corner 1
        gmme_id corner1( Corner::type_name_static(), 1 );
        builder.topology.add_mesh_entity_boundary_relation(
            gmme_id( Line::type_name_static(), 2 ), corner1 );
        builder.topology.add_mesh_entity_boundary_relation(
            gmme_id( Line::type_name_static(), 3 ), corner1 );
        builder.topology.add_mesh_entity_boundary_relation(
            gmme_id( Line::type_name_static(), 17 ), corner1 );

        // For corner 2
        gmme_id corner2( Corner::type_name_static(), 2 );
        builder.topology.add_mesh_entity_boundary_relation(
            gmme_id( Line::type_name_static(), 2 ), corner2 );
        builder.topology.add_mesh_entity_boundary_relation(
            gmme_id( Line::type_name_static(), 19 ), corner2 );
        builder.topology.add_mesh_entity_boundary_relation(
            gmme_id( Line::type_name_static(), 1 ), corner2 );

        // For corner 3
        gmme_id corner3( Corner::type_name_static(), 3 );
        builder.topology.add_mesh_entity_boundary_relation(
            gmme_id( Line::type_name_static(), 0 ), corner3 );
        builder.topology.add_mesh_entity_boundary_relation(
            gmme_id( Line::type_name_static(), 1 ), corner3 );
        builder.topology.add_mesh_entity_boundary_relation(
            gmme_id( Line::type_name_static(), 15 ), corner3 );

        // For corner 4
        gmme_id corner4( Corner::type_name_static(), 4 );
        builder.topology.add_mesh_entity_boundary_relation(
            gmme_id( Line::type_name_static(), 12 ), corner4 );
        builder.topology.add_mesh_entity_boundary_relation(
            gmme_id( Line::type_name_static(), 8 ), corner4 );
        builder.topology.add_mesh_entity_boundary_relation(
            gmme_id( Line::type_name_static(), 11 ), corner4 );

        // For corner 5
        gmme_id corner5( Corner::type_name_static(), 5 );
        builder.topology.add_mesh_entity_boundary_relation(
            gmme_id( Line::type_name_static(), 10 ), corner5 );
        builder.topology.add_mesh_entity_boundary_relation(
            gmme_id( Line::type_name_static(), 16 ), corner5 );
        builder.topology.add_mesh_entity_boundary_relation(
            gmme_id( Line::type_name_static(), 11 ), corner5 );

        // For corner 6
        gmme_id corner6( Corner::type_name_static(), 6 );
        builder.topology.add_mesh_entity_boundary_relation(
            gmme_id( Line::type_name_static(), 10 ), corner6 );
        builder.topology.add_mesh_entity_boundary_relation(
            gmme_id( Line::type_name_static(), 18 ), corner6 );
        builder.topology.add_mesh_entity_boundary_relation(
            gmme_id( Line::type_name_static(), 9 ), corner6 );

        // For corner 7
        gmme_id corner7( Corner::type_name_static(), 7 );
        builder.topology.add_mesh_entity_boundary_relation(
            gmme_id( Line::type_name_static(), 8 ), corner7 );
        builder.topology.add_mesh_entity_boundary_relation(
            gmme_id( Line::type_name_static(), 9 ), corner7 );
        builder.topology.add_mesh_entity_boundary_relation(
            gmme_id( Line::type_name_static(), 14 ), corner7 );

        // For corner 8
        gmme_id corner8( Corner::type_name_static(), 8 );
        builder.topology.add_mesh_entity_boundary_relation(
            gmme_id( Line::type_name_static(), 12 ), corner8 );
        builder.topology.add_mesh_entity_boundary_relation(
            gmme_id( Line::type_name_static(), 13 ), corner8 );
        builder.topology.add_mesh_entity_boundary_relation(
            gmme_id( Line::type_name_static(), 7 ), corner8 );
        builder.topology.add_mesh_entity_boundary_relation(
            gmme_id( Line::type_name_static(), 4 ), corner8 );

        // For corner 9
        gmme_id corner9( Corner::type_name_static(), 9 );
        builder.topology.add_mesh_entity_boundary_relation(
            gmme_id( Line::type_name_static(), 16 ), corner9 );
        builder.topology.add_mesh_entity_boundary_relation(
            gmme_id( Line::type_name_static(), 17 ), corner9 );
        builder.topology.add_mesh_entity_boundary_relation(
            gmme_id( Line::type_name_static(), 6 ), corner9 );
        builder.topology.add_mesh_entity_boundary_relation(
            gmme_id( Line::type_name_static(), 7 ), corner9 );

        // For corner 10
        gmme_id corner10( Corner::type_name_static(), 10 );
        builder.topology.add_mesh_entity_boundary_relation(
            gmme_id( Line::type_name_static(), 6 ), corner10 );
        builder.topology.add_mesh_entity_boundary_relation(
            gmme_id( Line::type_name_static(), 18 ), corner10 );
        builder.topology.add_mesh_entity_boundary_relation(
            gmme_id( Line::type_name_static(), 19 ), corner10 );
        builder.topology.add_mesh_entity_boundary_relation(
            gmme_id( Line::type_name_static(), 5 ), corner10 );

        // For corner 11
        gmme_id corner11( Corner::type_name_static(), 11 );
        builder.topology.add_mesh_entity_boundary_relation(
            gmme_id( Line::type_name_static(), 4 ), corner11 );
        builder.topology.add_mesh_entity_boundary_relation(
            gmme_id( Line::type_name_static(), 14 ), corner11 );
        builder.topology.add_mesh_entity_boundary_relation(
            gmme_id( Line::type_name_static(), 15 ), corner11 );
        builder.topology.add_mesh_entity_boundary_relation(
            gmme_id( Line::type_name_static(), 5 ), corner11 );

        /////////////////////////////////////////////////////////

        // For line 0
        gmme_id line0( Line::type_name_static(), 0 );
        builder.topology.add_mesh_entity_boundary_relation(
            gmme_id( Surface::type_name_static(), 0 ), line0 );
        builder.topology.add_mesh_entity_boundary_relation(
            gmme_id( Surface::type_name_static(), 4 ), line0 );

        // For line 1
        gmme_id line1( Line::type_name_static(), 1 );
        builder.topology.add_mesh_entity_boundary_relation(
            gmme_id( Surface::type_name_static(), 0 ), line1 );
        builder.topology.add_mesh_entity_boundary_relation(
            gmme_id( Surface::type_name_static(), 10 ), line1 );

        // For line 2
        gmme_id line2( Line::type_name_static(), 2 );
        builder.topology.add_mesh_entity_boundary_relation(
            gmme_id( Surface::type_name_static(), 0 ), line2 );
        builder.topology.add_mesh_entity_boundary_relation(
            gmme_id( Surface::type_name_static(), 6 ), line2 );

        // For line 3
        gmme_id line3( Line::type_name_static(), 3 );
        builder.topology.add_mesh_entity_boundary_relation(
            gmme_id( Surface::type_name_static(), 0 ), line3 );
        builder.topology.add_mesh_entity_boundary_relation(
            gmme_id( Surface::type_name_static(), 8 ), line3 );

        // For line 4
        gmme_id line4( Line::type_name_static(), 4 );
        builder.topology.add_mesh_entity_boundary_relation(
            gmme_id( Surface::type_name_static(), 1 ), line4 );
        builder.topology.add_mesh_entity_boundary_relation(
            gmme_id( Surface::type_name_static(), 4 ), line4 );
        builder.topology.add_mesh_entity_boundary_relation(
            gmme_id( Surface::type_name_static(), 3 ), line4 );

        // For line 5
        gmme_id line5( Line::type_name_static(), 5 );
        builder.topology.add_mesh_entity_boundary_relation(
            gmme_id( Surface::type_name_static(), 1 ), line5 );
        builder.topology.add_mesh_entity_boundary_relation(
            gmme_id( Surface::type_name_static(), 9 ), line5 );
        builder.topology.add_mesh_entity_boundary_relation(
            gmme_id( Surface::type_name_static(), 10 ), line5 );

        // For line 6
        gmme_id line6( Line::type_name_static(), 6 );
        builder.topology.add_mesh_entity_boundary_relation(
            gmme_id( Surface::type_name_static(), 1 ), line6 );
        builder.topology.add_mesh_entity_boundary_relation(
            gmme_id( Surface::type_name_static(), 6 ), line6 );
        builder.topology.add_mesh_entity_boundary_relation(
            gmme_id( Surface::type_name_static(), 5 ), line6 );

        // For line 7
        gmme_id line7( Line::type_name_static(), 7 );
        builder.topology.add_mesh_entity_boundary_relation(
            gmme_id( Surface::type_name_static(), 1 ), line7 );
        builder.topology.add_mesh_entity_boundary_relation(
            gmme_id( Surface::type_name_static(), 7 ), line7 );
        builder.topology.add_mesh_entity_boundary_relation(
            gmme_id( Surface::type_name_static(), 8 ), line7 );

        // For line 8
        gmme_id line8( Line::type_name_static(), 8 );
        builder.topology.add_mesh_entity_boundary_relation(
            gmme_id( Surface::type_name_static(), 2 ), line8 );
        builder.topology.add_mesh_entity_boundary_relation(
            gmme_id( Surface::type_name_static(), 3 ), line8 );

        // For line 9
        gmme_id line9( Line::type_name_static(), 9 );
        builder.topology.add_mesh_entity_boundary_relation(
            gmme_id( Surface::type_name_static(), 2 ), line9 );
        builder.topology.add_mesh_entity_boundary_relation(
            gmme_id( Surface::type_name_static(), 9 ), line9 );

        // For line 10
        gmme_id line10( Line::type_name_static(), 10 );
        builder.topology.add_mesh_entity_boundary_relation(
            gmme_id( Surface::type_name_static(), 2 ), line10 );
        builder.topology.add_mesh_entity_boundary_relation(
            gmme_id( Surface::type_name_static(), 5 ), line10 );

        // For line 11
        gmme_id line11( Line::type_name_static(), 11 );
        builder.topology.add_mesh_entity_boundary_relation(
            gmme_id( Surface::type_name_static(), 2 ), line11 );
        builder.topology.add_mesh_entity_boundary_relation(
            gmme_id( Surface::type_name_static(), 7 ), line11 );

        // For line 12
        gmme_id line12( Line::type_name_static(), 12 );
        builder.topology.add_mesh_entity_boundary_relation(
            gmme_id( Surface::type_name_static(), 3 ), line12 );
        builder.topology.add_mesh_entity_boundary_relation(
            gmme_id( Surface::type_name_static(), 7 ), line12 );

        // For line 13
        gmme_id line13( Line::type_name_static(), 13 );
        builder.topology.add_mesh_entity_boundary_relation(
            gmme_id( Surface::type_name_static(), 8 ), line13 );
        builder.topology.add_mesh_entity_boundary_relation(
            gmme_id( Surface::type_name_static(), 4 ), line13 );

        // For line 14
        gmme_id line14( Line::type_name_static(), 14 );
        builder.topology.add_mesh_entity_boundary_relation(
            gmme_id( Surface::type_name_static(), 3 ), line14 );
        builder.topology.add_mesh_entity_boundary_relation(
            gmme_id( Surface::type_name_static(), 9 ), line14 );

        // For line 15
        gmme_id line15( Line::type_name_static(), 15 );
        builder.topology.add_mesh_entity_boundary_relation(
            gmme_id( Surface::type_name_static(), 4 ), line15 );
        builder.topology.add_mesh_entity_boundary_relation(
            gmme_id( Surface::type_name_static(), 10 ), line15 );

        // For line 16
        gmme_id line16( Line::type_name_static(), 16 );
        builder.topology.add_mesh_entity_boundary_relation(
            gmme_id( Surface::type_name_static(), 7 ), line16 );
        builder.topology.add_mesh_entity_boundary_relation(
            gmme_id( Surface::type_name_static(), 5 ), line16 );

        // For line 17
        gmme_id line17( Line::type_name_static(), 17 );
        builder.topology.add_mesh_entity_boundary_relation(
            gmme_id( Surface::type_name_static(), 8 ), line17 );
        builder.topology.add_mesh_entity_boundary_relation(
            gmme_id( Surface::type_name_static(), 6 ), line17 );

        // For line 18
        gmme_id line18( Line::type_name_static(), 18 );
        builder.topology.add_mesh_entity_boundary_relation(
            gmme_id( Surface::type_name_static(), 5 ), line18 );
        builder.topology.add_mesh_entity_boundary_relation(
            gmme_id( Surface::type_name_static(), 9 ), line18 );

        // For line 19
        gmme_id line19( Line::type_name_static(), 19 );
        builder.topology.add_mesh_entity_boundary_relation(
            gmme_id( Surface::type_name_static(), 6 ), line19 );
        builder.topology.add_mesh_entity_boundary_relation(
            gmme_id( Surface::type_name_static(), 10 ), line19 );

        /////////////////////////////////////////////////////////

        // For surface 0
        gmme_id surface0( Surface::type_name_static(), 0 );
        builder.topology.add_mesh_entity_boundary_relation(
            gmme_id( Region::type_name_static(), 0 ), surface0 );

        // For surface 1
        gmme_id surface1( Surface::type_name_static(), 1 );
        builder.topology.add_mesh_entity_boundary_relation(
            gmme_id( Region::type_name_static(), 0 ), surface1 );

        // For surface 2
        gmme_id surface2( Surface::type_name_static(), 2 );
        builder.topology.add_mesh_entity_boundary_relation(
            gmme_id( Region::type_name_static(), 1 ), surface2 );

        // For surface 3
        gmme_id surface3( Surface::type_name_static(), 3 );
        builder.topology.add_mesh_entity_boundary_relation(
            gmme_id( Region::type_name_static(), 1 ), surface3 );

        // For surface 4
        gmme_id surface4( Surface::type_name_static(), 4 );
        builder.topology.add_mesh_entity_boundary_relation(
            gmme_id( Region::type_name_static(), 0 ), surface4 );

        // For surface 5
        gmme_id surface5( Surface::type_name_static(), 5 );
        builder.topology.add_mesh_entity_boundary_relation(
            gmme_id( Region::type_name_static(), 1 ), surface5 );

        // For surface 6
        gmme_id surface6( Surface::type_name_static(), 6 );
        builder.topology.add_mesh_entity_boundary_relation(
            gmme_id( Region::type_name_static(), 0 ), surface6 );

        // For surface 7
        gmme_id surface7( Surface::type_name_static(), 7 );
        builder.topology.add_mesh_entity_boundary_relation(
            gmme_id( Region::type_name_static(), 1 ), surface7 );

        // For surface 8
        gmme_id surface8( Surface::type_name_static(), 8 );
        builder.topology.add_mesh_entity_boundary_relation(
            gmme_id( Region::type_name_static(), 0 ), surface8 );

        // For surface 9
        gmme_id surface9( Surface::type_name_static(), 9 );
        builder.topology.add_mesh_entity_boundary_relation(
            gmme_id( Region::type_name_static(), 1 ), surface9 );

        // For surface 10
        gmme_id surface10( Surface::type_name_static(), 10 );
        builder.topology.add_mesh_entity_boundary_relation(
            gmme_id( Region::type_name_static(), 0 ), surface10 );

        // For the Universe Boundary
        builder.topology.add_universe_boundary( 0, true );
        builder.topology.add_universe_boundary( 2, true );
        builder.topology.add_universe_boundary( 3, true );
        builder.topology.add_universe_boundary( 4, true );
        builder.topology.add_universe_boundary( 5, true );
        builder.topology.add_universe_boundary( 6, true );
        builder.topology.add_universe_boundary( 7, true );
        builder.topology.add_universe_boundary( 8, true );
        builder.topology.add_universe_boundary( 9, true );
        builder.topology.add_universe_boundary( 10, true );

        //#####################################
        // Setting the parent/child relations #
        //#####################################

        // Remember :
        // Child of a Contact is a Line
        // Child of an Interface is a Surface
        // Child of a Layer is a Region

        // We use the method "add_geological_entity_child"
        // First argument is the parent (ie a GeoModelGeologicalEntity)
        // Second argument is the index of the child (ie a GeoModelMeshEntity)

        // For Contact 0
        builder.geology.add_geological_entity_child(
            gmge_id( Contact::type_name_static(), 0 ), 0 );

        // For Contact 1
        builder.geology.add_geological_entity_child(
            gmge_id( Contact::type_name_static(), 1 ), 1 );

        // For Contact 2
        builder.geology.add_geological_entity_child(
            gmge_id( Contact::type_name_static(), 2 ), 2 );

        // For Contact 3
        builder.geology.add_geological_entity_child(
            gmge_id( Contact::type_name_static(), 3 ), 3 );

        // For Contact 4
        builder.geology.add_geological_entity_child(
            gmge_id( Contact::type_name_static(), 4 ), 4 );

        // For Contact 5
        builder.geology.add_geological_entity_child(
            gmge_id( Contact::type_name_static(), 5 ), 5 );

        // For Contact 6
        builder.geology.add_geological_entity_child(
            gmge_id( Contact::type_name_static(), 6 ), 6 );

        // For Contact 7
        builder.geology.add_geological_entity_child(
            gmge_id( Contact::type_name_static(), 7 ), 7 );

        // For Contact 8
        builder.geology.add_geological_entity_child(
            gmge_id( Contact::type_name_static(), 8 ), 8 );

        // For Contact 9
        builder.geology.add_geological_entity_child(
            gmge_id( Contact::type_name_static(), 9 ), 9 );

        // For Contact 10
        builder.geology.add_geological_entity_child(
            gmge_id( Contact::type_name_static(), 10 ), 10 );

        // For Contact 11
        builder.geology.add_geological_entity_child(
            gmge_id( Contact::type_name_static(), 11 ), 11 );

        // For Contact 12
        builder.geology.add_geological_entity_child(
            gmge_id( Contact::type_name_static(), 12 ), 12 );
        builder.geology.add_geological_entity_child(
            gmge_id( Contact::type_name_static(), 12 ), 13 );

        // For Contact 13
        builder.geology.add_geological_entity_child(
            gmge_id( Contact::type_name_static(), 13 ), 14 );
        builder.geology.add_geological_entity_child(
            gmge_id( Contact::type_name_static(), 13 ), 15 );

        // For Contact 14
        builder.geology.add_geological_entity_child(
            gmge_id( Contact::type_name_static(), 14 ), 16 );
        builder.geology.add_geological_entity_child(
            gmge_id( Contact::type_name_static(), 14 ), 17 );

        // For Contact 15
        builder.geology.add_geological_entity_child(
            gmge_id( Contact::type_name_static(), 15 ), 19 );
        builder.geology.add_geological_entity_child(
            gmge_id( Contact::type_name_static(), 15 ), 18 );

        /////////////////////////////////////////////////

        // For Interface 0
        builder.geology.add_geological_entity_child(
            gmge_id( Interface::type_name_static(), 0 ), 0 );

        // For Interface 1
        builder.geology.add_geological_entity_child(
            gmge_id( Interface::type_name_static(), 1 ), 1 );

        // For Interface 2
        builder.geology.add_geological_entity_child(
            gmge_id( Interface::type_name_static(), 2 ), 2 );

        // For Interface 3
        builder.geology.add_geological_entity_child(
            gmge_id( Interface::type_name_static(), 3 ), 3 );
        builder.geology.add_geological_entity_child(
            gmge_id( Interface::type_name_static(), 3 ), 4 );

        // For Interface 4
        builder.geology.add_geological_entity_child(
            gmge_id( Interface::type_name_static(), 4 ), 5 );
        builder.geology.add_geological_entity_child(
            gmge_id( Interface::type_name_static(), 4 ), 6 );

        // For Interface 5
        builder.geology.add_geological_entity_child(
            gmge_id( Interface::type_name_static(), 5 ), 10 );
        builder.geology.add_geological_entity_child(
            gmge_id( Interface::type_name_static(), 5 ), 9 );

        // For Interface 6
        builder.geology.add_geological_entity_child(
            gmge_id( Interface::type_name_static(), 6 ), 8 );
        builder.geology.add_geological_entity_child(
            gmge_id( Interface::type_name_static(), 6 ), 7 );

        ///////////////////////////////////////////////////

        // For Layer 0
        builder.geology.add_geological_entity_child(
            gmge_id( Layer::type_name_static(), 0 ), 0 );

        // For Layer 1
        builder.geology.add_geological_entity_child(
            gmge_id( Layer::type_name_static(), 1 ), 1 );

        // Then, we end the model building
        // This method will set the missing information for the boundaries
        // and parent/child relation. e. g., if you decide to use the
        // add_geological_entity_child (like above), the child has no information of who
        // is his parent. This method deal with that by filling the missing information
        builder.end_geomodel();

        // We save the builded model
        geomodel_save( geomodel, "builded_model.gm" );

    } catch( const RINGMeshException& e ) {
        Logger::err( e.category(), e.what() );
        return 1;
    } catch( const std::exception& e ) {
        Logger::err( "Exception", e.what() );
        return 1;
    }
    return 0;
}
