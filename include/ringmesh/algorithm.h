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
*     http://www.ring-team.org
*
*     RING Project
*     Ecole Nationale Superieure de Geologie - Georessources
*     2 Rue du Doyen Marcel Roubault - TSA 70605
*     54518 VANDOEUVRE-LES-NANCY
*     FRANCE
*/

#ifndef __RINGMESH_ALGORITHM__
#define __RINGMESH_ALGORITHM__


#include <vector>
#include <algorithm>

namespace RINGMesh {

    template< class T >
    static std::vector< T > intersect(
        const std::vector< T >& v1,
        const std::vector< T >& v2 )
    {
        std::vector< T > intersect( v1.size() + v2.size() ) ;
        std::vector< index_t >::iterator it ;
        it = std::set_intersection( v1.begin(), v1.end(), v2.begin(), v2.end(),
                                    intersect.begin() ) ;
        intersect.resize( it - intersect.begin() ) ;
        return intersect ;
    }

    template< typename T, typename container >
    static bool contains( const container& v, const T& t, bool sorted = false )
    {
        if( sorted )
            return find_sorted( v, t ) != NO_ID ;
        else
            return find( v, t ) != NO_ID ;
    }

    template< typename T, typename container >
    static index_t find( const container& v, const T& t )
    {
        typename container::const_iterator it = std::find( v.begin(), v.end(),
                                                           t ) ;
        if( it == v.end() )
            return NO_ID ;
        else
            return static_cast< index_t >( it - v.begin() ) ;
    }

    template< typename T, typename container >
    static index_t find_sorted( const container& v, const T& t )
    {
        typename container::const_iterator low = std::lower_bound( v.begin(),
                                                                   v.end(), t ) ;
        if( low == v.end() || t < *low )
            return NO_ID ;
        else
            return static_cast< index_t >( low - v.begin() ) ;
    }

    template< class T1, class T2 >
    static bool inexact_equal( const T1& v1, const T2& v2 )
    {
        for( index_t i = 0; i < 3; i++ ) {
            float64 diff( v1[ i ] - v2[ i ] ) ;
            if( diff > epsilon || diff < -epsilon ) {
                return false ;
            }
        }
        return true ;
    }

    template< class T1, class T2 >
    static bool triple_equal(
        const T1& rhs1,
        const T1& rhs2,
        const T1& rhs3,
        const T2& lhs1,
        const T2& lhs2,
        const T2& lhs3 )
    {
        if( rhs1 == lhs1 ) {
            if( rhs2 == lhs2 && rhs3 == lhs3 ) {
                return true ;
            } else if( rhs2 == lhs3 && rhs3 == lhs2 ) {
                return true ;
            }
        } else if( rhs1 == lhs2 ) {
            if( rhs2 == lhs1 && rhs3 == lhs3 ) {
                return true ;
            } else if( rhs2 == lhs3 && rhs3 == lhs1 ) {
                return true ;
            }
        } else if( rhs1 == lhs3 ) {
            if( rhs2 == lhs1 && rhs3 == lhs2 ) {
                return true ;
            } else if( rhs2 == lhs2 && rhs3 == lhs1 ) {
                return true ;
            }
        }
        return false ;
    }


    /*!
    * Class to sort two vectors using indirect sorting
    */
    template< class T1, class T2 >
    class IndirectSort {
    public:
        IndirectSort( std::vector< T1 >& input, std::vector< T2 >& output )
            : input_( input ), output_( output )
        {

        }
        void sort()
        {
            if( input_.size() < 2 ) return ;
            for( index_t it1 = 0; it1 < input_.size() - 1; it1++ ) {
                index_t ref_index = it1 ;
                T1 ref_value = input_[ it1 ] ;
                for( index_t it2 = it1 + 1; it2 < input_.size(); it2++ ) {
                    index_t new_index = it2 ;
                    T1 new_value = input_[ it2 ] ;
                    if( ref_value > new_value ) {
                        ref_value = new_value ;
                        ref_index = new_index ;
                    }
                }
                std::iter_swap( input_.begin() + it1, input_.begin() + ref_index ) ;
                std::iter_swap( output_.begin() + it1,
                                output_.begin() + ref_index ) ;

            }
        }

    private:
        std::vector< T1 >& input_ ;
        std::vector< T2 >& output_ ;
    } ;

}

#endif
