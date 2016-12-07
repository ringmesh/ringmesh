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

#include <ringmesh/ringmesh_tests_config.h>

#include <vector>

#include <geogram/points/colocate.h>
#include <geogram/basic/logger.h>

#include <ringmesh/geomodel/geo_model.h>
#include <ringmesh/geomodel/geo_model_mesh_entity.h>
#include <ringmesh/io/io.h>

/*!
 * @author Arnaud Botella
 */

using namespace RINGMesh ;

void test_ann_geogram()
{
    Logger::out( "TEST" ) << "Test MakeUnique Geogram" << std::endl ;

    GeoModel in ;
    std::string input_model_file_name( ringmesh_test_data_path ) ;
    input_model_file_name += "modelA2.ml" ;
    geomodel_load( in, input_model_file_name ) ;

    index_t nb_non_unique_vertices = in.nb_corners() ;

    for( index_t l = 0; l < in.nb_lines(); l++ ) {
        nb_non_unique_vertices += in.line( l ).nb_vertices() ;
    }
    for( index_t s = 0; s < in.nb_surfaces(); s++ ) {
        nb_non_unique_vertices += in.surface( s ).nb_vertices() ;
    }

    std::vector< vec3 > all_vertices( nb_non_unique_vertices ) ;
    index_t index = 0 ;
    for( index_t c = 0; c < in.nb_corners(); c++ ) {
        all_vertices[index++ ] = in.corner( c ).vertex( 0 ) ;
    }
    for( index_t l = 0; l < in.nb_lines(); l++ ) {
        const Line& line = in.line( l ) ;
        for( index_t v = 0; v < line.nb_vertices(); v++ ) {
            all_vertices[index++ ] = line.vertex( v ) ;
        }
    }
    for( index_t s = 0; s < in.nb_surfaces(); s++ ) {
        const Surface& surface = in.surface( s ) ;
        for( index_t v = 0; v < surface.nb_vertices(); v++ ) {
            all_vertices[index++ ] = surface.vertex( v ) ;
        }
    }

    GEO::vector< index_t > old2new ;
    index_t geo_nb = GEO::Geom::colocate( all_vertices[0].data(), 3,
        nb_non_unique_vertices, old2new, in.epsilon() ) ;

    index_t ringmesh_nb = in.mesh.vertices.nb() ;

    if( ringmesh_nb != geo_nb ) {
        Logger::out( "TEST" ) << "initial_nb=" << nb_non_unique_vertices
            << std::endl ;
        Logger::out( "TEST" ) << "geo_nb=" << geo_nb << std::endl ;
        Logger::out( "TEST" ) << "ringmesh_nb=" << ringmesh_nb << std::endl ;
        throw RINGMeshException( "TEST", "FAILED" ) ;
    }
}

void test_ann_ringmesh()
{
    Logger::out( "TEST" ) << "Test MakeUnique RINGMesh" << std::endl ;
    vec3 p1( 0, 0, 0 ) ;
    vec3 p2( 1, 1, 1 ) ;
    vec3 p3( 2, 2, 2 ) ;
    vec3 p4( 3, 3, 3 ) ;

    std::vector< vec3 > vertices( 6 ) ;
    vertices[0] = p1 ;
    vertices[1] = p2 ;
    vertices[2] = p1 ;
    vertices[3] = p3 ;
    vertices[4] = p2 ;
    vertices[5] = p4 ;

    GEO::vector< vec3 > hardcoded_unique_vertices( 4 ) ;
    GEO::vector< index_t > hardcoded_index_map( 6 ) ;

    hardcoded_index_map[0] = 0 ;
    hardcoded_index_map[1] = 1 ;
    hardcoded_index_map[2] = 0 ;
    hardcoded_index_map[3] = 2 ;
    hardcoded_index_map[4] = 1 ;
    hardcoded_index_map[5] = 3 ;

    hardcoded_unique_vertices[0] = p1 ;
    hardcoded_unique_vertices[1] = p2 ;
    hardcoded_unique_vertices[2] = p3 ;
    hardcoded_unique_vertices[3] = p4 ;

    ColocaterANN ann( vertices ) ;
    GEO::vector< vec3 > unique_vertices ;
    GEO::vector< index_t > index_map ;
    ann.get_colocated_index_mapping( global_epsilon, index_map, unique_vertices ) ;
    for( index_t i = 0; i < index_map.size(); i++ ) {
        if( index_map[i] != hardcoded_index_map[i] ) {
            throw RINGMeshException( "TEST", "Index map found is wrong" ) ;
        }
    }

    for( index_t v = 0; v < unique_vertices.size(); v++ ) {
        if( unique_vertices[v] != hardcoded_unique_vertices[v] ) {
            throw RINGMeshException( "TEST", "unique vertices found are wrong" ) ;
        }
    }
}
int main()
{
    try {
        default_configure() ;

        test_ann_geogram() ;
        test_ann_ringmesh() ;

    } catch( const RINGMeshException& e ) {
        Logger::err( e.category() ) << e.what() << std::endl ;
        return 1 ;
    } catch( const std::exception& e ) {
        Logger::err( "Exception" ) << e.what() << std::endl ;
        return 1 ;
    }
    Logger::out( "TEST" ) << "SUCCESS" << std::endl ;
    return 0 ;
}
