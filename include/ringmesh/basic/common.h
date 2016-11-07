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

#ifndef __RINGMESH_COMMON__
#define __RINGMESH_COMMON__

/* Include the configuration file generated by CMAKE
 from cmake config options and the
 include/ringmesh/config_ringmesh.h.in file.
 File is in RINGMESH_BIN/ringmesh directory.
 */
#include <ringmesh/ringmesh_config.h>

#if defined( _WIN32 )
#    ifndef WIN32
#        define WIN32
#    endif
#endif

#ifdef WIN32
#   ifdef RINGMESH_EXPORTS
#        define RINGMESH_API __declspec( dllexport )
#    else
#        define RINGMESH_API __declspec( dllimport )
#    endif
#else
#   define RINGMESH_API
#endif

#ifndef NDEBUG
#   define RINGMESH_DEBUG
#else
#   undef RINGMESH_DEBUG
#endif

#ifdef WIN32
#   pragma warning( disable: 4267 )
#   pragma warning( disable: 4251 ) // dll interface warnings
#   pragma warning( disable: 4275 ) // let's pray we have no issues
#endif

#ifdef USE_OPENMP
#   ifdef WIN32
#       define RINGMESH_PARALLEL_LOOP __pragma("omp parallel for")
#       define RINGMESH_PARALLEL_LOOP_DYNAMIC __pragma( "omp parallel for schedule(dynamic)" )
#   else
#       define RINGMESH_PARALLEL_LOOP _Pragma("omp parallel for")
#       define RINGMESH_PARALLEL_LOOP_DYNAMIC _Pragma( "omp parallel for schedule(dynamic)" )
#   endif
#else
#   define RINGMESH_PARALLEL_LOOP
#   define RINGMESH_PARALLEL_LOOP_DYNAMIC
#endif

#define ringmesh_disable_copy( Class ) \
    private: \
    Class( const Class & ) ; \
    Class& operator=( const Class& )

// To avoid unused argument warning in function definition
template< typename T > inline void ringmesh_unused( T const& )
{
}

#include <omp.h>

#include <ringmesh/basic/types.h>
#include <ringmesh/basic/ringmesh_assert.h>

#include <geogram/basic/logger.h>

#define DEBUG( a ) \
    Logger::out( "Debug" ) << #a << " = " << a << std::endl

#include <stdexcept>

namespace RINGMesh {

    void RINGMESH_API configure_geogram() ;
    void RINGMESH_API configure_ringmesh() ;
    void RINGMESH_API default_configure() ;

    /*!
     * RINGMesh exception class.
     * Example:
     *       throw RINGMeshException( "I/O", "Error while loading the model" ) ;
     *
     *       try {
     *          ...
     *       } catch( const RINGMeshException& e ) {
     *          Logger::err( e.category() ) << e.what() << std::endl ;
     *       } catch( const std::exception& e ) {
     *          // Catch all others STL exceptions
     *          Logger::err( "Exception" ) << e.what() << std::endl;
     *       }
     */
    class RINGMESH_API RINGMeshException: public std::runtime_error {
    public:
        explicit RINGMeshException(
            const std::string& category,
            const std::string& message )
            : std::runtime_error( message ), category_( category )
        {
        }
        virtual ~RINGMeshException() throw()
        {
        }

        const std::string& category() const
        {
            return category_ ;
        }
    protected:
        std::string category_ ;
    } ;
}

#endif

