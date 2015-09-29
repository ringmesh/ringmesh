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

/*! \author Jeanne Pellerin and Arnaud Botella */

#ifndef __RINGMESH_BOUNDARY_MODEL__
#define __RINGMESH_BOUNDARY_MODEL__

#include <ringmesh/common.h>
#include <ringmesh/geo_model_element.h>

#include <geogram/points/kd_tree.h>

#include <vector>

namespace RINGMesh {

    /*!
     * @brief Unique storage of the vertices of a GeoModel
     * @details Each instance is unique, unlike vertices in 
     *          the model Corner, Line, and Surface meshes.
     *          Attributes may be defined on the vertices.
     *
     * @todo Take also the Regions vertices
     */
    class RINGMESH_API GeoModelVertices {
    ringmesh_disable_copy( GeoModelVertices ) ;
    public:

        /*!
         * @brief Identification of a vertex in a GeoModelElement
         * @todo Je changerai bien ce nom moche au refactoring [JP]
         */
        struct VertexInBME {
            VertexInBME( GME::gme_t t, index_t vertex_id_in )
                : gme_id( t ), v_id( vertex_id_in )
            {
            }
            VertexInBME()
                : gme_id(), v_id( NO_ID )
            {
            }
            bool operator<( const VertexInBME& rhs ) const
            {
                if( gme_id != rhs.gme_id ) {
                    return gme_id < rhs.gme_id ;
                } else {
                    return v_id < rhs.v_id ;
                }
            }
            bool operator==( const VertexInBME& rhs ) const
            {
                return gme_id == rhs.gme_id && v_id == rhs.v_id ;
            }
            bool is_defined() const
            {
                return gme_id.is_defined() && v_id != NO_ID ;
            }
            /// Unique identifier of the associated GeoModelElement
            GME::gme_t gme_id ;
            /// Index of the vertex in the BME
            index_t v_id ;
        } ;

        /*!
         * @brief Constructor from an existing GeoModel
         */
        GeoModelVertices( const GeoModel& bm )
            : bm_( bm ), kdtree_( nil ), lock_( 0 ), kdtree_to_update_( true )
        {
        }

        ~GeoModelVertices()
        {
        }

        /// Maybe this is not the best test, maybe we should store a bool
        /// to store this
        bool is_initialized() const
        {
            return mesh_.vertices.nb() > 0 ;
        }

        /*!
         * @brief Number of vertices stored.
         * @details Calls initialize() if vertices are not filled yet
         */
        index_t nb() const ;

        /*!
         * @brief Coordinates of a vertex of the GeoModel
         * @pre v < nb()
         * @todo Review : Change the name of this function [JP]
         */
        const vec3& unique_vertex( index_t v ) const ;

        /*!
         * @brief Returns the index of the given vertex in the model
         * @param[in] p input point coordinates
         * @return index of the vertex in the model if found 
         * (distance < epsilon), otherwise NO_ID
         */
        index_t vertex_index( const vec3& p ) const ;

        /*!
         * @brief Get the vertices in BME corresponding to the given unique vertex
         */
        const std::vector< VertexInBME >& bme_vertices( index_t v ) const ;

        /*!
         * @brief To use when building the model by first adding its vertices
         * @warning The client is responsible for setting the mapping between the points
         * of the BME and the unique vertex 
         */
        index_t add_unique_vertex( const vec3& point ) ;

        /*!
         * @brief Add a vertex in a GeoModelElement 
         *        corresponding to an existing vertex of the model
         */
        void add_unique_to_bme( index_t v, const VertexInBME& v_bme ) ;

        /*! 
         * @brief Change one of the BME vertex associated to a vertex
         * @param v Index of the vertex
         * @param i Index of the BME vertex
         * @param v_bme Id of BME and of the vertex in that BME
         */
        void set_bme( index_t v, index_t i, const VertexInBME& v_bme ) ;

        /*!
         * @brief Set the point coordinates of all the vertices that 
         *        share this unique vertex, including the unique vertex itself.
         * @param[in] v Index of the vertex
         * @param[in] point New coordinates
         */
        void update_point( index_t v, const vec3& point ) ;

        /*!
         * @brief Clear the vertices - clear the bme_vertices_ - 
         *        clear global vertex information in the all BMME
         * @warning Not stable - crashes if atributes are still bound
         */
        void clear() ;

        /*!
         * @brief Returns the Geogram attribute manager on the global vertices
         */
        GEO::AttributesManager& attribute_manager() const
        {
            return mesh_.vertices.attributes() ;
        }

        /*!
         * @brief Remove colocated vertices
         */
        void remove_colocated() ;

        /*! 
         * @brief Remove all invalid VertexInBME and delete the vertices 
         * that are not anymore in any GeoModelElement
         */
        void erase_invalid_vertices() ;

        /*! 
         * @brief Delete vertices for which to_delete[i] != i 
         * @detail The global vertices are deleted, bme_vertices_
         * is updated and the model_vertx_id in the GeoModelMeshElement
         * of the BoudnaryModel are updated too.
         *
         * @param[in,out] to_delete can be NO_ID or give the index of a 
         *  kept vertex with wich information should be merged.
         *  It is recyled to give the mapping between old and new vertex indices        
         * @pre to_delete[ v ] is either NO_ID, or is equal or inferior to v
         */
        void erase_vertices( std::vector< index_t >& to_delete ) ;

    private:
        /*!
         * @brief Initialize the vertices from the vertices 
         *        of the GeoModel Corners, Lines, and Surfaces
         * @details Fills the mesh_.vertices, bme_vertices_ and 
         *         delete colocated vertices
         */
        void initialize() ;

        /*!
         * @brief Set a vertex as invalid
         * @details Sets all the ids of the corresponding VertexInBME to NO_ID
         */
        void set_invalid_vertex( index_t v ) ;

        /*!
         * @brief True if the vertex is not valid
         * @details A vertex is invalid if all corresponding VertexInBME 
         * have a NO_ID value.
         */
        bool is_invalid_vertex( index_t v ) const ;

        /*!
         * @brief Delete the KdTree and set the pointer to nil.         
         */
        void set_to_update() ;

        /*!
         * @brief Build the KdTree of the vertices 
         * @note The function is const to be called when accessing a point index
         *  from coordinated without an ugly const-cast.
         */
        void initialize_kdtree() const ;

    private:
        /// Attached GeoModel owning the vertices
        const GeoModel& bm_ ;

        /*! 
         * @brief Mesh storing the coordinates of the vertices that are not colocated
         * @details Each vertex is unique. 
         * On these vertices attributes can be defined
         */
        GEO::Mesh mesh_ ;

        /*! 
         * Vertices in GeoModelElements corresponding to each vertex
         * @todo Change this extremely expensive storage !!!
         */
        std::vector< std::vector< VertexInBME > > bme_vertices_ ;

        /// Kd-tree of the model vertices
        mutable GEO::NearestNeighborSearch_var kdtree_ ;
        mutable bool kdtree_to_update_ ;

        /// Lock to protect from multi-threading during clear()
        GEO::Process::spinlock lock_ ;
    } ;

    /*!
     * @brief The class to describe a geological model represented 
     * by its boundary surfaces and whose regions can be optionally meshed
     */
    class RINGMESH_API GeoModel {
    ringmesh_disable_copy( GeoModel ) ;
        friend class GeoModelBuilder ;

    public:
        const static index_t NO_ID = index_t( -1 ) ;

        /*!
         * @brief Constructs an empty GeoModel
         */
        GeoModel() ;

        /*!
         * @brief Delete all GeoModelElements stored and owned by the GeoModel
         */
        virtual ~GeoModel() ;

        void copy( const GeoModel& from ) ;

        /*!
         * @brief Name of the model
         */
        const std::string& name() const
        {
            return name_ ;
        }

        /*!
         * @brief Get the directory for debug information
         * @todo To move [JP]
         */
        const std::string& debug_directory() const
        {
            return debug_directory_ ;
        }

        /*!
         * @brief Set the directory where debugging information shall be stored
         * @details Test that this directory exists, if not
         *          keep the previous value.
         *          The default directory is the executable directory.
         * @todo To move [JP]
         */
        void set_debug_directory( const std::string& directory ) ;

        /*!
         * \name Generic GeoModelElement accessors
         * @{
         */

        /*!
         * @brief Returns the number of elements of the given type
         * @details Default value is 0.
         * @param[in] type the element type
         */
        index_t nb_elements( GME::TYPE type ) const
        {
            if( type < GME::NO_TYPE ) {
                return elements( type ).size() ;
            } else if( type == GME::ALL_TYPES ) {
                ringmesh_assert( !nb_elements_per_type_.empty() ) ;
                return nb_elements_per_type_.back() ;
            } else {
                return 0 ;
            }
        }

        /*!
         * @brief Returns a const reference the identified GeoModelElement
         * @details The default value is the universe Region
         * @param[in] type Type of the element
         * @param[in] index Index of the element
         *
         */
        const GeoModelElement& element( GME::gme_t id ) const
        {
            ringmesh_assert( id.index < nb_elements( id.type ) ) ;
            if( id.type < GME::NO_TYPE ) {
                return *elements( id.type )[id.index] ;
            } else if( id.type == GME::ALL_TYPES ) {
                return element( global_to_typed_id( id ) ) ;
            } else {
                return universe_ ;
            }
        }

        const GeoModelMeshElement& mesh_element( GME::gme_t id ) const
        {
            ringmesh_assert( GME::has_mesh( id.type ) ) ;
            return dynamic_cast< const GeoModelMeshElement& >( element( id ) ) ;
        }

        /*! @}
         * \name Specicalized accessors.
         * @{
         */
        index_t nb_corners() const
        {
            return nb_elements( GME::CORNER ) ;
        }
        index_t nb_lines() const
        {
            return nb_elements( GME::LINE ) ;
        }
        index_t nb_surfaces() const
        {
            return nb_elements( GME::SURFACE ) ;
        }
        index_t nb_regions() const
        {
            return nb_elements( GME::REGION ) ;
        }
        index_t nb_contacts() const
        {
            return nb_elements( GME::CONTACT ) ;
        }
        index_t nb_interfaces() const
        {
            return nb_elements( GME::INTERFACE ) ;
        }
        index_t nb_layers() const
        {
            return nb_elements( GME::LAYER ) ;
        }

        const Corner& corner( index_t index ) const
        {
            // Yes, we could use static_cast, but I do not trust the
            // Builder and I prefer to check [JP]  
            return dynamic_cast< const Corner& >( *corners_.at( index ) ) ;
        }

        const Line& line( index_t index ) const
        {
            return dynamic_cast< const Line& >( *lines_.at( index ) ) ;
        }

        const Surface& surface( index_t index ) const
        {
            return dynamic_cast< const Surface& >( *surfaces_.at( index ) ) ;
        }

        const Region& region( index_t index ) const
        {
            return dynamic_cast<const Region&>( *regions_.at( index ) ) ;
        }

        const GeoModelElement& contact( index_t index ) const
        {
            return element( GME::gme_t( GME::CONTACT, index ) ) ;
        }

        const GeoModelElement& one_interface( index_t index ) const
        {
            return element( GME::gme_t( GME::INTERFACE, index ) ) ;
        }

        const GeoModelElement& layer( index_t index ) const
        {
            return element( GME::gme_t( GME::LAYER, index ) ) ;
        }

        const Region& universe() const
        {
            return dynamic_cast<const Region&> ( universe_ ) ;
        }

        /*!
         * @}
         */

        /* @todo Move into an API
         * It is a very very bad idea to modify the coordinates of vertices the BME 
         * without the BM knowing !!! [JP]
         */
        void translate( const vec3& ) ;
        void rotate(
            const vec3& origin,
            const vec3& axis,
            float64 angle,
            bool degrees = false ) ;

        bool check_model_validity( bool check_surface_intersections = true ) const ;           

    private:
        bool check_elements_validity() const ;
        bool check_geology_validity() const ;        

        void copy_macro_topology( const GeoModel& from ) ;
        void copy_meshes( const GeoModel& from ) ;

        /*! 
         * @brief Convert a global BME index into a typed index
         * @details Relies on the nb_elements_per_type_ vector that 
         *          must be up to date at all times 
         *          See the GeoModelBuilder::end_model() function
         * @param[in] global A BME id of TYPE - ALL_TYPES
         * @return A BME id of an element of the model, or a invalid one if nothing found
         */
        inline GME::gme_t global_to_typed_id( const GME::gme_t& global ) const
        {
            index_t t = NO_ID ;
            for( index_t i = 1; i < nb_elements_per_type_.size(); i++ ) {
                if( global.index >= nb_elements_per_type_[i - 1]
                    && global.index < nb_elements_per_type_[i] ) {
                    t = i - 1 ;
                    break ;
                }
            }
            if( static_cast< GME::TYPE >( t ) < GME::NO_TYPE ) {
                GME::TYPE T = static_cast< GME::TYPE >( t ) ;
                index_t i = global.index - nb_elements_per_type_[t] ;
                return GME::gme_t( T, i ) ;
            } else {
                return GME::gme_t() ;
            }
        }

        /*!
         * @brief Generic accessor to the storage of elements of the given type
         * @pre The type must be valid NO_TYPE or ALL_TYPES will throw an assertion
         */
        std::vector< GME* >& modifiable_elements( GME::TYPE type )
        {
            return const_cast< std::vector< GME* >& >( elements( type ) ) ;
        }

        /*!
         * @brief Generic accessor to the storage of elements of the given type
         * @pre The type must be valid. NO_TYPE or ALL_TYPES will throw an assertion
         */
        const std::vector< GME* >& elements( GME::TYPE type ) const
        {
            switch( type ) {
                case GME::CORNER:
                    return corners_ ;
                case GME::LINE:
                    return lines_ ;
                case GME::SURFACE:
                    return surfaces_ ;
                case GME::REGION:
                    return regions_ ;
                case GME::CONTACT:
                    return contacts_ ;
                case GME::INTERFACE:
                    return interfaces_ ;
                case GME::LAYER:
                    return layers_ ;
                default:
                    ringmesh_assert_not_reached;
                return surfaces_ ;
            }
        }

    public:
        GeoModelVertices vertices ;

    private:
        // Name of the model
        std::string name_ ;

        // Base manifold elements of a model
        std::vector< GeoModelElement* > corners_ ;
        std::vector< GeoModelElement* > lines_ ;
        std::vector< GeoModelElement* > surfaces_ ;
        std::vector< GeoModelElement* > regions_ ;

        /// The region including all the other regions
        /// \todo Put it as the last item in regions_ and do not forget to create it in the Builder
        Region universe_ ;

        /*!
         * @brief Contacts between Intefaces
         * Parent of a set of Line
         */
        std::vector< GeoModelElement* > contacts_ ;
        /*!
         * @brief Interfaces between layers
         * Parent of a set of Surface
         */
        std::vector< GeoModelElement* > interfaces_ ;

        /*!
         * @brief Rock layers
         * Parent of a set of Region
         */
        std::vector< GeoModelElement* > layers_ ;

        /// Allow global access to BME. It MUST be updated if one element is added.
        std::vector< index_t > nb_elements_per_type_ ;

        /// Name of the debug directory in which to save stuff 
        /// @note Move this in another class
        std::string debug_directory_ ;
    } ;

}

#endif
