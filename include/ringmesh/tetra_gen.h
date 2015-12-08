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
 *     Ecole Nationale Superieure de Geologie - GeoRessources
 *     2 Rue du Doyen Marcel Roubault - TSA 70605
 *     54518 VANDOEUVRE-LES-NANCY 
 *     FRANCE
*/

#ifndef __RINGMESH_TETRA_GEN__
#define __RINGMESH_TETRA_GEN__

#include <ringmesh/common.h>
#include <ringmesh/utils.h>

#include <geogram/basic/counted.h>
#include <geogram/basic/smart_pointer.h>
#include <geogram/basic/factory.h>

#include <vector>

/*!
* @file ringmesh/tetragen.h
* @brief API class interfacing GeoModel with external tetrahedral meshers 
* @author Arnaud Botella
*/

#ifdef USE_MG_TETRA
    extern "C" {
        #include <meshgems/meshgems.h>
        #include <meshgems/tetra.h>
    }
#endif

namespace GEO {
    class Mesh ;
}

namespace RINGMesh {
    class GeoModelElement ;
    class TetraGen ;
    class WellGroup ;
}

namespace RINGMesh {

    class RINGMESH_API TetraGen: public GEO::Counted {
        ringmesh_disable_copy( TetraGen ) ;
    public:
        virtual ~TetraGen() ;
        static TetraGen* create( GEO::Mesh& tetmesh, const std::string& algo_name ) ;
        static void initialize() ;

        void set_boundaries( const GeoModelElement& region, const WellGroup* wells = nil ) ;
        void set_internal_points( const std::vector< vec3 >& points ) ;

        /*!
         * @brief Send the set of points/edges/triangles to MGTetra or TetGen
         * @details A set of points/edges/triangles are given to MGtetra or Tetgen
         * The two mesh generators are configurated. Then check and repair functions
         * are launched in order to control the outputs
         * @param[in] refine tells whether or not there are refined options to set (true by defaults)
         */
        virtual bool tetrahedralize( bool refine = true ) = 0 ;

    protected:
        TetraGen() ;

        void initialize_storage( index_t nb_points, index_t nb_tets ) ;
        void set_point( index_t index, const double* point ) ;
        void set_tetra( index_t index, int* tet, index_t nb_lines, index_t nb_triangles ) ;

    private:
        void set_mesh( GEO::Mesh& tetmesh ) ;

    protected:
        GEO::Mesh* tetmesh_ ;
        const GeoModelElement* region_ ;
        const WellGroup* wells_ ;
    } ;

    typedef GEO::SmartPointer< TetraGen > TetraGen_var ;
    typedef GEO::Factory0< TetraGen > TetraGenFactory;

#define ringmesh_register_tetragen(type, name) \
    geo_register_creator(TetraGenFactory, type, name)
}

#endif
