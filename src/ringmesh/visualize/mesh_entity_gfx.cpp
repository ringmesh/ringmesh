/*
 * Copyright (c) 2012-2018, Association Scientifique pour la Geologie et ses
 * Applications (ASGA). All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL ASGA BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *     http://www.ring-team.org
 *
 *     RING Project
 *     Ecole Nationale Superieure de Geologie - GeoRessources
 *     2 Rue du Doyen Marcel Roubault - TSA 70605
 *     54518 VANDOEUVRE-LES-NANCY
 *     FRANCE
 */

/*!
 * @file Implementation of visualization of GeoModelEntities
 * @author Benjamin Chauvin and Arnaud Botella
 */

#include <ringmesh/visualize/mesh_entity_gfx.h>

#ifdef RINGMESH_WITH_GRAPHICS

#include <ringmesh/geomodel/core/geomodel.h>
#include <ringmesh/geomodel/core/geomodel_entity.h>
#include <ringmesh/geomodel/core/geomodel_geological_entity.h>
#include <ringmesh/geomodel/core/geomodel_mesh_entity.h>

#include <ringmesh/visualize/geogram_gfx.h>
#include <ringmesh/visualize/geomodel_gfx.h>

namespace
{
    using namespace RINGMesh;

    std::string get_attribute_name_with_coordinate(
        const std::string& name, index_t coordinate )
    {
        return name + "[" + std::to_string( coordinate ) + "]";
    }

    void compute_attribute_range(
        GEO::ReadOnlyScalarAttributeAdapter& attribute,
        double& min,
        double& max )
    {
        if( attribute.is_bound() )
        {
            for( auto i : range( attribute.size() ) )
            {
                double value = attribute[i];
                min = std::min( min, value );
                max = std::max( max, value );
            }
        }
    }
} // namespace

namespace RINGMesh
{
    template < index_t DIMENSION >
    class CellAttributeGfx : public AttributeGfx< DIMENSION >
    {
    public:
        CellAttributeGfx() = default;

        virtual ~CellAttributeGfx() = default;

        static std::string location_name_static()
        {
            return "cells";
        }

        std::string location_name() const override
        {
            return location_name_static();
        }

        void bind_attribute() override
        {
            std::string attribute_name = get_attribute_name_with_coordinate(
                this->manager_->name(), this->manager_->coordinate() );
            this->manager_->gfx().regions.set_scalar_attribute( GEO::MESH_CELLS,
                attribute_name, this->manager_->minimum(),
                this->manager_->maximum(), this->manager_->colormap() );
        }

        void unbind_attribute() override
        {
            this->manager_->gfx().regions.unset_scalar_attribute();
        }

        index_t nb_coordinates() override
        {
            GEO::AttributeStore* store =
                get_attribute_manager().find_attribute_store(
                    this->manager_->name() );

            if( store == nullptr )
                return 0;
            return store->dimension();
        }

        GEO::AttributesManager& get_attribute_manager() override
        {
            const GeoModel< DIMENSION >* geomodel =
                this->manager_->gfx().geomodel();

            index_t entity_index = 0;
            index_t max_nb_attributes = 0;
            for( auto e : range( geomodel->nb_regions() ) )
            {
                if( geomodel->region( e ).cell_attribute_manager().nb()
                    > max_nb_attributes )
                {
                    entity_index = e;
                    max_nb_attributes =
                        geomodel->region( e ).cell_attribute_manager().nb();
                }
            }
            return geomodel->region( entity_index ).cell_attribute_manager();
        }

    private:
        void do_compute_range(
            double& attribute_min, double& attribute_max ) override
        {
            std::string attribute_name = get_attribute_name_with_coordinate(
                this->manager_->name(), this->manager_->coordinate() );
            const GeoModel< DIMENSION >* geomodel =
                this->manager_->gfx().geomodel();
            for( const auto& region : geomodel->regions() )
            {
                GEO::ReadOnlyScalarAttributeAdapter attribute(
                    region.cell_attribute_manager(), attribute_name );
                compute_attribute_range(
                    attribute, attribute_min, attribute_max );
            }
        }
    };

    template < index_t DIMENSION >
    class CellVertexAttributeGfx : public AttributeGfx< DIMENSION >
    {
    public:
        CellVertexAttributeGfx() = default;

        virtual ~CellVertexAttributeGfx() = default;

        static std::string location_name_static()
        {
            return "cell_vertices";
        }

        std::string location_name() const override
        {
            return location_name_static();
        }

        void bind_attribute() override
        {
            std::string attribute_name = get_attribute_name_with_coordinate(
                this->manager_->name(), this->manager_->coordinate() );
            this->manager_->gfx().regions.set_scalar_attribute(
                GEO::MESH_VERTICES, attribute_name, this->manager_->minimum(),
                this->manager_->maximum(), this->manager_->colormap() );
        }

        void unbind_attribute() override
        {
            this->manager_->gfx().regions.unset_scalar_attribute();
        }

        index_t nb_coordinates() override
        {
            GEO::AttributeStore* store =
                get_attribute_manager().find_attribute_store(
                    this->manager_->name() );

            if( store == nullptr )
                return 0;
            return store->dimension();
        }

        GEO::AttributesManager& get_attribute_manager() override
        {
            const GeoModel< DIMENSION >* geomodel =
                this->manager_->gfx().geomodel();

            index_t entity_index = 0;
            index_t max_nb_attributes = 0;
            for( auto e : range( geomodel->nb_regions() ) )
            {
                if( geomodel->region( e ).vertex_attribute_manager().nb()
                    > max_nb_attributes )
                {
                    entity_index = e;
                    max_nb_attributes =
                        geomodel->region( e ).vertex_attribute_manager().nb();
                }
            }
            return geomodel->region( entity_index ).vertex_attribute_manager();
        }

    private:
        void do_compute_range(
            double& attribute_min, double& attribute_max ) override
        {
            std::string attribute_name = get_attribute_name_with_coordinate(
                this->manager_->name(), this->manager_->coordinate() );
            const GeoModel< DIMENSION >* geomodel =
                this->manager_->gfx().geomodel();
            for( const auto& region : geomodel->regions() )
            {
                GEO::ReadOnlyScalarAttributeAdapter attribute(
                    region.vertex_attribute_manager(), attribute_name );
                compute_attribute_range(
                    attribute, attribute_min, attribute_max );
            }
        }
    };

    template < index_t DIMENSION >
    class PolygonAttributeGfx : public AttributeGfx< DIMENSION >
    {
    public:
        PolygonAttributeGfx() = default;

        virtual ~PolygonAttributeGfx() = default;

        static std::string location_name_static()
        {
            return "polygon";
        }

        std::string location_name() const override
        {
            return location_name_static();
        }

        void bind_attribute() override
        {
            std::string attribute_name = get_attribute_name_with_coordinate(
                this->manager_->name(), this->manager_->coordinate() );
            this->manager_->gfx().surfaces.set_scalar_attribute(
                GEO::MESH_FACETS, attribute_name, this->manager_->minimum(),
                this->manager_->maximum(), this->manager_->colormap() );
        }

        void unbind_attribute() override
        {
            this->manager_->gfx().surfaces.unset_scalar_attribute();
        }

        index_t nb_coordinates() override
        {
            GEO::AttributeStore* store =
                get_attribute_manager().find_attribute_store(
                    this->manager_->name() );

            if( store == nullptr )
                return 0;
            return store->dimension();
        }

        GEO::AttributesManager& get_attribute_manager() override
        {
            const GeoModel< DIMENSION >* geomodel =
                this->manager_->gfx().geomodel();

            index_t entity_index = 0;
            index_t max_nb_attributes = 0;
            for( auto e : range( geomodel->nb_surfaces() ) )
            {
                if( geomodel->surface( e ).polygon_attribute_manager().nb()
                    > max_nb_attributes )
                {
                    entity_index = e;
                    max_nb_attributes =
                        geomodel->surface( e ).polygon_attribute_manager().nb();
                }
            }
            return geomodel->surface( entity_index )
                .polygon_attribute_manager();
        }

    private:
        void do_compute_range(
            double& attribute_min, double& attribute_max ) override
        {
            std::string attribute_name = get_attribute_name_with_coordinate(
                this->manager_->name(), this->manager_->coordinate() );
            const GeoModel< DIMENSION >* geomodel =
                this->manager_->gfx().geomodel();
            for( const auto& surface : geomodel->surfaces() )
            {
                GEO::ReadOnlyScalarAttributeAdapter attribute(
                    surface.polygon_attribute_manager(), attribute_name );
                compute_attribute_range(
                    attribute, attribute_min, attribute_max );
            }
        }
    };

    template < index_t DIMENSION >
    class PolygonVertexAttributeGfx : public AttributeGfx< DIMENSION >
    {
    public:
        PolygonVertexAttributeGfx() = default;

        virtual ~PolygonVertexAttributeGfx() = default;

        static std::string location_name_static()
        {
            return "polygon_vertices";
        }

        std::string location_name() const override
        {
            return location_name_static();
        }
        void bind_attribute() override
        {
            std::string attribute_name = get_attribute_name_with_coordinate(
                this->manager_->name(), this->manager_->coordinate() );
            this->manager_->gfx().surfaces.set_scalar_attribute(
                GEO::MESH_VERTICES, attribute_name, this->manager_->minimum(),
                this->manager_->maximum(), this->manager_->colormap() );
        }
        void unbind_attribute() override
        {
            this->manager_->gfx().surfaces.unset_scalar_attribute();
        }
        index_t nb_coordinates() override
        {
            GEO::AttributeStore* store =
                get_attribute_manager().find_attribute_store(
                    this->manager_->name() );

            if( store == nullptr )
                return 0;
            return store->dimension();
        }

        GEO::AttributesManager& get_attribute_manager() override
        {
            const GeoModel< DIMENSION >* geomodel =
                this->manager_->gfx().geomodel();

            index_t entity_index = 0;
            index_t max_nb_attributes = 0;
            for( auto e : range( geomodel->nb_surfaces() ) )
            {
                if( geomodel->surface( e ).vertex_attribute_manager().nb()
                    > max_nb_attributes )
                {
                    entity_index = e;
                    max_nb_attributes =
                        geomodel->surface( e ).vertex_attribute_manager().nb();
                }
            }
            return geomodel->surface( entity_index ).vertex_attribute_manager();
        }

    private:
        void do_compute_range(
            double& attribute_min, double& attribute_max ) override
        {
            std::string attribute_name = get_attribute_name_with_coordinate(
                this->manager_->name(), this->manager_->coordinate() );
            const GeoModel< DIMENSION >* geomodel =
                this->manager_->gfx().geomodel();
            for( const auto& surface : geomodel->surfaces() )
            {
                GEO::ReadOnlyScalarAttributeAdapter attribute(
                    surface.vertex_attribute_manager(), attribute_name );
                compute_attribute_range(
                    attribute, attribute_min, attribute_max );
            }
        }
    };

    template < index_t DIMENSION >
    AttributeGfxManagerBase< DIMENSION >::AttributeGfxManagerBase(
        GeoModelGfx< DIMENSION >& gfx )
        : gfx_( gfx )
    {
        register_attribute_location< PolygonVertexAttributeGfx >();
        register_attribute_location< PolygonAttributeGfx >();
    }

    template < index_t DIMENSION >
    std::string AttributeGfxManagerBase< DIMENSION >::location_name() const
    {
        if( attribute_ )
        {
            return attribute_->location_name();
        }
        else
        {
            return "location";
        }
    }

    template < index_t DIMENSION >
    void AttributeGfxManagerBase< DIMENSION >::compute_range()
    {
        if( attribute_ )
        {
            attribute_->compute_range();
        }
    }

    template < index_t DIMENSION >
    void AttributeGfxManagerBase< DIMENSION >::bind_attribute()
    {
        if( attribute_ )
        {
            attribute_->bind_attribute();
        }
    }

    template < index_t DIMENSION >
    std::vector< std::string >
        AttributeGfxManagerBase< DIMENSION >::get_attribute_names()
    {
        if( attribute_ )
        {
            return attribute_->get_attribute_names();
        }
        return std::vector< std::string >();
    }

    template < index_t DIMENSION >
    void AttributeGfxManagerBase< DIMENSION >::unbind_attribute()
    {
        if( attribute_ )
        {
            attribute_->unbind_attribute();
        }
    }

    template < index_t DIMENSION >
    index_t AttributeGfxManagerBase< DIMENSION >::nb_coordinates() const
    {
        if( attribute_ )
        {
            return attribute_->nb_coordinates();
        }
        return 0;
    }

    AttributeGfxManager< 3 >::AttributeGfxManager( GeoModelGfx< 3 >& gfx )
        : AttributeGfxManagerBase< 3 >( gfx )
    {
        this->register_attribute_location< CellVertexAttributeGfx >();
        this->register_attribute_location< CellAttributeGfx >();
    }

    template < index_t DIMENSION >
    std::unique_ptr< PointSetMeshGfx< DIMENSION > >
        PointSetMeshGfx< DIMENSION >::create_gfx(
            const PointSetMesh< DIMENSION >& mesh )
    {
        auto gfx = PointSetMeshGfxFactory< DIMENSION >::create(
            mesh.type_name(), mesh );
        if( !gfx )
        {
            throw RINGMeshException( "PointSetMeshGfx",
                "Could not create mesh gfx data structure: ",
                mesh.type_name() );
        }
        return gfx;
    }

    template < index_t DIMENSION >
    std::unique_ptr< LineMeshGfx< DIMENSION > >
        LineMeshGfx< DIMENSION >::create_gfx(
            const LineMesh< DIMENSION >& mesh )
    {
        auto gfx =
            LineMeshGfxFactory< DIMENSION >::create( mesh.type_name(), mesh );
        if( !gfx )
        {
            throw RINGMeshException( "LineMeshGfx",
                "Could not create mesh gfx data structure: ",
                mesh.type_name() );
        }
        return gfx;
    }

    template < index_t DIMENSION >
    std::unique_ptr< SurfaceMeshGfx< DIMENSION > >
        SurfaceMeshGfx< DIMENSION >::create_gfx(
            const SurfaceMesh< DIMENSION >& mesh )
    {
        auto gfx = SurfaceMeshGfxFactory< DIMENSION >::create(
            mesh.type_name(), mesh );
        if( !gfx )
        {
            throw RINGMeshException( "SurfaceMeshGfx",
                "Could not create mesh gfx data structure: ",
                mesh.type_name() );
        }
        return gfx;
    }

    template < index_t DIMENSION >
    std::unique_ptr< VolumeMeshGfx< DIMENSION > >
        VolumeMeshGfx< DIMENSION >::create_gfx(
            const VolumeMesh< DIMENSION >& mesh )
    {
        auto gfx =
            VolumeMeshGfxFactory< DIMENSION >::create( mesh.type_name(), mesh );
        if( !gfx )
        {
            throw RINGMeshException( "VolumeMeshGfx",
                "Could not create mesh gfx data structure: ",
                mesh.type_name() );
        }
        return gfx;
    }

    template std::unique_ptr< PointSetMeshGfx< 2 > > visualize_api
        PointSetMeshGfx< 2 >::create_gfx( const PointSetMesh< 2 >& );
    template std::unique_ptr< LineMeshGfx< 2 > >
        visualize_api LineMeshGfx< 2 >::create_gfx( const LineMesh< 2 >& );
    template std::unique_ptr< SurfaceMeshGfx< 2 > > visualize_api
        SurfaceMeshGfx< 2 >::create_gfx( const SurfaceMesh< 2 >& );
    template class visualize_api AttributeGfxManagerBase< 2 >;
    template class visualize_api AttributeGfxManager< 2 >;

    template std::unique_ptr< PointSetMeshGfx< 3 > > visualize_api
        PointSetMeshGfx< 3 >::create_gfx( const PointSetMesh< 3 >& );
    template std::unique_ptr< LineMeshGfx< 3 > >
        visualize_api LineMeshGfx< 3 >::create_gfx( const LineMesh< 3 >& );
    template std::unique_ptr< SurfaceMeshGfx< 3 > > visualize_api
        SurfaceMeshGfx< 3 >::create_gfx( const SurfaceMesh< 3 >& );
    template std::unique_ptr< VolumeMeshGfx< 3 > >
        visualize_api VolumeMeshGfx< 3 >::create_gfx( const VolumeMesh< 3 >& );
    template class visualize_api AttributeGfxManagerBase< 3 >;

} // namespace RINGMesh

#endif
