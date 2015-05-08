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

/*! \author Jeanne Pellerin and Arnaud Botella */

#ifndef __RINGMESH_BOUNDARY_MODEL_ELEMENT__
#define __RINGMESH_BOUNDARY_MODEL_ELEMENT__

#include <ringmesh/common.h>
#include <ringmesh/utils.h>

#include <geogram/mesh/mesh.h>

#include <vector>
#include <string>

namespace GEO {
    class MeshFacetsAABB ;
}

namespace RINGMesh {
    class BoundaryModel ;
    class Surface ;
}

namespace RINGMesh {

    /*!
     * @brief Generic class describing one element of a BoundaryModel
     */
    class RINGMESH_API BoundaryModelElement {
        ringmesh_disable_copy( BoundaryModelElement ) ;
    public:
        /*!
         * @brief Geological feature types for BoundaryModelElement
         * \todo Read all types, this is not sufficient
         */
        enum GEOL_FEATURE {
            ALL_GEOL,
            NO_GEOL,
            STRATI,
            FAULT,
            VOI,
            STRATI_FAULT,
            STRATI_VOI,
            FAULT_VOI
        } ;

        /*!
         * @brief Each BoundaryModelElement has a type
         * @details When no type is defined NO_TYPE should be used
         * There is two main categories of elements
         *   - low-level elements (CORNER, LINE, SURFACE, REGION) have a geometry and connectivity relationships
         *   - high-level elements (CONTACT, INTERFACE, LAYER) that are constituted of low-level elements
         *
         * DO NOT MODIFY THIS ENUM
         */
        enum TYPE {
            CORNER = 0,
            LINE,
            SURFACE,
            REGION,
            CONTACT,
            INTERFACE,
            LAYER,
            NO_TYPE,
            ALL_TYPES
        } ;

        /*! 
         * @brief Unique identification of a BoundaryModelElement in a BoundaryModel
         * @details Stores the TYPE of the element and its index in the BoundaryModel.
         *          Default values are NO_TYPE and NO_ID
         */
        struct bme_t {
            bme_t()
                : type( NO_TYPE ), index( NO_ID )
            {
            }
            bme_t( TYPE t, index_t id )
                : type( t ), index( id )
            {
            }
            bool operator!=( const bme_t& t ) const {
                return type != t.type || index != t.index ;
            }
            bool operator==( const bme_t& t ) const {
                return type == t.type && index == t.index ;
            }
            bool is_defined() const
            {
                return type != NO_TYPE &&
                       type != ALL_TYPES &&
                       index != NO_ID ;
            }
            /// TYPE of the BoundaryModelElement
            TYPE type ;
            ///  Index of the element in the BoundaryModel
            index_t index ;
        } ;

        const static index_t NO_ID = index_t(-1) ;

        static GEOL_FEATURE determine_geological_type( const std::string& in ) ;
        static GEOL_FEATURE determine_type( const std::vector< GEOL_FEATURE >& types ) ;

        static std::string geol_name( GEOL_FEATURE ) ;
        static std::string type_name( TYPE t ) ;

        // Key functions - They determine which element of which type
        // can fill the different class attributes
        static TYPE parent_type     ( TYPE t ) ;
        static TYPE child_type      ( TYPE t ) ;
        static TYPE boundary_type   ( TYPE t ) ;
        static TYPE in_boundary_type( TYPE t ) ;
        static index_t dimension    ( TYPE t ) ;

        static bool parent_allowed     ( TYPE t ) { return parent_type( t )      != NO_TYPE ; }
        static bool child_allowed      ( TYPE t ) { return child_type( t )       != NO_TYPE ; }
        static bool boundary_allowed   ( TYPE t ) { return boundary_type( t )    != NO_TYPE ; }
        static bool in_boundary_allowed( TYPE t ) { return in_boundary_type( t ) != NO_TYPE ; }
        

        /*!
         * @brief Constructs a BoundaryModelElement
         *
         * @param[in] model Pointer to the parent model.
         * @param[in] element_type Type of the element to create
         * @param[in] id Index of the element in the corresponding vector in the model
         */
        BoundaryModelElement(
            BoundaryModel* model = NULL,
            TYPE element_type = NO_TYPE,
            index_t id = NO_ID )
              : model_( model ), id_( element_type, id ),
                name_( "" ), geol_feature_( NO_GEOL )
        {
        }

        virtual ~BoundaryModelElement() {}


        /*! 
         * @brief Test the strict equality of the attributes.
         * @todo Sort the vectors before comparison ?
         */
        bool operator==( const BoundaryModelElement& rhs ) const ;

        /*!@}
        * \name Validity checks
        * @{
        */

        /*!
        * @brief Global validity check - Reimplemented in BMME
        */
        virtual bool is_valid() const
        {
            return is_connectivity_valid() ;
        }
        
        /*!
         * @brief Basic checks on the minimum required information 
         * @details Required connectivity information depend on the TYPE.   
         *          Check that connectivity information stored by elements is consistent.
         *          e.g. the parent of a BME must have it in its chidren list 
         * 
         * @todo Write meaningful message when the test fails ?
         */
        bool is_connectivity_valid() const ;

        
        /*!
         * \name Accessors to basic information
         * @{
         */
        bool has_model() const { return model_ != NULL ; }
        const BoundaryModel& model() const { return *model_ ; }
        bool has_name() const { return name_ != "" ; }
        const std::string& name() const { return name_ ; }
        bme_t bme_id() const { return id_ ; }
        bool has_geological_feature() const { return geol_feature_ != NO_GEOL ; }
        GEOL_FEATURE geological_feature() const { return geol_feature_ ; }
        bool is_on_voi() const ;

        /*!@}
         * \name Connectivity - boundary and in_boundary
         * @{
         */
        index_t nb_boundaries() const { return boundaries_.size() ;}
        const bme_t& boundary_id( index_t x ) const { return boundaries_[ x ] ;}
        const BoundaryModelElement& boundary( index_t x ) const ;

        bool side( index_t i ) const { return sides_[ i ] ;}

        index_t nb_in_boundary() const { return in_boundary_.size() ;}
        const bme_t& in_boundary_id( index_t x ) const { return in_boundary_[ x ] ;}
        const BoundaryModelElement& in_boundary( index_t x ) const ;

        /*!@}
         * \name Parent - children relationships
         * @{
         */
        bool has_parent() const { return parent_.index != NO_ID ;}
        const bme_t& parent_id() const { return parent_ ; }
        const BoundaryModelElement& parent() const ;

        index_t nb_children() const { return children_.size() ;}
        const bme_t& child_id( index_t x ) const { return children_[ x ] ;}
        const BoundaryModelElement& child( index_t x ) const ;

        /*!@}
         * \name Accessors to geometry - Reimplemented in BoundaryModelMeshElement
         * @{
         */
        virtual index_t nb_cells() const
        {
            return 0 ;
        }

        virtual index_t nb_vertices() const
        {
            return 0 ;
        }

        virtual index_t model_vertex_id( index_t p = 0 ) const
        {
            ringmesh_assert_not_reached ;
            return NO_ID ;
        }

        virtual const vec3& vertex( index_t p = 0 ) const
        {
            ringmesh_assert_not_reached ;
            return dummy_vec3 ;
        }

        virtual void set_vertex(
            index_t index,
            const vec3& point,
            bool update = true )
        {
            ringmesh_assert_not_reached ;
        }


        /*!@}
         * \name Modification of the element
         * @{
         */
        void copy_macro_topology(
            const BoundaryModelElement& rhs,
            BoundaryModel& model ) ;

        void set_model( BoundaryModel* m ) { model_ = m  ; }
        void set_element_type( TYPE t ) { id_.type = t ; }
        void set_id( index_t id ) { id_.index = id ; }
        void set_name( const std::string& name ) { name_ = name ; }
        void set_geological_feature( GEOL_FEATURE type ) { geol_feature_ = type ; }

        void add_boundary( const bme_t& b )
        {
            ringmesh_debug_assert( b.is_defined() ) ;
            ringmesh_debug_assert( boundary_type( id_.type ) == b.type ) ;
            boundaries_.push_back( b ) ;
        }

        void set_boundary( index_t id, const bme_t& b )
        {
            ringmesh_debug_assert( b.is_defined() ) ;
            ringmesh_debug_assert( boundary_type( id_.type ) == b.type ) ;
            ringmesh_debug_assert( id < nb_boundaries() ) ;
            boundaries_[ id ] = b ;
        }

        void add_boundary( const bme_t& b, bool side )
        {
            ringmesh_debug_assert( b.is_defined() ) ;
            ringmesh_debug_assert( boundary_type( id_.type ) == b.type ) ;
            boundaries_.push_back( b ) ;
            sides_.push_back( side ) ;
        }

        void set_boundary( index_t id, const bme_t& b, bool side )
        {
            ringmesh_debug_assert( b.is_defined() ) ;
            ringmesh_debug_assert( boundary_type( id_.type ) == b.type ) ;
            ringmesh_debug_assert( id < nb_boundaries() ) ;
            boundaries_[ id ] = b ;
            sides_[ id ] = side ;
        }

        void add_in_boundary( const bme_t& in_b )
        {
            ringmesh_debug_assert( in_b.is_defined() ) ;
            ringmesh_debug_assert( in_boundary_type( id_.type ) == in_b.type ) ;
            in_boundary_.push_back( in_b ) ;
        }

        void set_in_boundary( index_t id, const bme_t& in_b )
        {
            ringmesh_debug_assert( in_b.is_defined() ) ;
            ringmesh_debug_assert( in_boundary_type( id_.type ) == in_b.type ) ;
            ringmesh_debug_assert( id < nb_in_boundary() ) ;
            in_boundary_[ id ] = in_b ;
        }

        void set_parent( const bme_t& p )
        {
            ringmesh_debug_assert( p.is_defined() ) ;
            ringmesh_debug_assert( parent_type( id_.type ) == p.type ) ;
            parent_ = p ;
        }

        void add_child( const bme_t& c )
        {
            ringmesh_debug_assert( c.is_defined() ) ;
            ringmesh_debug_assert( child_type( id_.type ) == c.type ) ;
            children_.push_back( c ) ;
        }

        void set_child( index_t id, const bme_t& c )
        {
            ringmesh_debug_assert( c.is_defined() ) ;
            ringmesh_debug_assert( child_type( id_.type ) == c.type ) ;
            ringmesh_debug_assert( id < nb_children() ) ;
            children_[ id ] = c ;
        }

        /*!
         * @}
         */

    protected:
        /// Pointer to the BounadyModel owning this element
        BoundaryModel* model_ ;

        /// TYPE and id of the BoundaryModelElement
        bme_t id_ ;

        /// Name of the element - by default it is an empty string
        std::string name_ ;

        /// Geological feature of this object - default is NO_GEOL
        GEOL_FEATURE geol_feature_ ;

        /// Elements on the boundary of this element - see boundary_type( TYPE )
        std::vector< bme_t > boundaries_ ;

        /// Additional information for oriented boundaries (only for REGION)
        /// Side: + (true) or - (false)
        std::vector< bool > sides_ ;

        /// Elements in which boundary this element is - see in_boundary_type( TYPE )
        std::vector< bme_t > in_boundary_ ;

        /// Id of the parent - see parent_type( TYPE ) - default value is NO_ID.
        bme_t parent_ ;

        /// Elements constituting this one - see child_type( TYPE )
        std::vector< bme_t > children_ ;
    } ;


    // Ce n'est pas tres malin de faire ce genre de chose dans un .h dit Mr Stroupstrup
    // N'importe qui peut inclure un .h (Jeanne). Ici encore on pourrait le garder.    
    typedef BoundaryModelElement BME ;

    // I am against the use of a dummy_BME defined in .h
    // PLEASE do not put it back Jeanne
    // There are nicer ways to deal with these
    // const static BoundaryModelElement dummy_BME ;
    // const static BoundaryModelElement::bme_t dummy_bme_type ;

    /*!
    * @brief Name of the attribute storing the index of a vertex in the model
    * 
    * @todo Put it in the  BoundaryModelMeshElement - but if I do it I have 
    *       linking errors in the code that depend on it JP
    */
    const static std::string model_vertex_id_att_name = std::string( "model_vertex_id" ) ;

    /*!
     * @brief Abstract base class for BoundaryModelElement which have a geometrical representation
     *
     */
    class RINGMESH_API BoundaryModelMeshElement : public BoundaryModelElement {
        ringmesh_disable_copy( BoundaryModelMeshElement ) ;
    public :
        BoundaryModelMeshElement(
            BoundaryModel* model = NULL,
            TYPE element_type = NO_TYPE,
            index_t id = NO_ID ) 
            : BoundaryModelElement( model, element_type, id ) 
        {
            model_vertex_id_.bind( mesh_.vertices.attributes(), model_vertex_id_att_name ) ;
        }
        virtual ~BoundaryModelMeshElement() ;       

        /*! 
         * @brief Check if the mesh stored is valid.
         */
        virtual bool is_mesh_valid() const = 0 ;

        /*!
         * @brief Global validity of the element
         */
        virtual bool is_valid() const {
            return is_connectivity_valid() && is_mesh_valid();
        }

        /*! 
         * @brief Check the validity of model vertex ids.
         */
        bool are_model_vertices_valid() const ;

        
        /*!
         * @brief Returns the number of edges or facets of the mesh
         */
        virtual index_t nb_cells() const {
            switch ( bme_id().type ) {
            case LINE :
                return mesh_.edges.nb() ;
            case SURFACE :
                return mesh_.facets.nb() ;
            default :
                return 0 ;
            }
        }

        /*!
         * @brief Returns the number of vertices of the mesh
         */
        virtual index_t nb_vertices() const { return mesh_.vertices.nb() ; }

        virtual index_t model_vertex_id( index_t v = 0 ) const ;

        void set_model_vertex_id( index_t v, index_t model_id ) ;

        virtual const vec3& vertex( index_t v = 0 ) const ;
       

        virtual void set_vertex(
            index_t index,
            const vec3& point,
            bool update ) ;
        

        void set_vertex( index_t v, index_t model_vertex ) ;
        

        virtual void set_vertices(
            const std::vector< vec3 >& points, 
            bool clear_mesh = false ) ;

        virtual void set_vertices(
            const std::vector< index_t >& model_vertices, 
            bool clear_mesh = false ) ;

        virtual index_t local_id( index_t model_vertex_id ) const ;

        /*!
         * @}
         * \name Attribute managers
         * @{
         */ 
        virtual GEO::AttributesManager& vertex_attribute_manager() const {
            return mesh_.vertices.attributes() ;
        }

        virtual GEO::AttributesManager& cell_attribute_manager() const {
            return mesh_.facets.attributes() ;
        }
        void bind_attributes();
        
        void unbind_attributes();

        /*! @}
         */ 

        GEO::Mesh& mesh() const {
            return const_cast< GEO::Mesh& >( mesh_ ) ;
        }

    protected :
        GEO::Mesh mesh_ ;
        GEO::Attribute<index_t> model_vertex_id_ ;
    } ;



    /*!
     * @brief A Corner 
     *
     * @details Element of type CORNER. Its geometry is determined by one vertex.
     * Most corners are at the intersections of at least two Line, but some
     * are in the boundary of a closed Line.
     */
    class RINGMESH_API Corner : public BoundaryModelMeshElement {
    public:
        Corner(
            BoundaryModel* model = nil,
            index_t id = NO_ID,
            const vec3& vertex = vec3() )
              : BoundaryModelMeshElement( model, CORNER, id )
        {
            mesh_.vertices.create_vertex( vertex.data() ) ;
        }

        virtual ~Corner() {}

        virtual bool is_mesh_valid() const 
        { 
            return mesh_.vertices.nb() == 1 &&                    
                   mesh_.edges.nb()    == 0 && 
                   mesh_.facets.nb()   == 0 &&  
                   mesh_.cells.nb()    == 0 &&
                   mesh_.vertices.point(0) != vec3();
        }
        
        void set_vertex( const vec3& point, bool update_model )
        {
            BoundaryModelMeshElement::set_vertex( 0, point, update_model ) ;
        }

        void set_vertex( index_t model_point_id ) 
        {
            BoundaryModelMeshElement::set_vertex( 0, model_point_id ) ;    
        }

        void set_model_vertex_id( index_t model_id )
        {
            BoundaryModelMeshElement::set_model_vertex_id( 0, model_id ) ;
        }
    } ;


    /*!
     * @brief A boundary Line of a Surface
     *
     * @details To be valid a Line must have 2 Corners that may be the same
     * and be in the boundary of a least one Surface.
     */
    class RINGMESH_API Line : public BoundaryModelMeshElement {
    public:
        Line(
            BoundaryModel* model = nil,
            index_t id = NO_ID ) ;

        virtual ~Line() {}
        virtual bool is_mesh_valid() const ;

        virtual void set_vertices(
            const std::vector< vec3 >& points,
            bool clear_mesh = false ) ;

        virtual void set_vertices(
            const std::vector< index_t >& model_vertices,
            bool clear_mesh = false ) ;

        /*!
         * @brief A Line is closed if its two extremities are identitical 
         */
        bool is_closed() const
        {
            ringmesh_debug_assert( nb_boundaries() == 2 ) ;
            return ( boundaries_[ 0 ].is_defined() ) &&
                   ( boundaries_[ 0 ] == boundaries_[ 1 ] ) ;
        }

        bool is_inside_border( const BoundaryModelElement& e ) const ;

        bool equal( const std::vector< vec3 >& rhs_vertices ) const ;

        vec3 segment_barycenter( index_t s ) const ;
        double segment_length( index_t s ) const ;
        double total_length() const ;
    } ;



    class RINGMESH_API SurfaceTools {
    public:
        SurfaceTools( const Surface& surface ) ;
        ~SurfaceTools() ;

        const GEO::MeshFacetsAABB& aabb() const ;
        const ColocaterANN& ann() const ;

    private:
        const Surface& surface_ ;

        GEO::MeshFacetsAABB* aabb_ ;
        ColocaterANN* ann_ ;
    } ;

    /*!
     * @brief A polygonal manifold surface
     *
     * @details This is a BoundaryModelElement of type SURFACE.
     * It is defined by a set of vertices and a set of polygonal facets.
     * Its boundaries are several Lines and it is on the boundary of 1 or 2 Region
     */
    class RINGMESH_API Surface : public BoundaryModelMeshElement {
        friend class SurfaceTools ;

    public:
        const static index_t NO_ADJACENT = index_t(-1) ;

        Surface(
            BoundaryModel* model = nil,
            index_t id = NO_ID )
            : BoundaryModelMeshElement( model, SURFACE, id ), tools( *this )
        {
        }

        virtual ~Surface() ;

        virtual bool is_mesh_valid() const ;

        bool is_triangulated() const { return mesh_.facets.are_simplices() ; }

        /*!
        * @brief Returns the coordinates of point \param v in facet \param f
        */
        const vec3& vertex(
            index_t f,
            index_t v ) const ;

        // If I do not put these ones the stupid compiler does not find it
        // There is propbably a nicer solution (Jeanne)
        virtual const vec3& vertex( index_t v ) const {
            return BoundaryModelMeshElement::vertex(v) ;
        }
        index_t model_vertex_id( index_t v ) const {
            return BoundaryModelMeshElement::model_vertex_id(v) ;
        }

        /*!
         * \name Accessors to facet and vertices
         * @{
         */
        index_t facet_begin( index_t f ) const { return mesh_.facets.corners_begin( f ) ; }
        index_t facet_end( index_t f ) const { return mesh_.facets.corners_end( f ) ; }
        index_t nb_vertices_in_facet( index_t f ) const
        {
            return mesh_.facets.nb_vertices( f ) ;
        }

        bool is_triangle( index_t f ) const { return nb_vertices_in_facet( f ) == 3 ; }

        index_t next_in_facet(
            index_t f,
            index_t v ) const
        {
            ringmesh_debug_assert( v < nb_vertices_in_facet( f ) ) ;
            if( v != nb_vertices_in_facet( f ) - 1 ) {
                return v + 1 ;
            } else {
                return 0 ;
            }
        }

        index_t prev_in_facet(
            index_t f,
            index_t v ) const
        {
            ringmesh_debug_assert( v < nb_vertices_in_facet( f ) ) ;
            if( v > 0 ) {return v - 1 ;} else {return nb_vertices_in_facet( f ) - 1 ;}
        }

        index_t nb_corners() const { return mesh_.facet_corners.nb() ; }
        index_t model_vertex_id_at_corner( index_t corner ) const
        {
            return BoundaryModelMeshElement::model_vertex_id( mesh_.facet_corners.vertex( corner ) ) ;
        }

        /*!
         * @brief Returns the surface index of vertex \param v in facet \param f
         */
        index_t surf_vertex_id(
            index_t f,
            index_t v ) const
        {
            ringmesh_debug_assert( v < nb_vertices_in_facet( f ) ) ;
            return mesh_.facets.vertex( f, v ) ;
        }

        /*!
         * @brief Returns the index of vertex \param v in facet \param f in the parent BoundaryModel
         */
        index_t model_vertex_id(
            index_t f,
            index_t v ) const
        {
            ringmesh_debug_assert( v < nb_vertices_in_facet( f ) ) ;
            return BoundaryModelMeshElement::model_vertex_id( surf_vertex_id( f, v ) ) ;
        }

       
        /*!
         * @brief Returns a vertex surface index from its model index
         * @details Returns the first one only or NO_ID if no point is found
         * 
         * @todo To remove
         */
        index_t surf_vertex_id( index_t model_vertex_id ) const ;

        index_t facet_vertex_id(
            index_t t,
            index_t surf_vertex_id ) const ;

        index_t facet_id_from_model(
            index_t f,
            index_t model_vertex_id ) const ;

        index_t facet_from_surface_vertex_ids(
            index_t in0,
            index_t in1 ) const ;

        index_t facet_from_model_vertex_ids(
            index_t i0,
            index_t i1 ) const ;

        void edge_from_model_vertex_ids(
            index_t i0,
            index_t i1,
            index_t& f,
            index_t& e ) const ;

        void oriented_edge_from_model_vertex_ids(
            index_t i0,
            index_t i1,
            index_t& facet,
            index_t& edge ) const ;

        index_t facets_around_vertex(
            index_t surf_vertex_id,
            std::vector< index_t >& result,
            bool border_only ) const ;

        index_t facets_around_vertex(
            index_t surf_vertex_id,
            std::vector< index_t >& result,
            bool border_only,
            index_t first_facet ) const ;

        /*! @}
         * \name Geometrical request on facets
         * @{
         */
        vec3 facet_barycenter( index_t f ) const ;
        double facet_area( index_t f ) const ;
        vec3 facet_normal( index_t f ) const ;
        void vertex_normals( std::vector< vec3 >& normals ) const ;
        index_t closest_vertex_in_facet(
            index_t f,
            const vec3& vertex ) const ;
        vec3 edge_barycenter( index_t c ) const ;

        /*! @}
         * \name Adjacencies request
         * @{
         */
        /*! @brief Returns the index of the adjacent facet of \param f in this surface
         *  along the edge starting at \param v */
        index_t adjacent( index_t f, index_t v ) const
        {
            ringmesh_debug_assert( v < nb_vertices_in_facet( f ) ) ;
            return mesh_.facets.adjacent( f, v ) ;
        }

        /*! @brief Returns the index of the adjacent facet at the given corner
         */
        index_t adjacent( index_t c ) const
        {
            ringmesh_assert( c < mesh_.facet_corners.nb() ) ;
            return mesh_.facet_corners.adjacent_facet( c ) ;
        }

        bool is_on_border( index_t f, index_t v ) const
        {
            ringmesh_debug_assert( v < nb_vertices_in_facet( f ) ) ;
            return adjacent( f, v ) == GEO::NO_CELL ;
        }

        bool is_on_border( index_t f ) const
        {
            for( index_t adj = 0; adj < nb_vertices_in_facet( f ); adj++ ) {
                if( is_on_border( f, adj ) ) { return true ; }
            }
            return false ;
        }

        void next_on_border(
            index_t f,
            index_t from,
            index_t v,
            index_t& next_f,
            index_t& v_in_next,
            index_t& to = dummy_index_t ) const ;

        void next_on_border(
            index_t f,
            index_t e,
            index_t& next_f,
            index_t& next_e ) const ;

        /*! @}
         * \name Modifiers
         * @{
         */
        void set_adjacent(
            index_t f,
            index_t e,
            index_t adjacent )
        {
            mesh_.facets.set_adjacent( f, e, adjacent ) ;
        }

        void set_geometry(
            const std::vector< vec3 >& vertices,
            const std::vector< index_t >& facets,
            const std::vector< index_t >& facet_ptr ) ;

        void set_geometry(
            const std::vector< index_t >& model_vertex_ids,
            const std::vector< index_t >& facets,
            const std::vector< index_t >& facet_ptr ) ;

        void set_geometry(
            const std::vector< index_t >& facets,
            const std::vector< index_t >& facet_ptr ) ;

        void set_adjacent( const std::vector< index_t >& adjacent )
        {
            ringmesh_assert( adjacent.size() == mesh_.facet_corners.nb() ) ;
            for( index_t i = 0; i < adjacent.size(); i++ ) {
                mesh_.facet_corners.set_adjacent_facet( i, adjacent[i] ) ;
            }
        }

        void cut_by_line( const Line& L ) ;

    public:
        SurfaceTools tools ;
    } ;

        

    /*!
     * @brief Class to answer geometrical requests on a BoundaryModelElement
     */
    class RINGMESH_API BoundaryModelElementMeasure {
    public:
        static double size( const BoundaryModelElement* E ) ;

        static double cell_size(
            const BoundaryModelElement* E,
            index_t cell ) ;

        static double distance(
            const BoundaryModelElement* from,
            const vec3& p ) ;

        static double distance(
            const BoundaryModelElement* from,
            const BoundaryModelElement* to ) ;

        static vec3 barycenter( const BoundaryModelElement* E ) ;

        static vec3 barycenter(
            const BoundaryModelElement* E,
            const std::vector< index_t >& cells ) ;
    } ;
} // namespace

#endif
