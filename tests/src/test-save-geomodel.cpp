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

#include <geogram/basic/line_stream.h>

#include <ringmesh/basic/command_line.h>
#include <ringmesh/geomodel/geomodel.h>
#include <ringmesh/geomodel/geomodel_geological_entity.h>
#include <ringmesh/geomodel/geomodel_validity.h>
#include <ringmesh/io/io.h>

/*!
 * @file Test GeoModel saving
 * @author Arnaud Botella
 */

using namespace RINGMesh ;

void load_geomodel( GeoModel& geomodel, const std::string& file )
{
    bool loaded_model_is_valid = geomodel_load( geomodel, file ) ;
    if( !loaded_model_is_valid ) {
        throw RINGMeshException( "TEST",
            "Failed when loading model " + geomodel.name()
                + ": the loaded model is not valid." ) ;
    }
}

void get_line( GEO::LineInput& in )
{
    in.get_line() ;
    in.get_fields() ;
}

std::vector< std::string > extract_zip( const std::string& zip_file )
{
    std::string pwd = GEO::FileSystem::get_current_working_directory() ;
    unzFile uz = unzOpen( zip_file.c_str() ) ;
    std::vector< std::string > filenames ;
    do {
        char char_file_name[MAX_FILENAME] ;
        if( unzGetCurrentFileInfo64( uz, nullptr, char_file_name,
        MAX_FILENAME, nullptr, 0, nullptr, 0 ) != UNZ_OK ) {
            throw RINGMeshException( "I/O", "Unable to get file name" ) ;
        }
        unzip_current_file( uz, char_file_name ) ;
        filenames.push_back( pwd + "/" + char_file_name ) ;
    } while( unzGoToNextFile( uz ) == UNZ_OK ) ;

    return filenames ;
}

std::vector< std::string > get_gm_files( const std::string& file_name )
{
    std::string directory = GEO::FileSystem::dir_name( file_name ) + "/temp" ;
    GEO::FileSystem::create_directory( directory ) ;
    GEO::FileSystem::set_current_working_directory( directory ) ;
    return extract_zip( file_name ) ;
}

void check_files( const std::string& file1, const std::string& file2 )
{
    if( !compare_files( file1, file2 ) ) {
        throw RINGMeshException( "TEST",
            "Output file " + file1 + " does not match template file " + file2 ) ;
    }
}

void clean_files( const std::vector< std::string >& files )
{
    for( const std::string& file : files ) {
        GEO::FileSystem::delete_file( file ) ;
    }
    GEO::FileSystem::delete_directory( GEO::FileSystem::dir_name( files.front() ) ) ;
}

void clean_everything(
    const std::vector< std::string >& files1,
    const std::vector< std::string >& files2,
    const std::string& pwd )
{
    clean_files( files1 ) ;
    clean_files( files2 ) ;
    GEO::FileSystem::set_current_working_directory( pwd ) ;
}

void check_gm()
{
    std::string pwd = GEO::FileSystem::get_current_working_directory() ;
    std::vector< std::string > template_files = get_gm_files(
        ringmesh_test_data_path + "save/geomodel.gm" ) ;
    std::vector< std::string > test_files = get_gm_files(
        ringmesh_test_output_path + "geomodel.gm" ) ;

    try {
        for( index_t i = 0; i < template_files.size(); i++ ) {
            check_files( test_files[i], template_files[i] ) ;
        }
    } catch( const RINGMeshException& e ) {
        clean_everything( test_files, template_files, pwd ) ;
        throw e ;
    }
    clean_everything( test_files, template_files, pwd ) ;
}

void check_output( GEO::LineInput& in )
{
    std::string data_path = ringmesh_test_data_path + "save/" ;
    while( !in.eof() && in.get_line() ) {
        in.get_fields() ;
        std::string template_output = data_path + in.field( 0 ) ;
        std::string new_output = ringmesh_test_output_path + in.field( 0 ) ;
        check_files( new_output, template_output ) ;
    }
}

void io_geomodel( const std::string& geomodel_file, const std::string& extension )
{
    GeoModel geomodel ;
    load_geomodel( geomodel, geomodel_file ) ;
    geomodel_save( geomodel, ringmesh_test_output_path + "geomodel." + extension ) ;
}

void process_extension( const std::string& extension )
{
    GEO::LineInput in( ringmesh_test_data_path + "save/" + extension + ".txt" ) ;
    get_line( in ) ;
    io_geomodel( ringmesh_test_data_path + in.field( 0 ), extension ) ;
    if( extension == "gm" ) {
        check_gm() ;
    } else {
        check_output( in ) ;
    }
    Logger::out( "TEST", "Format ", extension, " OK" ) ;
}

int main()
{
    using namespace RINGMesh ;

    try {
        default_configure() ;

        CmdLine::import_arg_group( "out" ) ;

        Logger::out( "TEST", "Save GeoModel files" ) ;
        std::vector< std::string > extensions ;
        GeoModelIOHandlerFactory::list_creators( extensions ) ;
        for( const std::string& extension : extensions ) {
            process_extension( extension ) ;
        }

    } catch( const RINGMeshException& e ) {
        Logger::err( e.category(), e.what() ) ;
        return 1 ;
    } catch( const std::exception& e ) {
        Logger::err( "Exception", e.what() ) ;
        return 1 ;
    }
    Logger::out( "TEST", "SUCCESS" ) ;
    return 0 ;
}
