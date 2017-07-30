/*
 * Copyright (c) 2012-2017, Association Scientifique pour la Geologie et ses
 * Applications (ASGA) All rights reserved.
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
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL ASGA BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
 * @file Declaration of GeoModelGeologicalEntity and all its children classes
 * @author Jeanne Pellerin and Arnaud Botella
 */

#pragma once

#include <ringmesh/basic/common.h>

#include <memory>

#include <geogram/basic/factory.h>

#include <ringmesh/geomodel/geomodel_entity.h>
#include <ringmesh/geomodel/geomodel_indexing_types.h>

namespace RINGMesh {
template <index_t DIMENSION> class GeoModel;
template <index_t DIMENSION> class GeoModelMeshEntity;
template <index_t DIMENSION> class GeoModelBuilderTopology;
template <index_t DIMENSION> class GeoModelBuilderGeology;
template <index_t DIMENSION> class GeoModelBuilderRemovalBase;
template <index_t DIMENSION> class GeoModelBuilderRemoval;
template <index_t DIMENSION> class GeoModelBuilderInfo;
}

namespace RINGMesh {
template <index_t DIMENSION>
class GeoModelGeologicalEntity : public GeoModelEntity<DIMENSION> {
    ringmesh_disable_copy(GeoModelGeologicalEntity);
    ringmesh_template_assert_2d_or_3d(DIMENSION);

public:
    friend class GeoModelGeologicalEntityAccess<DIMENSION>;

    /*!
     * @brief Geological feature types for GeoModelEntity
     */
    enum struct GEOL_FEATURE {
        /// All geological features
        ALL_GEOL,
        /// Default value - No geological feature defined
        NO_GEOL,
        /// Stratigraphic surface - an horizon
        STRATI,
        /// Unconformity
        UNCONFORMITY,
        /// A normal fault
        NORMAL_FAULT,
        /// A reverse fault
        REVERSE_FAULT,
        /// An unspecified fault
        FAULT,
        /// Volume Of Interest
        VOI
    };

    /*!
     * @brief Map the name of a geological type with a value of GEOL_FEATURE
     *
     * @param[in] in Name of the feature. Can be
     * \li "reverse_fault"
     * \li "normal_fault"
     * \li "fault"
     * \li "top"
     * \li "none"
     * \li "topographic"
     * \li "unconformity"
     * \li "boundary"
     * Other strings will end up in \p NO_GEOL
     * @return The geological feature index
     * @todo Add other types of unconformity, see
     * RINGMesh::GeoModelEntity::TYPE. --GC
     */
    static GEOL_FEATURE determine_geological_type(const std::string& in);
    /*!
     * \return the (lowercase) string associated to a
     * GeoModelELement::GEOL_FEATURE
     */
    static std::string geol_name(GEOL_FEATURE feature);
    static bool is_fault(GEOL_FEATURE feature);
    static bool is_stratigraphic_limit(GEOL_FEATURE feature)
    {
        return feature == GEOL_FEATURE::STRATI
            || feature == GEOL_FEATURE::UNCONFORMITY;
    }

    bool has_geological_feature() const
    {
        return geological_feature() != GEOL_FEATURE::NO_GEOL;
    }

    GEOL_FEATURE geological_feature() const { return geol_feature_; }

    static void initialize();

    virtual ~GeoModelGeologicalEntity() = default;

    gmge_id gmge() const { return gmge_id(type_name(), this->index()); }

    GeologicalEntityType entity_type() const { return gmge().type(); }

    virtual MeshEntityType child_type_name() const = 0;
    virtual bool is_on_voi() const;
    virtual bool is_connectivity_valid() const;
    virtual bool is_valid() const;
    static GeologicalEntityType type_name_static()
    {
        return ForbiddenGeologicalEntityType::type_name_static();
    }
    virtual GeologicalEntityType type_name() const
    {
        return type_name_static();
    }
    index_t nb_children() const
    {
        return static_cast<index_t>(children_.size());
    }
    const gmme_id& child_gmme(index_t x) const;
    const GeoModelMeshEntity<DIMENSION>& child(index_t x) const;

    virtual bool is_identification_valid() const;

protected:
    GeoModelGeologicalEntity(const GeoModel<DIMENSION>& geomodel)
        : GeoModelEntity<DIMENSION>(geomodel, NO_ID)
    {
    }

    virtual bool is_index_valid() const;

private:
    void copy_geological_entity(const GeoModelGeologicalEntity<DIMENSION>& from)
    {
        this->copy_name(from);
        geol_feature_ = from.geol_feature_;
        children_ = from.children_;
    }

protected:
    /// Children relations of this entity
    std::vector<index_t> children_;

    /// Geological feature of this object - default is NO_GEOL
    GEOL_FEATURE geol_feature_{ GEOL_FEATURE::NO_GEOL };
};

CLASS_DIMENSION_ALIASES(GeoModelGeologicalEntity);

template <index_t DIMENSION>
using GeoModelGeologicalEntityFactory
    = GEO::Factory1<GeoModelGeologicalEntity<DIMENSION>, GeoModel<DIMENSION>>;

CLASS_DIMENSION_ALIASES(GeoModelGeologicalEntityFactory);

#define ringmesh_register_GeoModelGeologicalEntity2D_creator(type)             \
    geo_register_creator(                                                      \
        GeoModelGeologicalEntityFactory2D, type, type::type_name_static())

#define ringmesh_register_GeoModelGeologicalEntity3D_creator(type)             \
    geo_register_creator(                                                      \
        GeoModelGeologicalEntityFactory3D, type, type::type_name_static())

template <index_t DIMENSION>
class Contact : public GeoModelGeologicalEntity<DIMENSION> {
    ringmesh_template_assert_2d_or_3d(DIMENSION);

public:
    Contact(const GeoModel<DIMENSION>& geomodel)
        : GeoModelGeologicalEntity<DIMENSION>(geomodel)
    {
    }
    virtual ~Contact() = default;

    static GeologicalEntityType type_name_static()
    {
        return GeologicalEntityType("Contact");
    }
    GeologicalEntityType type_name() const override
    {
        return type_name_static();
    }
    MeshEntityType child_type_name() const override;
};

CLASS_DIMENSION_ALIASES(Contact);

template <index_t DIMENSION>
class Interface : public GeoModelGeologicalEntity<DIMENSION> {
    ringmesh_template_assert_2d_or_3d(DIMENSION);

public:
    Interface(const GeoModel<DIMENSION>& geomodel)
        : GeoModelGeologicalEntity<DIMENSION>(geomodel)
    {
    }
    virtual ~Interface() = default;

    static GeologicalEntityType type_name_static()
    {
        return GeologicalEntityType("Interface");
    }
    GeologicalEntityType type_name() const override
    {
        return type_name_static();
    }
    MeshEntityType child_type_name() const override;
};

CLASS_DIMENSION_ALIASES(Interface);

template <index_t DIMENSION>
class Layer : public GeoModelGeologicalEntity<DIMENSION> {
    ringmesh_template_assert_2d_or_3d(DIMENSION);

public:
    Layer(const GeoModel<DIMENSION>& geomodel)
        : GeoModelGeologicalEntity<DIMENSION>(geomodel)
    {
    }
    virtual ~Layer() = default;

    static GeologicalEntityType type_name_static()
    {
        return GeologicalEntityType("Layer");
    }
    GeologicalEntityType type_name() const override
    {
        return type_name_static();
    }
    MeshEntityType child_type_name() const override;
};

CLASS_DIMENSION_ALIASES(Layer);

template <index_t DIMENSION> class GeoModelGeologicalEntityAccess {
    ringmesh_disable_copy(GeoModelGeologicalEntityAccess);
    friend class GeoModelBuilderTopology<DIMENSION>;
    friend class GeoModelBuilderGeology<DIMENSION>;
    friend class GeoModelBuilderInfo<DIMENSION>;
    friend class GeoModelBuilderRemovalBase<DIMENSION>;

private:
    GeoModelGeologicalEntityAccess(GeoModelGeologicalEntity<DIMENSION>& gmge)
        : gmge_(gmge)
    {
    }

    std::string& modifiable_name() { return gmge_.name_; }

    index_t& modifiable_index() { return gmge_.id_; }

    typename GeoModelGeologicalEntity<DIMENSION>::GEOL_FEATURE&
    modifiable_geol_feature()
    {
        return gmge_.geol_feature_;
    }

    std::vector<index_t>& modifiable_children() { return gmge_.children_; }

    static std::unique_ptr<GeoModelGeologicalEntity<DIMENSION>>
    create_geological_entity(const GeologicalEntityType& type,
        const GeoModel<DIMENSION>& geomodel, index_t index_in_geomodel);

    void copy(const GeoModelGeologicalEntity<DIMENSION>& from)
    {
        gmge_.copy_geological_entity(from);
    }

private:
    GeoModelGeologicalEntity<DIMENSION>& gmge_;
};
}
