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

#include <ringmesh/io/io.h>

#include <ringmesh/geomodel/geomodel.h>
#include <ringmesh/geomodel/geomodel_api.h>
#include <ringmesh/geomodel/geomodel_builder.h>
#include <ringmesh/geomodel/geomodel_builder_gocad.h>
#include <ringmesh/geomodel/geomodel_entity.h>
#include <ringmesh/geomodel/geomodel_validity.h>

/*!
 * @file Implementation of classes to load and save surface GeoModels meshes
 * @author various
 */

namespace {
    using namespace RINGMesh;

#ifdef RINGMESH_WITH_GEOLOGYJS
#    include "boundary_geomodel/io_html.cpp"
#endif

#include "boundary_geomodel/io_model3d.cpp"
#include "boundary_geomodel/io_smesh.cpp"
#include "boundary_geomodel/io_stl.cpp"

}
/************************************************************************/
namespace RINGMesh {
    /*
     * Initializes the possible handlers for IO GeoModel files
     */
    template< >
    void GeoModelIOHandler< 2 >::initialize_boundary_geomodel_output()
    {
    }

    template< >
    void GeoModelIOHandler< 3 >::initialize_boundary_geomodel_output()
    {
        ringmesh_register_GeoModelIOHandler3D_creator( MLIOHandler, "ml" );
        ringmesh_register_GeoModelIOHandler3D_creator( SMESHIOHandler, "smesh" );
        ringmesh_register_GeoModelIOHandler3D_creator( STLIOHandler, "stl" );
#ifdef RINGMESH_WITH_GEOLOGYJS
        ringmesh_register_GeoModelIOHandler_creator( HTMLIOHandler, "html" );
#endif
    }

}
