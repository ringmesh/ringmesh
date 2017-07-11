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

#include <ringmesh/ringmesh_tests_config.h>

#include <ringmesh/geomodel/geomodel.h>
#include <ringmesh/geomodel/geomodel_mesh.h>
#include <ringmesh/io/io.h>

/*!
 * @author Benjamin Chauvin
 */

namespace {
    using namespace RINGMesh;

    void load_geomodel( GeoModel& in, const std::string& filename )
    {
        std::string input_model_file_name( ringmesh_test_data_path );
        input_model_file_name += filename;

        bool loaded_model_is_valid = geomodel_load( in, input_model_file_name );

        if( !loaded_model_is_valid ) {
            throw RINGMeshException( "RINGMesh Test",
                "Failed when loading model " + in.name()
                    + ": the loaded model is not valid." );
        }
    }

    void run_tests()
    {
        GeoModel in;
        load_geomodel( in, "modelA1_volume_meshed.gm" );

        // test cell from gm to gmm
        for( index_t reg_i = 0; reg_i < in.nb_regions(); ++reg_i ) {
            const Region& cur_reg = in.region( reg_i );
            GEO::Attribute< double > cell_double_attr(
                cur_reg.cell_attribute_manager(), "cell_double_attr" );
            GEO::Attribute< vec3 > cell_vec3_attr( cur_reg.cell_attribute_manager(),
                "cell_vec3_attr" );
            /*GEO::Attribute< const char* > cell_string_attr(
                cur_reg.cell_attribute_manager(), "cell_string_attr" );*/
            for( index_t cell_i = 0; cell_i < cur_reg.nb_mesh_elements();
                ++cell_i ) {
                cell_double_attr[cell_i] = cur_reg.mesh_element_size( cell_i );
                cell_vec3_attr[cell_i] = cur_reg.mesh_element_barycenter( cell_i );
                /*cell_string_attr[cell_i] =
                    std::to_string( cell_double_attr[cell_i] ).data()*/;
            }
        }

        const GeoModelMesh& gmm = in.mesh;
        gmm.transfer_cell_attributes_from_gm_regions_to_gmm();

        const GeoModelMeshCells& gmmc = gmm.cells;
        GEO::AttributesManager& gmmc_attr_mgr = gmmc.attribute_manager();

        /// TODO store attribute in string to avoid to write them several times...
        if( !gmmc_attr_mgr.is_defined( "cell_double_attr" ) ) {
            throw RINGMeshException( "RINGMesh Test",
                "Problem of transfer from the GeoModel regions and the GeoModelMesh for cell_double_attr" );
        }
        if( !gmmc_attr_mgr.is_defined( "cell_vec3_attr" ) ) {
            throw RINGMeshException( "RINGMesh Test",
                "Problem of transfer from the GeoModel regions and the GeoModelMesh for cell_vec3_attr" );
        }
        /*if( !gmmc_attr_mgr.is_defined( "cell_string_attr" ) ) {
            /// TODO put a better message, attr not defined in gmm...
            throw RINGMeshException( "RINGMesh Test",
                "Problem of transfer from the GeoModel regions and the GeoModelMesh for cell_string_attr" );
        }*/
        GEO::Attribute< double > cell_double_attr( gmmc_attr_mgr,
            "cell_double_attr" );
        GEO::Attribute< vec3 > cell_vec3_attr( gmmc_attr_mgr, "cell_vec3_attr" );
        /*GEO::Attribute< const char* > cell_string_attr( gmmc_attr_mgr,
            "cell_string_attr" );*/
        for( index_t cell_i = 0; cell_i < gmmc.nb_cells(); ++cell_i ) {
            double cell_volume = gmmc.volume( cell_i );
            if( std::abs( cell_volume - cell_double_attr[cell_i] )
                > in.epsilon3() ) {
                throw RINGMeshException( "RINGMesh Test", "Bad transfer" ); //TODO improve message
            }
            vec3 diff = gmmc.barycenter( cell_i ) - cell_vec3_attr[cell_i];
            if( std::abs( diff.x ) > in.epsilon()
                || std::abs( diff.y ) > in.epsilon()
                || std::abs( diff.z ) > in.epsilon() ) {
                throw RINGMeshException( "RINGMesh Test", "Bad transfer" ); //TODO improve message
            }
            /*std::string volume_in_string = std::to_string( cell_volume );
            if( volume_in_string != std::string( cell_string_attr[cell_i] ) ) {
                throw RINGMeshException( "RINGMesh Test", "Bad transfer" ); //TODO improve message
            }*/
        }
    }
}

int main()
{
    using namespace RINGMesh;

    try {
        default_configure();

        Logger::out( "TEST", "Test IO for a GeoModel in .gm" );
        run_tests();

    } catch( const RINGMeshException& e ) {
        Logger::err( e.category(), e.what() );
        return 1;
    } catch( const std::exception& e ) {
        Logger::err( "Exception", e.what() );
        return 1;
    }
    Logger::out( "TEST", "SUCCESS" );
    return 0;
}
