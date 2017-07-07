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

#include <ringmesh/basic/matrix.h>
#include <geogram/basic/string.h>

/*!
 * @author Benjamin Chauvin
 */

using namespace RINGMesh;

void set_get_test( SparseMatrix< double, light >& mat )
{
    mat.set_element( 0, 0, 1. );
    mat.set_element( 1, 1, 1. );
    mat.set_element( 2, 2, 1. );
    double elt;
    for( index_t i = 0; i < mat.ni(); ++i ) {
        if( !mat.get_element( i, i, elt )
            || std::abs( elt - 1. ) > global_epsilon ) {
            throw RINGMeshException( "TEST",
                "Matrix element at (" + GEO::String::to_string( i ) + ","
                    + GEO::String::to_string( i )
                    + ") should exist and be correct!" );
        }
        for( index_t j = 0; j < mat.nj(); ++j ) {
            if( i != j ) {
                if( mat.get_element( i, j, elt ) ) {
                    throw RINGMeshException( "TEST",
                        "Matrix element at (" + GEO::String::to_string( i ) + ","
                            + GEO::String::to_string( i ) + ") should not exist!" );
                }
            }
        }
    }
}

void symmetry_test( SparseMatrix< double, light >& mat )
{
    mat.set_element( 0, 1, 3. );
    double elt;
    if( !mat.get_element( 0, 1, elt ) || std::abs( elt - 3. ) > global_epsilon ) {
        throw RINGMeshException( "TEST",
            "Matrix element at (0,1) should not exist and be correct!" );
    }
    if( mat.is_symmetrical() ) {
        if( !mat.get_element( 1, 0, elt )
            || std::abs( elt - 3. ) > global_epsilon ) {
            throw RINGMeshException( "TEST",
                "Matrix element at (1,0) should exist and be correct!" );
        }
    } else {
        if( mat.get_element( 1, 0, elt ) && std::abs( elt - 3. ) < global_epsilon ) {
            throw RINGMeshException( "TEST",
                "Matrix element at (1,0) should not exist and the matrix should not be symmetrical!" );
        }
    }
}

void product_by_vector_test(
    SparseMatrix< double, light >& mat,
    const std::vector< double >& product_result )
{
    std::vector< double > vect_to_multiply( 3 );
    vect_to_multiply[0] = 5.;
    vect_to_multiply[1] = 2.;
    vect_to_multiply[2] = 9.;
    std::vector< double > result = product_matrix_by_vector< double >( mat,
        vect_to_multiply );
    if( std::abs( result[0] - product_result[0] ) > global_epsilon
        || std::abs( result[1] - product_result[1] ) > global_epsilon
        || std::abs( result[2] - product_result[2] ) > global_epsilon ) {
        throw RINGMeshException( "TEST", "Product matrix by vector failed!" );
    }
}

void one_test_seq(
    SparseMatrix< double, light >& mat,
    const std::vector< double >& product_result )
{
    mat.build_matrix( 3, 3 );
    set_get_test( mat );
    symmetry_test( mat );
    product_by_vector_test( mat, product_result );
}

void run_tests()
{
    SparseMatrix< double, light > sym_mat( true );
    std::vector< double > product_result( 3 );
    product_result[0] = 11.;
    product_result[1] = 17.;
    product_result[2] = 9.;
    one_test_seq( sym_mat, product_result );

    SparseMatrix< double, light > asym_mat( false );
    product_result[1] = 2.;
    one_test_seq( asym_mat, product_result );

}

int main()
{
    using namespace RINGMesh;

    try {
        default_configure();

        Logger::out( "TEST", "Test Matrix" );
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
