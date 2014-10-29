/*[
 * Association Scientifique pour la Geologie et ses Applications (ASGA)
 * Copyright (c) 1993-2013 ASGA. All Rights Reserved.
 *
 * This program is a Trade Secret of the ASGA and it is not to be:
 * - reproduced, published, or disclosed to other,
 * - distributed or displayed,
 * - used for purposes or on Sites other than described
 *   in the GOCAD Advancement Agreement,
 * without the prior written authorization of the ASGA. Licencee
 * agrees to attach or embed this Notice on all copies of the program,
 * including partial copies or modified versions thereof.
 ]*/

#ifndef __GRGMESH_PERMUTATION__
#define __GRGMESH_PERMUTATION__

#include <grgmeshlib/common.h>

namespace GRGMesh {

    namespace Permutation {

        /**
         * @brief checks whether a specified vector encodes
         *  a valid permutation.
         */
        inline bool is_valid( const std::vector< int32 >& permutation )
        {
            std::vector< bool > visited( permutation.size(), false ) ;
            for( uint32 i = 0; i < permutation.size(); i++ ) {
                if( permutation[i] < 0
                    || permutation[i] >= int32( permutation.size() ) ) {
                    return false ;
                }
                if( visited[i] ) {
                    return false ;
                }
                visited[i] = true ;
            }
            return true ;
        }

        /**
         * @brief used internally by apply_permutation()
         */
        inline bool is_marked( const std::vector< int32 >& permutation, int32 i )
        {
            grgmesh_debug_assert( i >= 0 && i < int32( permutation.size() ) ) ;
            return ( permutation[i] < 0 ) ;
        }

        /**
         * @brief used internally by apply_permutation()
         */
        inline void mark( std::vector< int32 >& permutation, int32 i )
        {
            grgmesh_debug_assert( i >= 0 && i < int32( permutation.size() ) ) ;
            grgmesh_debug_assert( !is_marked( permutation, i ) ) ;
            permutation[i] = -permutation[i] - 1 ;
        }

        /**
         * @brief used internally by apply_permutation()
         */
        inline void unmark( std::vector< int32 >& permutation, int32 i )
        {
            grgmesh_debug_assert( i >= 0 && i < int32( permutation.size() ) ) ;
            grgmesh_debug_assert( is_marked( permutation, i ) ) ;
            permutation[i] = -permutation[i] - 1 ;
        }

        /**
         * @brief applies a permutation in-place.
         * It is equivalent to:
         * @code
         * for(i=0; i<permutation.size(); i++) {
         *    data2[i] = data[permutation[i]]
         * }
         * data = data2 ;
         * @endcode
         * @param data the vector to permute
         * @param [in] permutation_in the permutation. 
         *  It is temporarily changed during execution of the 
         *  function, but identical to the input on exit.
         */
        inline void apply(
            void* data_in,
            std::vector< int32 >& permutation_in,
            uint32 elemsize )
        {
            pointer data = (pointer) ( data_in ) ;
            std::vector< int32 >& permutation =
                const_cast< std::vector< int32 >& >( permutation_in ) ;
            grgmesh_debug_assert( is_valid( permutation ) ) ;
            pointer temp = static_cast< pointer >( alloca( elemsize ) ) ;
            for( uint32 k = 0; k < permutation.size(); k++ ) {
                if( is_marked( permutation, k ) ) {
                    continue ;
                }
                int32 i = k ;
                std::memcpy( temp, data + i * elemsize, elemsize ) ;
                int32 j = permutation[k] ;
                mark( permutation, k ) ;
                while( j != int32( k ) ) {
                    std::memcpy( data + i * elemsize, data + j * elemsize,
                        elemsize ) ;
                    int32 nj = permutation[j] ;
                    mark( permutation, j ) ;
                    i = j ;
                    j = nj ;
                }
                std::memcpy( data + i * elemsize, temp, elemsize ) ;
            }
            for( uint32 k = 0; k < permutation.size(); k++ ) {
                unmark( permutation, k ) ;
            }
        }

        /**
         * @brief applies a permutation in-place.
         * It is equivalent to:
         * @code
         * for(i=0; i<permutation.size(); i++) {
         *    data2[i] = data[permutation[i]]
         * }
         * data = data2 ;
         * @endcode
         * @param data the vector to permute
         * @param [in] permutation_in the permutation. 
         *  It is temporarily changed during execution of the 
         *  function, but identical to the input on exit.
         */
        template< class T > inline void apply(
            std::vector< T >& data,
            std::vector< int32 >& permutation_in )
        {
            std::vector< int32 >& permutation =
                const_cast< std::vector< int32 >& >( permutation_in ) ;
            grgmesh_debug_assert( is_valid( permutation ) ) ;
            T temp ;
            for( uint32 k = 0; k < permutation.size(); k++ ) {
                if( is_marked( permutation, k ) ) {
                    continue ;
                }
                int32 i = k ;
                temp = data[i] ;
                int32 j = permutation[k] ;
                mark( permutation, k ) ;
                while( j != int32( k ) ) {
                    data[i] = data[j] ;
                    int32 nj = permutation[j] ;
                    mark( permutation, j ) ;
                    i = j ;
                    j = nj ;
                }
                data[i] = temp ;
            }
            for( uint32 k = 0; k < permutation.size(); k++ ) {
                unmark( permutation, k ) ;
            }
        }

        /**
         * @brief inverts a permutation in-place.
         * It is equivalent to:
         * @code
         * for(index_t i=0; i<permutation.size()(); i++) {
         *      perminv[permutation[i]] = i ;
         * }
         * permutation = perminv ;
         * @endcode
         */
        inline void invert( std::vector< int32 >& permutation )
        {
            grgmesh_debug_assert( is_valid( permutation ) ) ;
            for( uint32 k = 0; k < permutation.size(); k++ ) {
                if( is_marked( permutation, k ) ) {
                    continue ;
                }
                int32 i = k ;
                int32 j = permutation[i] ;
                while( j != int32( k ) ) {
                    int32 temp = permutation[j] ;
                    permutation[j] = i ;
                    mark( permutation, j ) ;
                    i = j ;
                    j = temp ;
                }
                permutation[j] = i ;
                mark( permutation, j ) ;
            }
            for( uint32 k = 0; k < permutation.size(); k++ ) {
                unmark( permutation, k ) ;
            }
        }

    }

}

#endif
