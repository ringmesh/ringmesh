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

#ifndef __RINGMESH_GEO_MODEL_BUILDER_FROM_MESH__
#define __RINGMESH_GEO_MODEL_BUILDER_FROM_MESH__

#include <ringmesh/basic/common.h>

#include <vector>
#include <string>
#include <stack>

#include <ringmesh/geomodel/geo_model_builder.h>

/*!
 * @file ringmesh/geo_model_builder_from_mesh.h
 * @brief Classes to build GeoModel from Geogram meshes
 * @author Jeanne Pellerin
 */

namespace RINGMesh {
    // Implementation class
    class GeoModelEntityFromMesh ;

    /*!
     * @brief To build a GeoModel from a set of disconnected polygonal surfaces
     */
    class RINGMESH_API GeoModelBuilderSurfaceMesh: public GeoModelBuilder {
    public:
        GeoModelBuilderSurfaceMesh( GeoModel& geomodel, const GEO::Mesh& mesh )
            : GeoModelBuilder( geomodel ), mesh_( mesh )
        {
            options_.compute_lines = true ;
            options_.compute_corners = true ;
            options_.compute_regions_brep = true ;
        }
        void build_polygonal_surfaces_from_connected_components() ;

    private:
        const GEO::Mesh& mesh_ ;
    } ;
}

#endif
