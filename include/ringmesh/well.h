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
 *     Ecole Nationale Superieure de Geologie - Georessources
 *     2 Rue du Doyen Marcel Roubault - TSA 70605
 *     54518 VANDOEUVRE-LES-NANCY
 *     FRANCE
 */

#ifndef __RINGMESH_WELL_
#define __RINGMESH_WELL_

#include <ringmesh/common.h>

#include <geogram/mesh/mesh.h>

namespace RINGMesh {
    class BoundaryModel ;
    class Edge ;
    class Well ;
}

namespace RINGMesh {

    class RINGMESH_API WellMesh {
        ringmesh_disable_copy( WellMesh ) ;
    protected:
        WellMesh( const Well* well ) ;
        virtual ~WellMesh() {}

    public:
        /*!
         * Gets the associated well
         */
        const Well& well() const { return *well_ ; }
        GEO::Mesh& mesh() const ;
        const vec3& point( index_t p = 0 ) const ;
        index_t nb_points() const ;

    protected:
        /// Pointer to the Well owning this element
        const Well* well_ ;
        /// Mesh of the element
        GEO::Mesh mesh_ ;
    } ;

// --------------------------------------------------------------------------

    class RINGMESH_API WellCorner: public WellMesh {
    public:
        /*!
         * Struct to store information about the corner
         */
        struct corner_info_t {
            corner_info_t()
                : is_on_surface( false ), id( NO_ID )
            {
            }
            corner_info_t( bool is_on_surface_in, const index_t& id_in )
                : is_on_surface( is_on_surface_in ), id( id_in )
            {
            }
            /// True is the corner is on a surface, false if is in a region
            bool is_on_surface ;
            /// The id of the corresponding surface or region
            index_t id ;
        } ;

        WellCorner(
            const Well* well,
            const vec3& point,
            const corner_info_t& corner_info ) ;
        virtual ~WellCorner() ;

        const corner_info_t& corner_info() const ;

    private:
        /// Information on the corner (cf. corner_info_t)
        GEO::Attribute< corner_info_t > corner_info_ ;
    } ;

// --------------------------------------------------------------------------

    class RINGMESH_API WellPart: public WellMesh {
    public:
        WellPart( const Well* well, index_t id ) ;

        /*!
         * Sets the corber id
         * @param[in] c the corner id (0 or 1)
         * @param[in] id the corner id in the corners_ vector the the well
         */
        void set_corner( index_t c, index_t id ) { corners_[c] = id ;}
        /*!
         * Gets the id of a corner
         * @param[in] c the corner id (0 or 1)
         * @return the corresponding id
         */
        index_t corner( index_t c ) const
        {
            ringmesh_debug_assert( c < 2 ) ;
            return corners_[ c ] ;
        }

        void set_points( const std::vector< vec3 >& points ) ;

        index_t nb_edges() const ;
        double length() const ;

        /*!
         * Sets the id of the part corresponding to the position in the parts_ vector of the well
         * @param[in] id the id to set
         */
        void set_id( index_t id ) { id_ = id ;}
        /*!
         * Gets the id of the part
         */
        index_t id() const { return id_ ;}

    private:
        /// id of the part corresponding to the position in the parts_ vector of the well
        index_t id_ ;
        /// id in the corners_ vector the the well
        index_t corners_[2] ;
    } ;

// --------------------------------------------------------------------------

    class RINGMESH_API Well {
        ringmesh_disable_copy( Well ) ;
    public:
        Well() ;
        ~Well() ;

        void copy_corners_and_informations( Well& well ) const ;

        void get_part_edges(
            index_t p,
            std::vector< Edge >& edges ) const ;
        void get_region_edges(
            index_t p,
            std::vector< Edge >& edges ) const ;

        /*!
         * Creates a new corner
         * @param[in] p the geometric position of the corner
         * @param[in] corner_info the corner_info_t corresponding to the corner to create
         * @return the id of the created corner
         */
        index_t create_corner(
            const vec3& p,
            const WellCorner::corner_info_t& corner_info)
        {
            corners_.push_back( new WellCorner( this, p, corner_info ) ) ;
            return corners_.size() - 1 ;
        }
        index_t find_corner( const vec3& p ) const ;
        /*!
         * Gets a corner
         * @param[in] c the id of the corner
         */
        const WellCorner& corner( index_t c ) const
        {
            ringmesh_debug_assert( c < corners_.size() ) ;
            return *corners_[ c ] ;
        }

        /*!
         * Creates a new part
         * @param[in] region the region id corresponding to the new part
         * @return the id of the created part
         */
        index_t create_part( index_t region )
        {
            parts_.push_back( new WellPart( this, parts_.size() ) ) ;
            part_region_id_.push_back( region ) ;
            return parts_.size() - 1 ;
        }
        /*!
         * Gets a part
         * @param[in] part the part id
         */
        const WellPart& part( index_t part ) const
        {
            ringmesh_debug_assert( part < parts_.size() ) ;
            return *parts_[ part ] ;
        }
        /*!
         * Gets a part
         * @param[in] part the part id
         */
        WellPart& part( index_t part )
        {
            ringmesh_debug_assert( part < parts_.size() ) ;
            return *parts_[ part ] ;
        }
        /*!
         * Gets the region id of a part
         * @param[in] part the part id
         * @return the region id of the part
         */
        index_t part_region_id( index_t part ) const
        {
            ringmesh_debug_assert( part < nb_parts() ) ;
            return part_region_id_[ part ] ;
        }

        /*!
         * Gets the number of corners
         */
        index_t nb_corners() const { return corners_.size() ;}
        /*!
         * Gets the number of parts
         */
        index_t nb_parts() const { return parts_.size() ;}
        index_t nb_edges() const ;
        /*!
         * Sets the well name
         * @param[in] name the name to set
         */
        void set_name( const std::string& name ) { name_ = name ;}
        /*!
         * Gets the well name
         */
        const std::string& name() const { return name_ ;}

    private:
        /// Vector of the corners of the well
        std::vector< WellCorner* > corners_ ;
        /// Vector of the parts of the well
        std::vector< WellPart* > parts_ ;
        /// Vector of the region id of the parts
        std::vector< index_t > part_region_id_ ;
        /// Name of the well
        std::string name_ ;
        /// Number of edges in the well
        index_t nb_edges_ ;
    } ;

// --------------------------------------------------------------------------

    class RINGMESH_API WellGroup {
        ringmesh_disable_copy( WellGroup ) ;
    public:
        WellGroup() ;
        virtual ~WellGroup() ;

        void get_region_edges(
            index_t region,
            std::vector< Edge >& edges ) const ;

        void get_region_edges(
            index_t region,
            std::vector< std::vector< Edge > >& edges ) const ;

        /*!
         * Gets the associated BoundaryModel
         */
        const BoundaryModel* model() const { return model_ ;}
        /*!
         * Sets the associated BoundaryModel
         */
        void set_model( RINGMesh::BoundaryModel* model ) { model_ = model ;}
        bool is_well_already_added( const std::string& name ) const ;

        void create_wells( index_t nb_wells ) ;
        void add_well( const GEO::Mesh& mesh, const std::string& name ) ;

        /*!
         * Gets the number of wells
         */
        index_t nb_wells() const { return wells_.size() ;}
        /*!
         * Gets the well
         * @param[in] w the well id
         * @return the corresponding well
         */
        const Well& well( index_t w ) const { return *wells_[ w ] ;}

    protected:
        /// Vector of the wells
        std::vector< Well* > wells_ ;
        /// Associated BoundaryModel
        BoundaryModel* model_ ;
    } ;
}
#endif
