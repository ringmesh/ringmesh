/*
 * Copyright (c) 2012-2015, Association Scientifique pour la Geologie et ses Applications (ASGA)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the <organization> nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  Contacts:
 *     Arnaud.Botella@univ-lorraine.fr
 *     Antoine.Mazuyer@univ-lorraine.fr
 *     Jeanne.Pellerin@wias-berlin.de
 *
 *     http://www.gocad.org
 *
 *     GOCAD Project
 *     Ecole Nationale Sup�rieure de G�ologie - Georessources
 *     2 Rue du Doyen Marcel Roubault - TSA 70605
 *     54518 VANDOEUVRE-LES-NANCY
 *     FRANCE
 */

//#include <ringmesh/boundary_model.h>
//#include <ringmesh/io.h>
#include <ringmesh/utils.h>
#include <geogram/basic/logger.h>

using namespace RINGMesh ;

namespace {
    bool are_equal_matrices(
        const GEO::Matrix< float64, 4 >& lhs,
        const GEO::Matrix< float64, 4 >& rhs )
    {
        for( index_t mat_i = 0; mat_i < 4; ++mat_i ) {
            for( index_t mat_j = 0; mat_j < 4; ++mat_j ) {
                float64 diff = lhs( mat_i, mat_j ) - rhs( mat_i, mat_j ) ;
                if( diff > epsilon || diff < -epsilon ) {
                    return false ;
                }
            }
        }
        return true ;
    }
}

int main( int argc, char** argv )
{

    GEO::Logger::out( "TEST" )
        << "Test rotations of a boundary model and a macro mesh" << std::endl ;

    const vec3 origin( 0, 0, 0 ) ;
    const vec3 axis( 0, 0, 1 ) ;
    float64 angle = 90 ;
    GEO::Matrix< float64, 4 > rot_mat ;
    Math::rotation_matrix_about_arbitrary_axis( origin, axis, angle, true,
        rot_mat ) ;
    GEO::Matrix< float64, 4 > result ;
    result( 0, 0 ) = 0 ;
    result( 0, 1 ) = -1 ;
    result( 0, 2 ) = 0 ;
    result( 0, 3 ) = 0 ;

    result( 1, 0 ) = 1 ;
    result( 1, 1 ) = 0 ;
    result( 1, 2 ) = 0 ;
    result( 1, 3 ) = 0 ;

    result( 2, 0 ) = 0 ;
    result( 2, 1 ) = 0 ;
    result( 2, 2 ) = 1 ;
    result( 2, 3 ) = 0 ;

    result( 3, 0 ) = 0 ;
    result( 3, 1 ) = 0 ;
    result( 3, 2 ) = 0 ;
    result( 3, 3 ) = 1 ;

    if( !are_equal_matrices( rot_mat, result ) ) {
        GEO::Logger::out( "TEST" ) << "FAILED" << std::endl ;
        return 1 ;
    }

    GEO::Logger::out( "TEST" ) << "SUCCESS" << std::endl ;
    return 0 ;
}
