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

#include <grgmeshlib/aabb_tree.h>
#include <grgmeshlib/reorder.h>
#include <grgmeshlib/permutation.h>

#include <fstream>

namespace GRGMesh {
    static uint32 max_node_index(
        uint32 node_index,
        uint32 b,
        uint32 e )
    {
        grgmesh_debug_assert( e > b ) ;
        if( b + 1 == e ) {
            return node_index ;
        }
        uint32 m = b + ( e - b ) / 2 ;
        uint32 childl = 2 * node_index ;
        uint32 childr = 2 * node_index + 1 ;
        return std::max( max_node_index( childl, b, m ),
            max_node_index( childr, m, e ) ) ;
    }
/*
    static void init_bboxes_recursive(
        const TetraMesh& M,
        std::vector< Box3d >& bboxes,
        uint32 node_index,
        uint32 b,
        uint32 e )
    {
        grgmesh_debug_assert( node_index < bboxes.size() ) ;
        grgmesh_debug_assert( b != e ) ;
        if( b + 1 == e ) {
            Box3d bbox ;
            bbox.add_point( M.vertex( b, 0 ) ) ;
            bbox.add_point( M.vertex( b, 1 ) ) ;
            bbox.add_point( M.vertex( b, 2 ) ) ;
            bbox.add_point( M.vertex( b, 3 ) ) ;
            bboxes[node_index] = bbox ;
            return ;
        }
        uint32 m = b + ( e - b ) / 2 ;
        uint32 childl = 2 * node_index ;
        uint32 childr = 2 * node_index + 1 ;
        grgmesh_debug_assert( childl < bboxes.size() ) ;
        grgmesh_debug_assert( childr < bboxes.size() ) ;
        init_bboxes_recursive( M, bboxes, childl, b, m ) ;
        init_bboxes_recursive( M, bboxes, childr, m, e ) ;
        grgmesh_debug_assert( childl < bboxes.size() ) ;
        grgmesh_debug_assert( childr < bboxes.size() ) ;
        bboxes[node_index] = bboxes[childl].bbox_union( bboxes[childr] ) ;
    }
*/
    static void init_bboxes_recursive(
        const GRGMesh::SurfacePart& M,
        std::vector< Box3d >& bboxes,
        uint32 node_index,
        uint32 b,
        uint32 e )
    {
        grgmesh_debug_assert( node_index < bboxes.size() ) ;
        grgmesh_debug_assert( b != e ) ;
        if( b + 1 == e ) {
            Box3d bbox ;
            bbox.add_point( M.point( b, 0 ) ) ;
            bbox.add_point( M.point( b, 1 ) ) ;
            bbox.add_point( M.point( b, 2 ) ) ;
            bboxes[node_index] = bbox ;
            return ;
        }
        uint32 m = b + ( e - b ) / 2 ;
        uint32 childl = 2 * node_index ;
        uint32 childr = 2 * node_index + 1 ;
        grgmesh_debug_assert( childl < bboxes.size() ) ;
        grgmesh_debug_assert( childr < bboxes.size() ) ;
        init_bboxes_recursive( M, bboxes, childl, b, m ) ;
        init_bboxes_recursive( M, bboxes, childr, m, e ) ;
        grgmesh_debug_assert( childl < bboxes.size() ) ;
        grgmesh_debug_assert( childr < bboxes.size() ) ;
        bboxes[node_index] = bboxes[childl].bbox_union( bboxes[childr] ) ;
    }

/*
    TetAABBTree::TetAABBTree( TetraMesh& M )
        : mesh_( M )
    {
        mesh_.reorder_morton() ;
        bboxes_.resize( max_node_index( 1, 0, mesh_.nb_tetra() ) + 1 ) ; // <-- this is because size == max_index + 1 !!!
        init_bboxes_recursive( mesh_, bboxes_, 1, 0, mesh_.nb_tetra() ) ;
    }

    void TetAABBTree::get_nearest_tet_hint(
        const vec3& p,
        uint32& nearest_t,
        vec3& nearest_point,
        float64& sq_dist ) const
    {

        // Find a good initial value for nearest_f by traversing
        // the boxes and selecting the child such that the center
        // of its bounding box is nearer to the query point.
        // For a large mesh (20M facets) this gains up to 10%
        // performance as compared to picking nearest_f randomly.
        uint32 b = 0 ;
        uint32 e = mesh_.nb_tetra() - 1 ;
        uint32 n = 1 ;
        while( e != b + 1 ) {
            uint32 m = b + ( e - b ) / 2 ;
            uint32 childl = 2 * n ;
            uint32 childr = 2 * n + 1 ;
            if( bboxes_[childl].distance_to_center( p )
                < bboxes_[childr].distance_to_center( p ) ) {
                e = m ;
                n = childl ;
            } else {
                b = m ;
                n = childr ;
            }
        }
        nearest_t = b ;

        nearest_point = mesh_.vertex( nearest_t, 0 ) ;
        sq_dist = length2( p - nearest_point ) ;
    }

    void TetAABBTree::nearest_tet_recursive(
        const vec3& p,
        uint32& nearest_t,
        vec3& nearest_point,
        float64& sq_dist,
        uint32 n,
        uint32 b,
        uint32 e ) const
    {
        ogf_debug_assert( e > b ) ;

        // If node is a leaf: compute point-facet distance
        // and replace current if nearer
        if( b + 1 == e ) {
            vec3 cur_nearest_point ;
            float64 cur_sq_dist = mesh_.get_nearest_point( p, b, cur_nearest_point ) ;
            if( cur_sq_dist < sq_dist ) {
                nearest_t = b ;
                nearest_point = cur_nearest_point ;
                sq_dist = cur_sq_dist ;
            }
            return ;
        }
        uint32 m = b + ( e - b ) / 2 ;
        uint32 childl = 2 * n ;
        uint32 childr = 2 * n + 1 ;

        float64 dl = bboxes_[childl].signed_distance( p ) ;
        float64 dr = bboxes_[childr].signed_distance( p ) ;

        // Traverse the "nearest" child first, so that it has more chances
        // to prune the traversal of the other child.
        if( dl < dr ) {
            if( dl < sq_dist ) {
                nearest_tet_recursive( p, nearest_t, nearest_point, sq_dist,
                    childl, b, m ) ;
            }
            if( dr < sq_dist ) {
                nearest_tet_recursive( p, nearest_t, nearest_point, sq_dist,
                    childr, m, e ) ;
            }
        } else {
            if( dr < sq_dist ) {
                nearest_tet_recursive( p, nearest_t, nearest_point, sq_dist,
                    childr, m, e ) ;
            }
            if( dl < sq_dist ) {
                nearest_tet_recursive( p, nearest_t, nearest_point, sq_dist,
                    childl, b, m ) ;
            }
        }
    }
*/

    static void reorder_morton( SurfacePart& M )
    {
        std::vector< int32 > sorted_indices ;
        SurfacePartMutator mutator( M ) ;

        // Step 1: reorder vertices
        morton_facet_vertex_sort( M, sorted_indices ) ;

        Permutation::apply( mutator.points(), sorted_indices ) ;
        if( M.is_resolution_set() ) {
            Permutation::apply( mutator.resolution(), sorted_indices ) ;
        }

        Permutation::invert(sorted_indices) ;
        std::vector< uint32 >& facets = mutator.facets() ;
        for( uint32 t = 0; t < M.nb_simplices(); t++ ) {
            for( uint32 p = 0; p < 3; p++ ) {
                facets[3*t+p] = sorted_indices[facets[3*t+p]] ;
            }
        }

        sorted_indices.resize( 0 ) ;

        // Step 2: reorder facets
        morton_facet_sort( M, sorted_indices ) ;

        Permutation::apply( &mutator.facets()[0], sorted_indices, sizeof(uint32) * 3 ) ;
        std::ofstream out( "out") ;
        for( unsigned int i = 0 ; i < mutator.adjacents().size(); i++ )
            out << mutator.adjacents()[i] << std::endl ;

        Permutation::apply( &mutator.adjacents()[0], sorted_indices, sizeof(int32) * 3 ) ;

        std::ofstream out2( "out2") ;
        for( unsigned int i = 0 ; i < mutator.adjacents().size(); i++ )
            out2 << mutator.adjacents()[i] << std::endl ;

        if( M.is_U_set() ) {
            Permutation::apply( mutator.U(), sorted_indices ) ;
        }
        if( M.is_V_set() ) {
            Permutation::apply( mutator.V(), sorted_indices ) ;
        }
        if( M.is_W_set() ) {
            Permutation::apply( mutator.W(), sorted_indices ) ;
        }

        Permutation::invert(sorted_indices) ;
        std::vector< int32 >& adjacents = mutator.adjacents() ;
        for( uint32 t = 0; t < M.nb_simplices(); t++ ) {
            for( uint32 p = 0; p < 3; p++ ) {
                if( !M.is_on_border(t,p) ) {
                    adjacents[3*t+p] = sorted_indices[adjacents[3*t+p]] ;
                }
            }
        }
    }

    static float64 get_nearest_point(
        const SurfacePart& M,
        const vec3& p,
        int t,
        vec3& nearest_p )
    {
        const vec3& p1 = M.point( t, 0 ) ;
        const vec3& p2 = M.point( t, 1 ) ;
        const vec3& p3 = M.point( t, 2 ) ;
        float64 distance = Utils::point_triangle_squared_distance( p, p1, p2, p3,
            nearest_p ) ;
        if( Utils::point_inside_triangle( p, p1, p2, p3 ) ) {
            nearest_p = p ;
            distance = -distance ;
        }

        return distance ;
    }

    FacetAABBTree::FacetAABBTree( SurfacePart& M )
        : mesh_( M )
    {
        reorder_morton( mesh_) ;
        bboxes_.resize( max_node_index( 1, 0, mesh_.nb_simplices() ) + 1 ) ; // <-- this is because size == max_index + 1 !!!
        init_bboxes_recursive( mesh_, bboxes_, 1, 0, mesh_.nb_simplices() ) ;
    }

    void FacetAABBTree::get_nearest_facet_hint(
        const vec3& p,
        uint32& nearest_t,
        vec3& nearest_point,
        float64& sq_dist ) const
    {

        // Find a good initial value for nearest_f by traversing
        // the boxes and selecting the child such that the center
        // of its bounding box is nearer to the query point.
        // For a large mesh (20M facets) this gains up to 10%
        // performance as compared to picking nearest_f randomly.
        uint32 b = 0 ;
        uint32 e = mesh_.nb_simplices() - 1 ;
        if( e > 0 ) {
            uint32 n = 1 ;
            while( e != b + 1 ) {
                uint32 m = b + ( e - b ) / 2 ;
                uint32 childl = 2 * n ;
                uint32 childr = 2 * n + 1 ;
                if( bboxes_[childl].distance_to_center( p )
                    < bboxes_[childr].distance_to_center( p ) ) {
                    e = m ;
                    n = childl ;
                } else {
                    b = m ;
                    n = childr ;
                }
            }
        }
        nearest_t = b ;

        nearest_point = mesh_.point( nearest_t, 0 ) ;
        sq_dist = length2( p - nearest_point ) ;
    }

    void FacetAABBTree::nearest_facet_recursive(
        const vec3& p,
        uint32& nearest_t,
        vec3& nearest_point,
        float64& sq_dist,
        uint32 n,
        uint32 b,
        uint32 e ) const
    {
        grgmesh_debug_assert( e > b ) ;

        // If node is a leaf: compute point-facet distance
        // and replace current if nearer
        if( b + 1 == e ) {
            vec3 cur_nearest_point ;
            float64 cur_sq_dist = get_nearest_point( mesh_, p, b, cur_nearest_point ) ;
            if( cur_sq_dist < sq_dist ) {
                nearest_t = b ;
                nearest_point = cur_nearest_point ;
                sq_dist = cur_sq_dist ;
            }
            return ;
        }
        uint32 m = b + ( e - b ) / 2 ;
        uint32 childl = 2 * n ;
        uint32 childr = 2 * n + 1 ;

        float64 dl = bboxes_[childl].signed_distance( p ) ;
        float64 dr = bboxes_[childr].signed_distance( p ) ;

        // Traverse the "nearest" child first, so that it has more chances
        // to prune the traversal of the other child.
        if( dl < dr ) {
            if( dl < sq_dist ) {
                nearest_facet_recursive( p, nearest_t, nearest_point, sq_dist,
                    childl, b, m ) ;
            }
            if( dr < sq_dist ) {
                nearest_facet_recursive( p, nearest_t, nearest_point, sq_dist,
                    childr, m, e ) ;
            }
        } else {
            if( dr < sq_dist ) {
                nearest_facet_recursive( p, nearest_t, nearest_point, sq_dist,
                    childr, m, e ) ;
            }
            if( dl < sq_dist ) {
                nearest_facet_recursive( p, nearest_t, nearest_point, sq_dist,
                    childl, b, m ) ;
            }
        }
    }

}
