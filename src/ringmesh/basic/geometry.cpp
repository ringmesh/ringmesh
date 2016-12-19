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

#include <ringmesh/basic/geometry.h>

#include <geogram/mesh/mesh.h>
#include <geogram/mesh/mesh_geometry.h>

#include <geogram/numerics/predicates.h>

#include <ringmesh/basic/algorithm.h>
#include <ringmesh/geogram_extension/geogram_extension.h>

/*!
 * @file Basic geometrical requests 
 * @author Arnaud Botella
 *
 * @todo Comment on the robustness of the tests
 */

namespace {
    using namespace RINGMesh ;

    bool is_almost_zero( const double& value )
    {
        return value < global_epsilon && value > -global_epsilon ;
    }

    double triangle_signed_area(
        const vec3& p0,
        const vec3& p1,
        const vec3& p2,
        const vec3& triangle_normal )
    {
        double area = GEO::Geom::triangle_area( p0, p1, p2 ) ;
        vec3 area_normal = cross( p0 - p2, p1 - p2 ) ;
        if( dot( triangle_normal, area_normal ) < 0 ) {
            area = -area ;
        }
        return area ;
    }
}

namespace RINGMesh {

    bool operator==( const vec3& u, const vec3& v )
    {
        return u.x == v.x && u.y == v.y && u.z == v.z ;
    }

    bool operator<( const vec3& u, const vec3& v )
    {
        return u.x < v.x && u.y < v.y && u.z < v.z ;
    }

    bool operator!=( const vec3& u, const vec3& v )
    {
        return u.x != v.x || u.y != v.y || u.z != v.z ;
    }

    double point_triangle_distance(
        const vec3& point,
        const vec3& V0,
        const vec3& V1,
        const vec3& V2,
        vec3& closest_point,
        double& lambda0,
        double& lambda1,
        double& lambda2 )
    {
        vec3 diff = V0 - point ;
        vec3 edge0 = V1 - V0 ;
        vec3 edge1 = V2 - V0 ;
        double a00 = length2( edge0 ) ;
        double a01 = dot( edge0, edge1 ) ;
        double a11 = length2( edge1 ) ;
        double b0 = dot( diff, edge0 ) ;
        double b1 = dot( diff, edge1 ) ;
        double c = length2( diff ) ;
        double det = std::fabs( a00 * a11 - a01 * a01 ) ;
        double s = a01 * b1 - a11 * b0 ;
        double t = a01 * b0 - a00 * b1 ;
        double sqrDistance ;

        if( s + t <= det ) {
            if( s < 0.0 ) {
                if( t < 0.0 ) { // region 4
                    if( b0 < 0.0 ) {
                        t = 0.0 ;
                        if( -b0 >= a00 ) {
                            s = 1.0 ;
                            sqrDistance = a00 + 2.0 * b0 + c ;
                        } else {
                            s = -b0 / a00 ;
                            sqrDistance = b0 * s + c ;
                        }
                    } else {
                        s = 0.0 ;
                        if( b1 >= 0.0 ) {
                            t = 0.0 ;
                            sqrDistance = c ;
                        } else if( -b1 >= a11 ) {
                            t = 1.0 ;
                            sqrDistance = a11 + 2.0 * b1 + c ;
                        } else {
                            t = -b1 / a11 ;
                            sqrDistance = b1 * t + c ;
                        }
                    }
                } else { // region 3
                    s = 0.0 ;
                    if( b1 >= 0.0 ) {
                        t = 0.0 ;
                        sqrDistance = c ;
                    } else if( -b1 >= a11 ) {
                        t = 1.0 ;
                        sqrDistance = a11 + 2.0 * b1 + c ;
                    } else {
                        t = -b1 / a11 ;
                        sqrDistance = b1 * t + c ;
                    }
                }
            } else if( t < 0.0 ) { // region 5
                t = 0.0 ;
                if( b0 >= 0.0 ) {
                    s = 0.0 ;
                    sqrDistance = c ;
                } else if( -b0 >= a00 ) {
                    s = 1.0 ;
                    sqrDistance = a00 + 2.0 * b0 + c ;
                } else {
                    s = -b0 / a00 ;
                    sqrDistance = b0 * s + c ;
                }
            } else { // region 0
                // minimum at interior point
                double invDet = double( 1.0 ) / det ;
                s *= invDet ;
                t *= invDet ;
                sqrDistance = s * ( a00 * s + a01 * t + 2.0 * b0 )
                    + t * ( a01 * s + a11 * t + 2.0 * b1 ) + c ;
            }
        } else {
            double tmp0, tmp1, numer, denom ;

            if( s < 0.0 ) { // region 2
                tmp0 = a01 + b0 ;
                tmp1 = a11 + b1 ;
                if( tmp1 > tmp0 ) {
                    numer = tmp1 - tmp0 ;
                    denom = a00 - 2.0 * a01 + a11 ;
                    if( numer >= denom ) {
                        s = 1.0 ;
                        t = 0.0 ;
                        sqrDistance = a00 + 2.0 * b0 + c ;
                    } else {
                        s = numer / denom ;
                        t = 1.0 - s ;
                        sqrDistance = s * ( a00 * s + a01 * t + 2.0 * b0 )
                            + t * ( a01 * s + a11 * t + 2.0 * b1 ) + c ;
                    }
                } else {
                    s = 0.0 ;
                    if( tmp1 <= 0.0 ) {
                        t = 1.0 ;
                        sqrDistance = a11 + 2.0 * b1 + c ;
                    } else if( b1 >= 0.0 ) {
                        t = 0.0 ;
                        sqrDistance = c ;
                    } else {
                        t = -b1 / a11 ;
                        sqrDistance = b1 * t + c ;
                    }
                }
            } else if( t < 0.0 ) { // region 6
                tmp0 = a01 + b1 ;
                tmp1 = a00 + b0 ;
                if( tmp1 > tmp0 ) {
                    numer = tmp1 - tmp0 ;
                    denom = a00 - 2.0 * a01 + a11 ;
                    if( numer >= denom ) {
                        t = 1.0 ;
                        s = 0.0 ;
                        sqrDistance = a11 + 2.0 * b1 + c ;
                    } else {
                        t = numer / denom ;
                        s = 1.0 - t ;
                        sqrDistance = s * ( a00 * s + a01 * t + 2.0 * b0 )
                            + t * ( a01 * s + a11 * t + 2.0 * b1 ) + c ;
                    }
                } else {
                    t = 0.0 ;
                    if( tmp1 <= 0.0 ) {
                        s = 1.0 ;
                        sqrDistance = a00 + 2.0 * b0 + c ;
                    } else if( b0 >= 0.0 ) {
                        s = 0.0 ;
                        sqrDistance = c ;
                    } else {
                        s = -b0 / a00 ;
                        sqrDistance = b0 * s + c ;
                    }
                }
            } else { // region 1
                numer = a11 + b1 - a01 - b0 ;
                if( numer <= 0.0 ) {
                    s = 0.0 ;
                    t = 1.0 ;
                    sqrDistance = a11 + 2.0 * b1 + c ;
                } else {
                    denom = a00 - 2.0 * a01 + a11 ;
                    if( numer >= denom ) {
                        s = 1.0 ;
                        t = 0.0 ;
                        sqrDistance = a00 + 2.0 * b0 + c ;
                    } else {
                        s = numer / denom ;
                        t = 1.0 - s ;
                        sqrDistance = s * ( a00 * s + a01 * t + 2.0 * b0 )
                            + t * ( a01 * s + a11 * t + 2.0 * b1 ) + c ;
                    }
                }
            }
        }

        // Account for numerical round-off error.
        if( sqrDistance < 0.0 ) {
            sqrDistance = 0.0 ;
        }

        closest_point = V0 + s * edge0 + t * edge1 ;
        lambda0 = 1.0 - s - t ;
        lambda1 = s ;
        lambda2 = t ;
        return sqrt( sqrDistance ) ;
    }

    /*!
     * Computes the distance between a point and a tetrahedron
     * @param[in] p the point
     * @param[in] p0 the first vertex of the tetrahedron
     * @param[in] p1 the second vertex of the tetrahedron
     * @param[in] p2 the third vertex of the tetrahedron
     * @param[in] p3 the fourth vertex of the tetrahedron
     * @param[out] nearest_p the nearest point on the tetrahedron
     * @return the distance between the point and the tetrahedron facets
     */
    double point_tetra_distance(
        const vec3& p,
        const vec3& p0,
        const vec3& p1,
        const vec3& p2,
        const vec3& p3,
        vec3& nearest_p )
    {
        vec3 vertices[4] = { p0, p1, p2, p3 } ;
        double not_used0, not_used1, not_used2 ;
        double dist = max_float64() ;
        for( index_t f = 0; f < GEO::MeshCellDescriptors::tet_descriptor.nb_facets;
            f++ ) {
            vec3 cur_p ;
            double distance =
                point_triangle_distance( p,
                    vertices[GEO::MeshCellDescriptors::tet_descriptor.facet_vertex[f][0]],
                    vertices[GEO::MeshCellDescriptors::tet_descriptor.facet_vertex[f][1]],
                    vertices[GEO::MeshCellDescriptors::tet_descriptor.facet_vertex[f][2]],
                    cur_p, not_used0, not_used1, not_used2 ) ;
            if( distance < dist ) {
                dist = distance ;
                nearest_p = cur_p ;
            }
        }
        return dist ;
    }

    /*!
     * Computes the distance between a point and a pyramid
     * @param[in] p the point
     * @param[in] p0 the first vertex of the pyramid
     * @param[in] p1 the second vertex of the pyramid
     * @param[in] p2 the third vertex of the pyramid
     * @param[in] p3 the fourth vertex of the pyramid
     * @param[in] p4 the fifth vertex of the pyramid
     * @param[out] nearest_p the nearest point on the pyramid
     * @return the distance between the point and the pyramid facets
     */
    double point_pyramid_distance(
        const vec3& p,
        const vec3& p0,
        const vec3& p1,
        const vec3& p2,
        const vec3& p3,
        const vec3& p4,
        vec3& nearest_p )
    {
        vec3 vertices[5] = { p0, p1, p2, p3, p4 } ;
        double not_used0, not_used1, not_used2 ;
        double dist = max_float64() ;
        for( index_t f = 0;
            f < GEO::MeshCellDescriptors::pyramid_descriptor.nb_facets; f++ ) {
            vec3 cur_p ;
            double distance = max_float64() ;
            index_t nb_vertices =
                GEO::MeshCellDescriptors::pyramid_descriptor.nb_vertices_in_facet[f] ;
            if( nb_vertices == 3 ) {
                distance =
                    point_triangle_distance( p,
                        vertices[GEO::MeshCellDescriptors::pyramid_descriptor.facet_vertex[f][0]],
                        vertices[GEO::MeshCellDescriptors::pyramid_descriptor.facet_vertex[f][1]],
                        vertices[GEO::MeshCellDescriptors::pyramid_descriptor.facet_vertex[f][2]],
                        cur_p, not_used0, not_used1, not_used2 ) ;
            } else if( nb_vertices == 4 ) {
                distance =
                    point_quad_distance( p,
                        vertices[GEO::MeshCellDescriptors::pyramid_descriptor.facet_vertex[f][0]],
                        vertices[GEO::MeshCellDescriptors::pyramid_descriptor.facet_vertex[f][1]],
                        vertices[GEO::MeshCellDescriptors::pyramid_descriptor.facet_vertex[f][2]],
                        vertices[GEO::MeshCellDescriptors::pyramid_descriptor.facet_vertex[f][3]],
                        cur_p ) ;
            } else {
                ringmesh_assert_not_reached ;
            }
            if( distance < dist ) {
                dist = distance ;
                nearest_p = cur_p ;
            }
        }
        return dist ;
    }

    /*!
     * Computes the distance between a point and a prism
     * @param[in] p the point
     * @param[in] p0 the first vertex of the prism
     * @param[in] p1 the second vertex of the prism
     * @param[in] p2 the third vertex of the prism
     * @param[in] p3 the fourth vertex of the prism
     * @param[in] p4 the fifth vertex of the prism
     * @param[in] p5 the sixth vertex of the prism
     * @param[out] nearest_p the nearest point on the prism
     * @return the distance between the point and the prism facets
     */
    double point_prism_distance(
        const vec3& p,
        const vec3& p0,
        const vec3& p1,
        const vec3& p2,
        const vec3& p3,
        const vec3& p4,
        const vec3& p5,
        vec3& nearest_p )
    {
        vec3 vertices[6] = { p0, p1, p2, p3, p4, p5 } ;
        double not_used0, not_used1, not_used2 ;

        double dist = max_float64() ;
        for( index_t f = 0; f < GEO::MeshCellDescriptors::prism_descriptor.nb_facets;
            f++ ) {
            vec3 cur_p ;
            double distance = max_float64() ;
            index_t nb_vertices =
                GEO::MeshCellDescriptors::prism_descriptor.nb_vertices_in_facet[f] ;
            if( nb_vertices == 3 ) {
                distance =
                    point_triangle_distance( p,
                        vertices[GEO::MeshCellDescriptors::prism_descriptor.facet_vertex[f][0]],
                        vertices[GEO::MeshCellDescriptors::prism_descriptor.facet_vertex[f][1]],
                        vertices[GEO::MeshCellDescriptors::prism_descriptor.facet_vertex[f][2]],
                        cur_p, not_used0, not_used1, not_used2 ) ;
            } else if( nb_vertices == 4 ) {
                distance =
                    point_quad_distance( p,
                        vertices[GEO::MeshCellDescriptors::prism_descriptor.facet_vertex[f][0]],
                        vertices[GEO::MeshCellDescriptors::prism_descriptor.facet_vertex[f][1]],
                        vertices[GEO::MeshCellDescriptors::prism_descriptor.facet_vertex[f][2]],
                        vertices[GEO::MeshCellDescriptors::prism_descriptor.facet_vertex[f][3]],
                        cur_p ) ;
            } else {
                ringmesh_assert_not_reached ;
            }
            if( distance < dist ) {
                dist = distance ;
                nearest_p = cur_p ;
            }
        }
        return dist ;
    }
    /*!
     * Computes the distance between a point and a hexahedron
     * @param[in] p the point
     * @param[in] p0 the first vertex of the hexahedron
     * @param[in] p1 the second vertex of the hexahedron
     * @param[in] p2 the third vertex of the hexahedron
     * @param[in] p3 the fourth vertex of the hexahedron
     * @param[in] p4 the fifth vertex of the hexahedron
     * @param[in] p5 the sixth vertex of the hexahedron
     * @param[in] p6 the seventh vertex of the hexahedron
     * @param[in] p7 the heith vertex of the hexahedron
     * @param[out] nearest_p the nearest point on the hexahedron
     * @return the distance between the point and the hexahedron facets
     */
    double point_hexa_distance(
        const vec3& p,
        const vec3& p0,
        const vec3& p1,
        const vec3& p2,
        const vec3& p3,
        const vec3& p4,
        const vec3& p5,
        const vec3& p6,
        const vec3& p7,
        vec3& nearest_p )
    {
        /// Review: Why not input an array ?
        vec3 vertices[8] = { p0, p1, p2, p3, p4, p5, p6, p7 } ;
        double dist = max_float64() ;
        for( index_t f = 0; f < GEO::MeshCellDescriptors::hex_descriptor.nb_facets;
            f++ ) {
            vec3 cur_p ;
            double distance =
                point_quad_distance( p,
                    vertices[GEO::MeshCellDescriptors::hex_descriptor.facet_vertex[f][0]],
                    vertices[GEO::MeshCellDescriptors::hex_descriptor.facet_vertex[f][1]],
                    vertices[GEO::MeshCellDescriptors::hex_descriptor.facet_vertex[f][2]],
                    vertices[GEO::MeshCellDescriptors::hex_descriptor.facet_vertex[f][3]],
                    cur_p ) ;
            if( distance < dist ) {
                dist = distance ;
                nearest_p = cur_p ;
            }
        }
        return dist ;
    }

    /*!
     * Tests if a point is inside a tetrahedron
     * @param[in] p the point to test
     * @param[in] p0 the first vertex of the tetrahedron
     * @param[in] p1 the second vertex of the tetrahedron
     * @param[in] p2 the third vertex of the tetrahedron
     * @param[in] p3 the fourth vertex of the tetrahedron
     * @param[in] exact_predicates if true, the algorithm uses exact predicates
     * @return returns true if the point is inside the tetrahedron
     */
    bool point_inside_tetra(
        const vec3& p,
        const vec3& p0,
        const vec3& p1,
        const vec3& p2,
        const vec3& p3,
        bool exact_predicates )
    {
        vec3 vertices[4] = { p0, p1, p2, p3 } ;
        Sign signs[4] ;
        if( !exact_predicates ) {
            for( index_t f = 0;
                f < GEO::MeshCellDescriptors::tet_descriptor.nb_facets; f++ ) {
                double volume =
                    GEO::Geom::tetra_signed_volume( p,
                        vertices[GEO::MeshCellDescriptors::tet_descriptor.facet_vertex[f][0]],
                        vertices[GEO::MeshCellDescriptors::tet_descriptor.facet_vertex[f][1]],
                        vertices[GEO::MeshCellDescriptors::tet_descriptor.facet_vertex[f][2]] ) ;
                if( is_almost_zero( volume ) ) {
                    return point_inside_tetra( p, p0, p1, p2, p3, true ) ;
                }
                signs[f] = sign( volume ) ;
            }
        } else {
            for( index_t f = 0;
                f < GEO::MeshCellDescriptors::tet_descriptor.nb_facets; f++ ) {
                signs[f] =
                    sign(
                        GEO::PCK::orient_3d( p.data(),
                            vertices[GEO::MeshCellDescriptors::tet_descriptor.facet_vertex[f][0]].data(),
                            vertices[GEO::MeshCellDescriptors::tet_descriptor.facet_vertex[f][1]].data(),
                            vertices[GEO::MeshCellDescriptors::tet_descriptor.facet_vertex[f][2]].data() ) ) ;
            }
        }
        return ( signs[0] >= 0 && signs[1] >= 0 && signs[2] >= 0 && signs[3] >= 0 )
            || ( signs[0] <= 0 && signs[1] <= 0 && signs[2] <= 0 && signs[3] <= 0 ) ;
    }

    /*!
     * Tests if a point is inside a pyramid
     * @param[in] p the point to test
     * @param[in] p0 the first vertex of the pyramid
     * @param[in] p1 the second vertex of the pyramid
     * @param[in] p2 the third vertex of the pyramid
     * @param[in] p3 the fourth vertex of the pyramid
     * @param[in] p4 the fifth vertex of the pyramid
     * @param[in] exact_predicates if true, the algorithm uses exact predicates
     * @return returns true if the point is inside the pyramid
     */
    bool point_inside_pyramid(
        const vec3& p,
        const vec3& p0,
        const vec3& p1,
        const vec3& p2,
        const vec3& p3,
        const vec3& p4,
        bool exact_predicates )
    {
        vec3 vertices[5] = { p0, p1, p2, p3, p4 } ;
        Sign signs[5] ;
        if( !exact_predicates ) {
            for( index_t f = 0;
                f < GEO::MeshCellDescriptors::pyramid_descriptor.nb_facets; f++ ) {
                double volume =
                    GEO::Geom::tetra_signed_volume( p,
                        vertices[GEO::MeshCellDescriptors::pyramid_descriptor.facet_vertex[f][0]],
                        vertices[GEO::MeshCellDescriptors::pyramid_descriptor.facet_vertex[f][1]],
                        vertices[GEO::MeshCellDescriptors::pyramid_descriptor.facet_vertex[f][2]] ) ;
                if( is_almost_zero( volume ) ) {
                    return point_inside_pyramid( p, p0, p1, p2, p3, p4, true ) ;
                }
                signs[f] = sign( volume ) ;
            }
        } else {
            for( index_t f = 0;
                f < GEO::MeshCellDescriptors::pyramid_descriptor.nb_facets; f++ ) {
                signs[f] =
                    sign(
                        GEO::PCK::orient_3d( p.data(),
                            vertices[GEO::MeshCellDescriptors::pyramid_descriptor.facet_vertex[f][0]].data(),
                            vertices[GEO::MeshCellDescriptors::pyramid_descriptor.facet_vertex[f][1]].data(),
                            vertices[GEO::MeshCellDescriptors::pyramid_descriptor.facet_vertex[f][2]].data() ) ) ;
            }
        }
        return ( signs[0] >= 0 && signs[1] >= 0 && signs[2] >= 0 && signs[3] >= 0
            && signs[4] >= 0 )
            || ( signs[0] <= 0 && signs[1] <= 0 && signs[2] <= 0 && signs[3] <= 0
                && signs[4] <= 0 ) ;
    }

    /*!
     * Tests if a point is inside a prism
     * @param[in] p the point to test
     * @param[in] p0 the first vertex of the prism
     * @param[in] p1 the second vertex of the prism
     * @param[in] p2 the third vertex of the prism
     * @param[in] p3 the fourth vertex of the prism
     * @param[in] p4 the fifth vertex of the prism
     * @param[in] p5 the sixth vertex of the prism
     * @param[in] exact_predicates if true, the algorithm uses exact predicates
     * @return returns true if the point is inside the prism
     */
    bool point_inside_prism(
        const vec3& p,
        const vec3& p0,
        const vec3& p1,
        const vec3& p2,
        const vec3& p3,
        const vec3& p4,
        const vec3& p5,
        bool exact_predicates )
    {
        vec3 vertices[6] = { p0, p1, p2, p3, p4, p5 } ;
        Sign signs[5] ;
        if( !exact_predicates ) {
            for( index_t f = 0;
                f < GEO::MeshCellDescriptors::prism_descriptor.nb_facets; f++ ) {
                double volume =
                    GEO::Geom::tetra_signed_volume( p,
                        vertices[GEO::MeshCellDescriptors::prism_descriptor.facet_vertex[f][0]],
                        vertices[GEO::MeshCellDescriptors::prism_descriptor.facet_vertex[f][1]],
                        vertices[GEO::MeshCellDescriptors::prism_descriptor.facet_vertex[f][2]] ) ;
                if( is_almost_zero( volume ) ) {
                    return point_inside_prism( p, p0, p1, p2, p3, p4, p5, true ) ;
                }
                signs[f] = sign( volume ) ;
            }
        } else {
            for( index_t f = 0;
                f < GEO::MeshCellDescriptors::prism_descriptor.nb_facets; f++ ) {
                signs[f] =
                    sign(
                        GEO::PCK::orient_3d( p.data(),
                            vertices[GEO::MeshCellDescriptors::prism_descriptor.facet_vertex[f][0]].data(),
                            vertices[GEO::MeshCellDescriptors::prism_descriptor.facet_vertex[f][1]].data(),
                            vertices[GEO::MeshCellDescriptors::prism_descriptor.facet_vertex[f][2]].data() ) ) ;
            }
        }
        return ( signs[0] >= 0 && signs[1] >= 0 && signs[2] >= 0 && signs[3] >= 0
            && signs[4] >= 0 )
            || ( signs[0] <= 0 && signs[1] <= 0 && signs[2] <= 0 && signs[3] <= 0
                && signs[4] <= 0 ) ;
    }
    /*!
     * Tests if a point is inside a hexahedron
     * @param[in] p the point to test
     * @param[in] p0 the first vertex of the hexahedron
     * @param[in] p1 the second vertex of the hexahedron
     * @param[in] p2 the third vertex of the hexahedron
     * @param[in] p3 the fourth vertex of the hexahedron
     * @param[in] p4 the fifth vertex of the hexahedron
     * @param[in] p5 the sixth vertex of the hexahedron
     * @param[in] p6 the seventh vertex of the hexahedron
     * @param[in] p7 the heigth vertex of the hexahedron
     * @param[in] exact_predicates if true, the algorithm uses exact predicates
     * @return returns true if the point is inside the hexahedron
     */
    bool point_inside_hexa(
        const vec3& p,
        const vec3& p0,
        const vec3& p1,
        const vec3& p2,
        const vec3& p3,
        const vec3& p4,
        const vec3& p5,
        const vec3& p6,
        const vec3& p7,
        bool exact_predicates )
    {
        vec3 vertices[8] = { p0, p1, p2, p3, p4, p5, p6, p7 } ;
        Sign signs[6] ;
        if( !exact_predicates ) {
            for( index_t f = 0;
                f < GEO::MeshCellDescriptors::hex_descriptor.nb_facets; f++ ) {
                double volume =
                    GEO::Geom::tetra_signed_volume( p,
                        vertices[GEO::MeshCellDescriptors::hex_descriptor.facet_vertex[f][0]],
                        vertices[GEO::MeshCellDescriptors::hex_descriptor.facet_vertex[f][1]],
                        vertices[GEO::MeshCellDescriptors::hex_descriptor.facet_vertex[f][2]] ) ;
                if( is_almost_zero( volume ) ) {
                    return point_inside_hexa( p, p0, p1, p2, p3, p4, p5, p6, p7,
                        true ) ;
                }
                signs[f] = sign( volume ) ;
            }
        } else {
            for( index_t f = 0;
                f < GEO::MeshCellDescriptors::hex_descriptor.nb_facets; f++ ) {
                signs[f] =
                    sign(
                        GEO::PCK::orient_3d( p.data(),
                            vertices[GEO::MeshCellDescriptors::hex_descriptor.facet_vertex[f][0]].data(),
                            vertices[GEO::MeshCellDescriptors::hex_descriptor.facet_vertex[f][1]].data(),
                            vertices[GEO::MeshCellDescriptors::hex_descriptor.facet_vertex[f][2]].data() ) ) ;
            }
        }
        return ( signs[0] >= 0 && signs[1] >= 0 && signs[2] >= 0 && signs[3] >= 0
            && signs[4] >= 0 && signs[5] >= 0 )
            || ( signs[0] <= 0 && signs[1] <= 0 && signs[2] <= 0 && signs[3] <= 0
                && signs[4] <= 0 && signs[5] <= 0 ) ;
    }

    /*!
     * Computes the intersection(s) between a circle and a plane
     * @param[in] O_plane a point on the plane
     * @param[in] N_plane the normal of the plane
     * @param[in] O_circle the center of the circle
     * @param[in] N_circle the normal of the plane supporting the circle
     * @param[in] r the radius of the circle
     * @param[out] result the intersected points
     * @return returns true if there is at least one intersection
     */
    bool circle_plane_intersection(
        const vec3& O_plane,
        const vec3& N_plane,
        const vec3& O_circle,
        const vec3& N_circle,
        double r,
        std::vector< vec3 >& result )
    {
        vec3 O_inter, D_inter ;
        vec3 norm_N_plane = normalize( N_plane ) ;
        vec3 norm_N_circle = normalize( N_circle ) ;
        if( !plane_plane_intersection( O_plane, norm_N_plane, O_circle,
            norm_N_circle, O_inter, D_inter ) ) {
            return false ;
        }

        // http://www.geometrictools.com/LibMathematics/Intersection/Intersection.html
        // Locate one or two points that are on the circle and line.  If the
        // line is t*D+P, the circle center is C, and the circle radius is r,
        // then r^2 = |t*D+P-C|^2 = |D|^2*t^2 + 2*Dot(D,P-C)*t + |P-C|^2.  This
        // is a quadratic equation of the form:  a2*t^2 + 2*a1*t + a0 = 0.
        vec3 diff = O_inter - O_circle ;
        double a2 = D_inter.length2() ;
        double a1 = dot( diff, D_inter ) ;
        double a0 = diff.length2() - r * r ;

        double discr = a1 * a1 - a0 * a2 ;
        if( discr < 0.0 ) {
            return false ;
        }

        if( std::fabs( a2 ) < global_epsilon ) {
            return false ;
        }
        double inv = 1.0 / a2 ;
        if( discr < global_epsilon ) {
            result.push_back( vec3( O_inter - ( a1 * inv ) * D_inter ) ) ;
        } else {
            double root = sqrt( discr ) ;
            result.push_back( vec3( O_inter - ( ( a1 + root ) * inv ) * D_inter ) ) ;
            result.push_back( vec3( O_inter - ( ( a1 - root ) * inv ) * D_inter ) ) ;
        }
        return true ;
    }

    /*!
     * Computes the intersection between two planes
     * @param[in] O_P0 a point on the first plane
     * @param[in] N_P0 the normal of the frst plane
     * @param[in] O_P1 a point on the second plane
     * @param[in] N_P1 the normal of the second plane
     * @param[out] O_inter a point on the intersected line
     * @param[out] D_inter the direction of the intersected line
     * @return true is there is an intersection between the planes
     */
    bool plane_plane_intersection(
        const vec3& O_P0,
        const vec3& N_P0,
        const vec3& O_P1,
        const vec3& N_P1,
        vec3& O_inter,
        vec3& D_inter )
    {
        // http://www.geometrictools.com/LibMathematics/Intersection/Intersection.html
        // If N0 and N1 are parallel, either the planes are parallel and separated
        // or the same plane.  In both cases, 'false' is returned.  Otherwise,
        // the intersection line is
        //   L(t) = t*Cross(N0,N1)/|Cross(N0,N1)| + c0*N0 + c1*N1
        // for some coefficients c0 and c1 and for t any real number (the line
        // parameter).  Taking dot products with the normals,
        //   d0 = Dot(N0,L) = c0*Dot(N0,N0) + c1*Dot(N0,N1) = c0 + c1*d
        //   d1 = Dot(N1,L) = c0*Dot(N0,N1) + c1*Dot(N1,N1) = c0*d + c1
        // where d = Dot(N0,N1).  These are two equations in two unknowns.  The
        // solution is
        //   c0 = (d0 - d*d1)/det
        //   c1 = (d1 - d*d0)/det
        // where det = 1 - d^2.

        double d = dot( N_P0, N_P1 ) ;
        if( std::fabs( d - 1 ) < global_epsilon ) return false ;

        double invDet = 1.0 / ( 1.0 - d * d ) ;
        double const_P0 = dot( N_P0, O_P0 ) ;
        double const_P1 = dot( N_P1, O_P1 ) ;
        double c0 = ( const_P0 - d * const_P1 ) * invDet ;
        double c1 = ( const_P1 - d * const_P0 ) * invDet ;
        O_inter = c0 * N_P0 + c1 * N_P1 ;
        D_inter = cross( N_P0, N_P1 ) ;
        return true ;
    }

    /*!
     * Computes barycentric coordinates of \p p
     * @param[in] p the query point
     * @param[in] p0 the first tetra vertex
     * @param[in] p1 the second tetra vertex
     * @param[in] p2 the third tetra vertex
     * @param[in] p3 the fourth tetra vertex
     * @param[out] lambda the parametric coordinates corresponding to points
     * @return false if the computation failed because of too small tetrahedron volume
     */
    bool tetra_barycentric_coordinates(
        const vec3& p,
        const vec3& p0,
        const vec3& p1,
        const vec3& p2,
        const vec3& p3,
        double lambda[4] )
    {
        double total_volume = GEO::Geom::tetra_signed_volume( p0, p1, p2, p3 ) ;
        if( total_volume < global_epsilon_3 ) {
            for( index_t i = 0; i < 4; i++ ) {
                lambda[i] = 0 ;
            }
            return false ;
        }
        double volume0 = GEO::Geom::tetra_signed_volume( p1, p3, p2, p ) ;
        double volume1 = GEO::Geom::tetra_signed_volume( p0, p2, p3, p ) ;
        double volume2 = GEO::Geom::tetra_signed_volume( p0, p3, p1, p ) ;
        double volume3 = GEO::Geom::tetra_signed_volume( p0, p1, p2, p ) ;

        lambda[0] = volume0 / total_volume ;
        lambda[1] = volume1 / total_volume ;
        lambda[2] = volume2 / total_volume ;
        lambda[3] = volume3 / total_volume ;
        return true ;
    }

    /*!
      * Computes barycentric coordinates of \p p
      * @param[in] p the query point
      * @param[in] p0 the first triangle vertex
      * @param[in] p1 the second triangle vertex
      * @param[in] p2 the third triangle vertex
      * @param[out] lambda the parametric coordinates corresponding to points
      * @return false if the computation failed because of too small triangle area
      */
    bool triangle_barycentric_coordinates(
        const vec3& p,
        const vec3& p0,
        const vec3& p1,
        const vec3& p2,
        double lambda[3] )
    {
        double total_area = GEO::Geom::triangle_area( p0, p1, p2 ) ;
        if( total_area < global_epsilon_sq ) {
            for( index_t i = 0; i < 3; i++ ) {
                lambda[i] = 0 ;
            }
            return false ;
        }
        vec3 triangle_normal = cross( p2 - p0, p1 - p0 ) ;
        double area0 = triangle_signed_area( p2, p1, p, triangle_normal ) ;
        double area1 = triangle_signed_area( p0, p2, p, triangle_normal ) ;
        double area2 = triangle_signed_area( p1, p0, p, triangle_normal ) ;

        lambda[0] = area0 / total_area ;
        lambda[1] = area1 / total_area ;
        lambda[2] = area2 / total_area ;
        return true ;
    }

    /*!
     * Computes the intersection between a plane and a line
     * @param[in] O_line a point on the line
     * @param[in] D_line the direction of the plane
     * @param[in] O_plane a point on the plane
     * @param[in] N_plane the normal of the plane
     * @param[out] result the intersected point
     * @return returns true if there is an intersection
     */
    bool line_plane_intersection(
        const vec3& O_line,
        const vec3& D_line,
        const vec3& O_plane,
        const vec3& N_plane,
        vec3& result )
    {
        double dot_directions = dot( D_line, N_plane ) ;
        if( std::fabs( dot_directions ) > global_epsilon ) {
            double plane_constant = 0.0 ;
            for( index_t i = 0; i < 3; i++ ) {
                plane_constant += O_plane[i] * N_plane[i] ;
            }
            double signed_distance = dot( N_plane, O_line ) - plane_constant ;
            result = O_line - signed_distance * D_line / dot_directions ;
            return true ;
        } else {
            // line is parallel to the plane
            return false ;
        }
    }

    /*!
     * Computes the intersection between a plane and a segment
     * @param[in] p0 the first vertex of the segment
     * @param[in] p1 the second vertex of the segment
     * @param[in] O_plane a point on the plane
     * @param[in] N_plane the normal of the plane
     * @param[out] result the intersected point
     * @return returns true if there is an intersection
     */
    bool segment_plane_intersection(
        const vec3& seg0,
        const vec3& seg1,
        const vec3& O_plane,
        const vec3& N_plane,
        vec3& result )
    {
        vec3 segment_direction = normalize( seg1 - seg0 ) ;
        vec3 segment_barycenter = 0.5 * ( seg0 + seg1 ) ;
        vec3 line_plane_result ;
        if( line_plane_intersection( segment_barycenter, segment_direction, O_plane,
            N_plane, line_plane_result ) ) {
            if( ( line_plane_result - segment_barycenter ).length2()
                > ( seg0 - segment_barycenter ).length2() ) {
                // result outside the segment
                return false ;
            } else {
                result = line_plane_result ;
                return true ;
            }
        } else {
            return false ;
        }
    }

    /*!
     * Computes the intersection between a disk and a segment
     * @param[in] p0 the first vertex of the segment
     * @param[in] p1 the second vertex of the segment
     * @param[in] O_circle the center of the disk
     * @param[in] N_circle the normal of the plane supporting the disk
     * @param[in] r the radius of the disk
     * @param[out] result the intersected point
     * @return returns true if there is an intersection
     */
    bool disk_segment_intersection(
        const vec3& p0,
        const vec3& p1,
        const vec3& O_circle,
        const vec3& N_circle,
        double r,
        vec3& result )
    {
        vec3 segment_plane_result ;
        if( segment_plane_intersection( p0, p1, O_circle, N_circle,
            segment_plane_result ) ) {
            if( ( segment_plane_result - O_circle ).length() <= r ) {
                result = segment_plane_result ;
                return true ;
            }
        }
        return false ;
    }

    /*!
     * Computes the intersection(s) between a circle and a triangle
     * @param[in] p0 the first vertex of the triangle
     * @param[in] p1 the second vertex of the triangle
     * @param[in] p2 the third vertex of the triangle
     * @param[in] O_circle the center of the circle
     * @param[in] N_circle the normal of the plane supporting the circle
     * @param[in] r the radius of the circle
     * @param[out] result the intersected points
     * @return returns true if there is at least one intersection
     */
    bool circle_triangle_intersection(
        const vec3& p0,
        const vec3& p1,
        const vec3& p2,
        const vec3& O_circle,
        const vec3& N_circle,
        double r,
        std::vector< vec3 >& result )
    {
        vec3 N_triangle = normalize( cross( p1 - p0, p2 - p0 ) ) ;
        vec3 barycenter = ( p0 + p1 + p2 ) / 3 ;
        std::vector< vec3 > inter_circle_plane ;
        if( circle_plane_intersection( barycenter, N_triangle, O_circle, N_circle, r,
            inter_circle_plane ) ) {
            for( index_t i = 0; i < inter_circle_plane.size(); i++ ) {
                const vec3& p = inter_circle_plane[i] ;
                if( point_inside_triangle( p, p0, p1, p2 ) ) {
                    result.push_back( p ) ;
                }
            }
        }
        return !result.empty() ;
    }

    /*!
     * Computes the orthogonal projection of a point on a segment
     * @param[in] p the point to project
     * @param[in] p0 the first vertex of the segment
     * @param[in] p1 the second vertex of the segment
     * @param[out] new_p the projected point
     * @return returns true if the projection is possible
     */
    bool point_segment_projection(
        const vec3& p,
        const vec3& p0,
        const vec3& p1,
        vec3& new_p )
    {
        vec3 center = ( p0 + p1 ) * 0.5 ;
        vec3 diff = p - center ;
        vec3 edge = p1 - p0 ;
        double extent = 0.5 * edge.length() ;
        edge = normalize( edge ) ;
        double d = dot( edge, diff ) ;

        if( std::fabs( d ) <= extent ) {
            new_p = center + d * edge ;
            return true ;
        }
        return false ;
    }

    /*!
     * Computes the orthogonal projection of a point on a plane
     * @param[in] p the point to project
     * @param[in] N_plane the normal of the plane
     * @param[in] O_plane the origin of the plane
     * @param[out] projected_p the projected point
     */
    void point_plane_projection(
        const vec3& p,
        const vec3& N_plane,
        const vec3& O_plane,
        vec3& projected_p )
    {
        vec3 N_unit_plane = normalize( N_plane ) ;
        vec3 v( p - O_plane ) ;
        double distance = dot( v, N_unit_plane ) ;
        projected_p = p - distance * N_unit_plane ;
    }

    double point_segment_distance(
        const vec3& p,
        const vec3& p0,
        const vec3& p1,
        vec3& nearest_p )
    {
        if( point_segment_projection( p, p0, p1, nearest_p ) ) {
            return length( nearest_p - p ) ;
        } else {
            double p0_distance_sq = length2( p0 - p ) ;
            double p1_distance_sq = length2( p1 - p ) ;
            if( p0_distance_sq < p1_distance_sq ) {
                nearest_p = p0 ;
                return std::sqrt( p0_distance_sq ) ;
            } else {
                nearest_p = p1 ;
                return std::sqrt( p1_distance_sq ) ;
            }
        }
    }

    /*!
     * Computes the smallest distance between a point and a quad
     * @param[in] p the point to test
     * @param[in] p0 the first vertex of the quad
     * @param[in] p1 the second vertex of the quad
     * @param[in] p2 the third vertex of the quad
     * @param[in] p3 the fourth vertex of the quad
     * @param[out] nearest_p the closest point on the quad
     * @return the smallest distance
     */
    double point_quad_distance(
        const vec3& p,
        const vec3& p0,
        const vec3& p1,
        const vec3& p2,
        const vec3& p3,
        vec3& nearest_p )
    {
        const vec3 center( ( p0 + p1 + p2 + p3 ) * 0.25 ) ;
        vec3 edge0( p1 - p0 ) ;
        vec3 edge1( p3 - p0 ) ;
        vec3 axis[2] = { normalize( edge0 ), normalize( edge1 ) } ;
        double extent[2] = { 0.5 * edge0.length(), 0.5 * edge1.length() } ;

        vec3 diff = center - p ;
        double b0 = dot( diff, axis[0] ) ;
        double b1 = dot( diff, axis[1] ) ;
        double s0 = -b0 ;
        double s1 = -b1 ;
        double sqrDistance = dot( diff, diff ) ;

        if( s0 < -extent[0] ) {
            s0 = -extent[0] ;
        } else if( s0 > extent[0] ) {
            s0 = extent[0] ;
        }
        sqrDistance += s0 * ( s0 + 2. * b0 ) ;

        if( s1 < -extent[1] ) {
            s1 = -extent[1] ;
        } else if( s1 > extent[1] ) {
            s1 = extent[1] ;
        }
        sqrDistance += s1 * ( s1 + 2. * b1 ) ;

        // Account for numerical round-off error.
        if( sqrDistance < 0 ) {
            sqrDistance = 0 ;
        }

        double distance = std::sqrt( sqrDistance ) ;
        nearest_p = center ;
        nearest_p += s0 * axis[0] ;
        nearest_p += s1 * axis[1] ;

        return distance ;
    }

    /*!
     * Computes the intersection of a segment and a triangle
     * @param[in] seg0 the first vertex of the segment
     * @param[in] seg1 the second vertex of the segment
     * @param[in] trgl0 the first vertex of the triangle
     * @param[in] trgl1 the second vertex of the triangle
     * @param[in] trgl2 the third vertex of the triangle
     * @param[out] result the intersected point
     * @return true is there is an intersection
     */
    bool segment_triangle_intersection(
        const vec3& seg0,
        const vec3& seg1,
        const vec3& trgl0,
        const vec3& trgl1,
        const vec3& trgl2,
        vec3& result )
    {
        // http://www.geometrictools.com/LibMathematics/Intersection/Intersection.html
        // Compute the offset origin, edges, and normal.
        vec3 seg_center = ( seg0 + seg1 ) / 2 ;
        vec3 diff = seg_center - trgl0 ;
        vec3 edge1 = trgl1 - trgl0 ;
        vec3 edge2 = trgl2 - trgl0 ;
        vec3 normal = cross( edge1, edge2 ) ;

        // Solve Q + t*D = b1*E1 + b2*E2 (Q = diff, D = segment direction,
        // E1 = edge1, E2 = edge2, N = Cross(E1,E2)) by
        //   |Dot(D,N)|*b1 = sign(Dot(D,N))*Dot(D,Cross(Q,E2))
        //   |Dot(D,N)|*b2 = sign(Dot(D,N))*Dot(D,Cross(E1,Q))
        //   |Dot(D,N)|*t = -sign(Dot(D,N))*Dot(Q,N)
        vec3 D = normalize( seg1 - seg0 ) ;
        double DdN = dot( D, normal ) ;
        signed_index_t sign ;
        if( DdN > global_epsilon ) {
            sign = 1 ;
        } else if( DdN < -global_epsilon ) {
            sign = -1 ;
            DdN = -DdN ;
        } else {
            // Segment and triangle are parallel, call it a "no intersection"
            // even if the segment does intersect.
            return false ;
        }

        double DdQxE2 = sign * dot( D, cross( diff, edge2 ) ) ;
        if( DdQxE2 >= 0 ) {
            double DdE1xQ = sign * dot( D, cross( edge1, diff ) ) ;
            if( DdE1xQ >= 0 ) {
                if( DdQxE2 + DdE1xQ <= DdN ) {
                    // Line intersects triangle, check if segment does.
                    double QdN = -sign * dot( diff, normal ) ;
                    double extDdN = length( seg1 - seg0 ) * DdN / 2. ;
                    if( -extDdN <= QdN && QdN <= extDdN ) {
                        // Segment intersects triangle.
                        double inv = 1. / DdN ;
                        double seg_parameter = QdN * inv ;

                        result = seg_center + seg_parameter * D ;
                        return true ;
                    }
                    // else: |t| > extent, no intersection
                }
                // else: b1+b2 > 1, no intersection
            }
            // else: b2 < 0, no intersection
        }
        // else: b1 < 0, no intersection
        return false ;
    }

    /*!
     * @brief Builds a rotational matrix about an arbitrary axis.
     *
     * Mathematical development: http://paulbourke.net/geometry/rotate/.
     *
     * @param[in] origin point in which passes the rotation axis.
     *
     * @param[in] axis vector which defines the rotation axis.
     *
     * @param[in] theta rotation angle (in radians or degrees).
     *
     * @param[in] degrees true is \p theta is in degrees, false
     * if in radians.
     *
     * @param[out] rot_mat the matrix which defines the rotation
     * of a point around the axis defined by point \p origin
     * and vector \p axis by an angle \p theta.
     * New coordinates of a point (x,y,z) are:
     * (x',y',z') = rot_mat*(x,y,z)
     */
    void rotation_matrix_about_arbitrary_axis(
        const vec3& origin,
        const vec3& axis,
        double theta,
        bool degrees,
        GEO::Matrix< 4, double >& rot_mat )
    {
        // Note: Rotation is impossible about an axis with null length.
        ringmesh_assert( axis != vec3() ) ;

        if( degrees ) {
            double pi = 3.141592653589793 ;
            theta = theta * pi / 180. ;
        }

        double axis_length = axis.length() ;
        ringmesh_assert( axis_length > 0. ) ;
        double x1 = origin[0] ;
        double y1 = origin[1] ;
        double z1 = origin[2] ;
        double a = axis[0] / axis_length ;
        double b = axis[1] / axis_length ;
        double c = axis[2] / axis_length ;
        double d = std::sqrt( b * b + c * c ) ;
        double cos_angle = std::cos( theta ) ;
        double sin_angle = std::sin( theta ) ;

        GEO::Matrix< 4, double > T ;
        T( 0, 0 ) = 1 ;
        T( 0, 1 ) = 0 ;
        T( 0, 2 ) = 0 ;
        T( 0, 3 ) = -x1 ;
        T( 1, 0 ) = 0 ;
        T( 1, 1 ) = 1 ;
        T( 1, 2 ) = 0 ;
        T( 1, 3 ) = -y1 ;
        T( 2, 0 ) = 0 ;
        T( 2, 1 ) = 0 ;
        T( 2, 2 ) = 1 ;
        T( 2, 3 ) = -z1 ;
        T( 3, 0 ) = 0 ;
        T( 3, 1 ) = 0 ;
        T( 3, 2 ) = 0 ;
        T( 3, 3 ) = 1 ;

        GEO::Matrix< 4, double > inv_T ;
        inv_T( 0, 0 ) = 1. ;
        inv_T( 0, 1 ) = 0. ;
        inv_T( 0, 2 ) = 0. ;
        inv_T( 0, 3 ) = x1 ;
        inv_T( 1, 0 ) = 0. ;
        inv_T( 1, 1 ) = 1. ;
        inv_T( 1, 2 ) = 0. ;
        inv_T( 1, 3 ) = y1 ;
        inv_T( 2, 0 ) = 0. ;
        inv_T( 2, 1 ) = 0. ;
        inv_T( 2, 2 ) = 1. ;
        inv_T( 2, 3 ) = z1 ;
        inv_T( 3, 0 ) = 0. ;
        inv_T( 3, 1 ) = 0. ;
        inv_T( 3, 2 ) = 0. ;
        inv_T( 3, 3 ) = 1. ;

#ifdef RINGMESH_DEBUG
        GEO::Matrix< 4, double > computed_inv_T = T.inverse() ;
#endif
        ringmesh_assert( inv_T( 0, 0 ) == computed_inv_T( 0, 0 ) ) ;
        ringmesh_assert( inv_T( 0, 1 ) == computed_inv_T( 0, 1 ) ) ;
        ringmesh_assert( inv_T( 0, 2 ) == computed_inv_T( 0, 2 ) ) ;
        ringmesh_assert( inv_T( 0, 3 ) == computed_inv_T( 0, 3 ) ) ;
        ringmesh_assert( inv_T( 1, 0 ) == computed_inv_T( 1, 0 ) ) ;
        ringmesh_assert( inv_T( 1, 1 ) == computed_inv_T( 1, 1 ) ) ;
        ringmesh_assert( inv_T( 1, 2 ) == computed_inv_T( 1, 2 ) ) ;
        ringmesh_assert( inv_T( 1, 3 ) == computed_inv_T( 1, 3 ) ) ;
        ringmesh_assert( inv_T( 2, 0 ) == computed_inv_T( 2, 0 ) ) ;
        ringmesh_assert( inv_T( 2, 1 ) == computed_inv_T( 2, 1 ) ) ;
        ringmesh_assert( inv_T( 2, 2 ) == computed_inv_T( 2, 2 ) ) ;
        ringmesh_assert( inv_T( 2, 3 ) == computed_inv_T( 2, 3 ) ) ;
        ringmesh_assert( inv_T( 3, 0 ) == computed_inv_T( 3, 0 ) ) ;
        ringmesh_assert( inv_T( 3, 1 ) == computed_inv_T( 3, 1 ) ) ;
        ringmesh_assert( inv_T( 3, 2 ) == computed_inv_T( 3, 2 ) ) ;
        ringmesh_assert( inv_T( 3, 3 ) == computed_inv_T( 3, 3 ) ) ;

        // Note: If d = 0, so rotation is along x axis. So Rx = inv_Rx = Id
        GEO::Matrix< 4, double > Rx ;
        Rx( 0, 0 ) = 1. ;
        Rx( 0, 1 ) = 0. ;
        Rx( 0, 2 ) = 0. ;
        Rx( 0, 3 ) = 0. ;
        Rx( 1, 0 ) = 0. ;
        Rx( 1, 3 ) = 0. ;
        Rx( 2, 0 ) = 0. ;
        Rx( 2, 3 ) = 0. ;
        Rx( 3, 0 ) = 0. ;
        Rx( 3, 1 ) = 0. ;
        Rx( 3, 2 ) = 0. ;
        Rx( 3, 3 ) = 1. ;
        if( d == 0. ) {
            Rx( 1, 1 ) = 1. ;
            Rx( 1, 2 ) = 0. ;
            Rx( 2, 1 ) = 0. ;
            Rx( 2, 2 ) = 1. ;
        } else {
            Rx( 1, 1 ) = c / d ;
            Rx( 1, 2 ) = -b / d ;
            Rx( 2, 1 ) = b / d ;
            Rx( 2, 2 ) = c / d ;
        }

        GEO::Matrix< 4, double > inv_Rx ;
        inv_Rx( 0, 0 ) = 1. ;
        inv_Rx( 0, 1 ) = 0. ;
        inv_Rx( 0, 2 ) = 0. ;
        inv_Rx( 0, 3 ) = 0. ;
        inv_Rx( 1, 0 ) = 0. ;
        inv_Rx( 1, 3 ) = 0. ;
        inv_Rx( 2, 0 ) = 0. ;
        inv_Rx( 2, 3 ) = 0. ;
        inv_Rx( 3, 0 ) = 0. ;
        inv_Rx( 3, 1 ) = 0. ;
        inv_Rx( 3, 2 ) = 0. ;
        inv_Rx( 3, 3 ) = 1. ;
        if( d == 0. ) {
            inv_Rx( 1, 1 ) = 1. ;
            inv_Rx( 1, 2 ) = 0. ;
            inv_Rx( 2, 1 ) = 0. ;
            inv_Rx( 2, 2 ) = 1. ;
        } else {
            inv_Rx( 1, 1 ) = c / d ;
            inv_Rx( 1, 2 ) = b / d ;
            inv_Rx( 2, 1 ) = -b / d ;
            inv_Rx( 2, 2 ) = c / d ;
        }

#ifdef RINGMESH_DEBUG
        GEO::Matrix< 4, double > computed_inv_Rx = Rx.inverse() ;
#endif
        ringmesh_assert( inv_Rx( 0, 0 ) == computed_inv_Rx( 0, 0 ) ) ;
        ringmesh_assert( inv_Rx( 0, 1 ) == computed_inv_Rx( 0, 1 ) ) ;
        ringmesh_assert( inv_Rx( 0, 2 ) == computed_inv_Rx( 0, 2 ) ) ;
        ringmesh_assert( inv_Rx( 0, 3 ) == computed_inv_Rx( 0, 3 ) ) ;
        ringmesh_assert( inv_Rx( 1, 0 ) == computed_inv_Rx( 1, 0 ) ) ;
        ringmesh_assert( inv_Rx( 1, 1 ) == computed_inv_Rx( 1, 1 ) ) ;
        ringmesh_assert( inv_Rx( 1, 2 ) == computed_inv_Rx( 1, 2 ) ) ;
        ringmesh_assert( inv_Rx( 1, 3 ) == computed_inv_Rx( 1, 3 ) ) ;
        ringmesh_assert( inv_Rx( 2, 0 ) == computed_inv_Rx( 2, 0 ) ) ;
        ringmesh_assert( inv_Rx( 2, 1 ) == computed_inv_Rx( 2, 1 ) ) ;
        ringmesh_assert( inv_Rx( 2, 2 ) == computed_inv_Rx( 2, 2 ) ) ;
        ringmesh_assert( inv_Rx( 2, 3 ) == computed_inv_Rx( 2, 3 ) ) ;
        ringmesh_assert( inv_Rx( 3, 0 ) == computed_inv_Rx( 3, 0 ) ) ;
        ringmesh_assert( inv_Rx( 3, 1 ) == computed_inv_Rx( 3, 1 ) ) ;
        ringmesh_assert( inv_Rx( 3, 2 ) == computed_inv_Rx( 3, 2 ) ) ;
        ringmesh_assert( inv_Rx( 3, 3 ) == computed_inv_Rx( 3, 3 ) ) ;

        GEO::Matrix< 4, double > Ry ;
        Ry( 0, 0 ) = d ;
        Ry( 0, 1 ) = 0. ;
        Ry( 0, 2 ) = -a ;
        Ry( 0, 3 ) = 0. ;
        Ry( 1, 0 ) = 0. ;
        Ry( 1, 1 ) = 1. ;
        Ry( 1, 2 ) = 0. ;
        Ry( 1, 3 ) = 0. ;
        Ry( 2, 0 ) = a ;
        Ry( 2, 1 ) = 0. ;
        Ry( 2, 2 ) = d ;
        Ry( 2, 3 ) = 0. ;
        Ry( 3, 0 ) = 0. ;
        Ry( 3, 1 ) = 0. ;
        Ry( 3, 2 ) = 0. ;
        Ry( 3, 3 ) = 1. ;

        GEO::Matrix< 4, double > inv_Ry ;
        inv_Ry( 0, 0 ) = d ;
        inv_Ry( 0, 1 ) = 0. ;
        inv_Ry( 0, 2 ) = a ;
        inv_Ry( 0, 3 ) = 0. ;
        inv_Ry( 1, 0 ) = 0. ;
        inv_Ry( 1, 1 ) = 1. ;
        inv_Ry( 1, 2 ) = 0. ;
        inv_Ry( 1, 3 ) = 0. ;
        inv_Ry( 2, 0 ) = -a ;
        inv_Ry( 2, 1 ) = 0. ;
        inv_Ry( 2, 2 ) = d ;
        inv_Ry( 2, 3 ) = 0. ;
        inv_Ry( 3, 0 ) = 0. ;
        inv_Ry( 3, 1 ) = 0. ;
        inv_Ry( 3, 2 ) = 0. ;
        inv_Ry( 3, 3 ) = 1. ;

#ifdef RINGMESH_DEBUG
        GEO::Matrix< 4, double > computed_inv_Ry = Ry.inverse() ;
#endif
        ringmesh_assert( inv_Ry( 0, 0 ) == computed_inv_Ry( 0, 0 ) ) ;
        ringmesh_assert( inv_Ry( 0, 1 ) == computed_inv_Ry( 0, 1 ) ) ;
        ringmesh_assert( inv_Ry( 0, 2 ) == computed_inv_Ry( 0, 2 ) ) ;
        ringmesh_assert( inv_Ry( 0, 3 ) == computed_inv_Ry( 0, 3 ) ) ;
        ringmesh_assert( inv_Ry( 1, 0 ) == computed_inv_Ry( 1, 0 ) ) ;
        ringmesh_assert( inv_Ry( 1, 1 ) == computed_inv_Ry( 1, 1 ) ) ;
        ringmesh_assert( inv_Ry( 1, 2 ) == computed_inv_Ry( 1, 2 ) ) ;
        ringmesh_assert( inv_Ry( 1, 3 ) == computed_inv_Ry( 1, 3 ) ) ;
        ringmesh_assert( inv_Ry( 2, 0 ) == computed_inv_Ry( 2, 0 ) ) ;
        ringmesh_assert( inv_Ry( 2, 1 ) == computed_inv_Ry( 2, 1 ) ) ;
        ringmesh_assert( inv_Ry( 2, 2 ) == computed_inv_Ry( 2, 2 ) ) ;
        ringmesh_assert( inv_Ry( 2, 3 ) == computed_inv_Ry( 2, 3 ) ) ;
        ringmesh_assert( inv_Ry( 3, 0 ) == computed_inv_Ry( 3, 0 ) ) ;
        ringmesh_assert( inv_Ry( 3, 1 ) == computed_inv_Ry( 3, 1 ) ) ;
        ringmesh_assert( inv_Ry( 3, 2 ) == computed_inv_Ry( 3, 2 ) ) ;
        ringmesh_assert( inv_Ry( 3, 3 ) == computed_inv_Ry( 3, 3 ) ) ;

        GEO::Matrix< 4, double > Rz ;
        Rz( 0, 0 ) = cos_angle ;
        Rz( 0, 1 ) = -sin_angle ;
        Rz( 0, 2 ) = 0. ;
        Rz( 0, 3 ) = 0. ;
        Rz( 1, 0 ) = sin_angle ;
        Rz( 1, 1 ) = cos_angle ;
        Rz( 1, 2 ) = 0. ;
        Rz( 1, 3 ) = 0. ;
        Rz( 2, 0 ) = 0. ;
        Rz( 2, 1 ) = 0. ;
        Rz( 2, 2 ) = 1. ;
        Rz( 2, 3 ) = 0. ;
        Rz( 3, 0 ) = 0. ;
        Rz( 3, 1 ) = 0. ;
        Rz( 3, 2 ) = 0. ;
        Rz( 3, 3 ) = 1. ;

        rot_mat = inv_T * inv_Rx * inv_Ry * Rz * Ry * Rx * T ;
    }

    /*!
     * Tests if a point is inside a triangle, more precisely if it is inside
     * a prism based on the triangle and its normal
     * @param[in] p the point to test
     * @param[in] p0 the first vertex of the triangle
     * @param[in] p1 the second vertex of the triangle
     * @param[in] p2 the third vertex of the triangle
     * @param[in] exact_predicates if true, the algorithm uses exact predicates
     * @return returns true if the point is inside
     */
    bool point_inside_triangle(
        const vec3& p,
        const vec3& p0,
        const vec3& p1,
        const vec3& p2,
        bool exact_predicates )
    {
        vec3 n = cross( p2 - p0, p1 - p0 ) ;
        vec3 q = p + n ;

        Sign s1, s2, s3 ;
        if( !exact_predicates ) {
            double vol1 = GEO::Geom::tetra_signed_volume( p, q, p0, p1 ) ;
            if( is_almost_zero( vol1 ) ) {
                return point_inside_triangle( p, p0, p1, p2, true ) ;
            }
            s1 = sign( vol1 ) ;
            double vol2 = GEO::Geom::tetra_signed_volume( p, q, p1, p2 ) ;
            if( is_almost_zero( vol2 ) ) {
                return point_inside_triangle( p, p0, p1, p2, true ) ;
            }
            s2 = sign( vol2 ) ;
            double vol3 = GEO::Geom::tetra_signed_volume( p, q, p2, p0 ) ;
            if( is_almost_zero( vol3 ) ) {
                return point_inside_triangle( p, p0, p1, p2, true ) ;
            }
            s3 = sign( vol3 ) ;
        } else {
            s1 = sign(
                GEO::PCK::orient_3d( p.data(), q.data(), p0.data(), p1.data() ) ) ;
            s2 = sign(
                GEO::PCK::orient_3d( p.data(), q.data(), p1.data(), p2.data() ) ) ;
            s3 = sign(
                GEO::PCK::orient_3d( p.data(), q.data(), p2.data(), p0.data() ) ) ;
            if( s1 == ZERO ) {
                return s2 == s3 ;
            } else if( s2 == ZERO ) {
                return s1 == s3 ;
            } else if( s3 == ZERO ) {
                return s1 == s2 ;
            }
        }

        return s1 == s2 && s2 == s3 ;
    }

    /*!
     * Tests if a point is inside a quad, more precisely if it is inside the box
     * based on the quad and its normal
     * @param[in] p the point to test
     * @param[in] p0 the first vertex of the quad
     * @param[in] p1 the second vertex of the quad
     * @param[in] p2 the third vertex of the quad
     * @param[in] p3 the fourth vertex of the quad
     * @param[in] exact_predicates if true, the algorithm uses exact predicates
     * @return returns true if the point is inside
     */
    bool point_inside_quad(
        const vec3& p,
        const vec3& p0,
        const vec3& p1,
        const vec3& p2,
        const vec3& p3,
        bool exact_predicates )
    {
        vec3 n = cross( p2 - p0, p1 - p0 ) ;
        vec3 q = p + n ;
        Sign s1, s2, s3, s4 ;
        if( !exact_predicates ) {
            double vol1 = GEO::Geom::tetra_signed_volume( p, q, p0, p1 ) ;
            if( is_almost_zero( vol1 ) ) {
                return point_inside_quad( p, p0, p1, p2, p3, true ) ;
            }
            s1 = sign( vol1 ) ;
            double vol2 = GEO::Geom::tetra_signed_volume( p, q, p1, p2 ) ;
            if( is_almost_zero( vol2 ) ) {
                return point_inside_quad( p, p0, p1, p2, p3, true ) ;
            }
            s2 = sign( vol2 ) ;
            double vol3 = GEO::Geom::tetra_signed_volume( p, q, p2, p3 ) ;
            if( is_almost_zero( vol3 ) ) {
                return point_inside_quad( p, p0, p1, p2, p3, true ) ;
            }
            s3 = sign( vol3 ) ;
            double vol4 = GEO::Geom::tetra_signed_volume( p, q, p3, p0 ) ;
            if( is_almost_zero( vol4 ) ) {
                return point_inside_quad( p, p0, p1, p2, p3, true ) ;
            }
            s4 = sign( vol4 ) ;
        } else {
            s1 = sign(
                GEO::PCK::orient_3d( p.data(), q.data(), p0.data(), p1.data() ) ) ;
            s2 = sign(
                GEO::PCK::orient_3d( p.data(), q.data(), p1.data(), p2.data() ) ) ;
            s3 = sign(
                GEO::PCK::orient_3d( p.data(), q.data(), p2.data(), p3.data() ) ) ;
            s4 = sign(
                GEO::PCK::orient_3d( p.data(), q.data(), p3.data(), p0.data() ) ) ;
            if( s1 == ZERO ) {
                return s2 == s3 && s3 == s4 ;
            } else if( s2 == ZERO ) {
                return s1 == s3 && s3 == s4 ;
            } else if( s3 == ZERO ) {
                return s1 == s2 && s2 == s4 ;
            } else if( s4 == ZERO ) {
                return s1 == s2 && s2 == s3 ;
            }
        }
        return s1 == s2 && s2 == s3 && s3 == s4 ;
    }

    NNSearch::NNSearch(
        const GEO::Mesh& mesh,
        const MeshLocation& location,
        bool copy )
        : nn_points_( nil ), delete_points_( true )
    {
        nn_tree_ = GEO::NearestNeighborSearch::create( 3, "BNN" ) ;
        switch( location ) {
            case VERTICES: {
                build_nn_search_vertices( mesh, copy ) ;
                break ;
            }
            case EDGES: {
                build_nn_search_edges( mesh ) ;
                break ;
            }
            case FACETS: {
                build_nn_search_facets( mesh ) ;
                break ;
            }
            case CELLS: {
                build_nn_search_cells( mesh ) ;
                break ;
            }
            case CELL_FACETS: {
                build_nn_search_cell_facets( mesh ) ;
                break ;
            }
            default:
                ringmesh_assert_not_reached ;
                break ;
        }
    }

    NNSearch::NNSearch( const std::vector< vec3 >& vertices, bool copy )
    {
        index_t nb_vertices = static_cast< index_t >( vertices.size() ) ;
        nn_tree_ = GEO::NearestNeighborSearch::create( 3, "BNN" ) ;
        if( copy ) {
            nn_points_ = new double[nb_vertices * 3] ;
            delete_points_ = true ;
            GEO::Memory::copy( nn_points_, vertices.data()->data(),
                3 * nb_vertices * sizeof(double) ) ;
        } else {
            nn_points_ = const_cast< double* >( vertices.data()->data() ) ;
            delete_points_ = false ;
        }
        nn_tree_->set_points( nb_vertices, nn_points_ ) ;
    }

    index_t NNSearch::get_colocated_index_mapping(
        double epsilon,
        GEO::vector< index_t >& index_map ) const
    {
        index_map.resize( nn_tree_->nb_points() ) ;
        for( index_t i = 0; i < index_map.size(); i++ ) {
            index_map[i] = i ;
        }
        std::vector< index_t > nb_colocalised_per_thread( omp_get_max_threads(),
            0 ) ;
        RINGMESH_PARALLEL_LOOP
        for( index_t i = 0; i < index_map.size(); i++ ) {
            std::vector< index_t > results ;
            vec3 query( nn_points_[3 * i], nn_points_[3 * i + 1],
                nn_points_[3 * i + 2] ) ;
            get_neighbors( query, results, epsilon ) ;
            index_t id = *std::min_element( results.begin(), results.end() ) ;
            if( id < i ) {
                index_map[i] = id ;
                index_t thread_id = static_cast< index_t >( omp_get_thread_num() ) ;
                nb_colocalised_per_thread[thread_id]++ ;
            }
        }

        index_t nb_colocalised_vertices = 0 ;
        for( index_t i = 0; i < nb_colocalised_per_thread.size(); i++ ) {
            nb_colocalised_vertices += nb_colocalised_per_thread[i] ;
        }
        return nb_colocalised_vertices ;
    }

    index_t NNSearch::get_colocated_index_mapping(
        double epsilon,
        GEO::vector< index_t >& index_map,
        GEO::vector< vec3 >& unique_points ) const
    {
        index_t nb_colocalised_vertices = get_colocated_index_mapping( epsilon,
            index_map ) ;
        unique_points.reserve( nb_points() - nb_colocalised_vertices ) ;
        index_t offset = 0 ;
        for( index_t p = 0; p < index_map.size(); p++ ) {
            if( index_map[p] == p ) {
                vec3 new_point( nn_points_[3 * p], nn_points_[3 * p + 1],
                    nn_points_[3 * p + 2] ) ;
                unique_points.push_back( new_point ) ;
                index_map[p] = p - offset ;
            } else {
                offset++ ;
                index_map[p] = index_map[index_map[p]] ;
            }
        }
        ringmesh_assert( offset == nb_colocalised_vertices ) ;
        return offset ;
    }

    /*!
     * Compute the neighbors of a given point, point closer than \param threshold_distance
     * @param[in] v the point to test
     * @param[out] result the neighbor point indices.
     * @param[in] threshold_distance distance defining the neighborhood
     * @return return true if there is at least one neighbor
     */
    bool NNSearch::get_neighbors(
        const vec3& v,
        std::vector< index_t >& result,
        double threshold_distance ) const
    {

        index_t nb_points = nn_tree_->nb_points() ;
        result.clear() ;
        if( nb_points == 0 ) {
            return false ;
        }
        double threshold_distance_sq = threshold_distance * threshold_distance ;
        index_t nb_neighbors = std::min( index_t( 5 ), nb_points ) ;
        std::vector< index_t > neighbors ;
        index_t cur_neighbor = 0 ;
        index_t prev_neighbor = 0 ;
        do {
            prev_neighbor = cur_neighbor ;
            cur_neighbor += nb_neighbors ;
            neighbors.resize( cur_neighbor ) ;
            double* distance_sq = (double*) alloca( sizeof(double) * cur_neighbor ) ;
            nb_neighbors = get_neighbors( v, cur_neighbor, neighbors, distance_sq ) ;
            for( index_t i = prev_neighbor; i < cur_neighbor; ++i ) {
                if( distance_sq[i] > threshold_distance_sq ) {
                    break ;
                }
                result.push_back( neighbors[i] ) ;
            }
        } while( result.size() == cur_neighbor && result.size() < nb_points ) ;

        return !result.empty() ;

    }

    /*!
     * Gets the neighboring points of a given one sorted by increasing distance
     * @param[in] v the point to test
     * @param[in] nb_neighbors the number of neighbors to return
     * @param[out] result the neighboring points
     * @param[out] dist the square distance between each neigbhor and the point \p v
     * @return the number of neighbors returned (can be less than \p nb_neighbors
     * if there is not enough points)
     */
    index_t NNSearch::get_neighbors(
        const vec3& v,
        index_t nb_neighbors,
        std::vector< index_t >& result,
        double* dist ) const
    {
        if( nn_tree_->nb_points() == 0 ) {
            return 0 ;
        }
        if( !dist ) {
            dist = (double*) alloca( sizeof(double) * nb_neighbors ) ;
        }
        nb_neighbors = std::min( nb_neighbors, nn_tree_->nb_points() ) ;
        result.resize( nb_neighbors ) ;
        nn_tree_->get_nearest_neighbors( nb_neighbors, v.data(), &result[0],
            dist ) ;
        return nb_neighbors ;
    }

    void NNSearch::build_nn_search_vertices( const GEO::Mesh& mesh, bool copy )
    {
        const GEO::MeshVertices& mesh_vertices = mesh.vertices ;
        index_t nb_vertices = mesh_vertices.nb() ;
        if( nb_vertices == 0 ) {
            return ;
        }
        if( !copy ) {
            nn_points_ = const_cast< double* >( mesh_vertices.point_ptr( 0 ) ) ;
            delete_points_ = false ;
        } else {
            nn_points_ = new double[nb_vertices * 3] ;
            GEO::Memory::copy( nn_points_, mesh_vertices.point_ptr( 0 ),
                nb_vertices * 3 * sizeof(double) ) ;
        }
        nn_tree_->set_points( nb_vertices, nn_points_ ) ;
    }

    void NNSearch::build_nn_search_edges( const GEO::Mesh& mesh )
    {
        const GEO::MeshEdges& mesh_edges = mesh.edges ;
        index_t nb_edges = mesh_edges.nb() ;
        if( nb_edges == 0 ) {
            return ;
        }
        nn_points_ = new double[nb_edges * 3] ;
        for( index_t i = 0; i < nb_edges; i++ ) {
            index_t first_vertex_id = mesh_edges.vertex( i, 0 ) ;
            const vec3& first_vertex_vec = mesh.vertices.point( first_vertex_id ) ;
            index_t second_vertex_id = mesh.edges.vertex( i, 1 ) ;
            const vec3& second_vertex_vec = mesh.vertices.point( second_vertex_id ) ;

            vec3 center = ( first_vertex_vec + second_vertex_vec ) / 2. ;
            index_t index_in_nn_search = 3 * i ;
            fill_nn_search_points( index_in_nn_search, center ) ;
        }
        nn_tree_->set_points( nb_edges, nn_points_ ) ;
    }

    void NNSearch::build_nn_search_facets( const GEO::Mesh& mesh )
    {
        index_t nb_facets = mesh.facets.nb() ;
        if( nb_facets == 0 ) {
            return ;
        }
        nn_points_ = new double[nb_facets * 3] ;
        for( index_t i = 0; i < nb_facets; i++ ) {
            vec3 center = GEO::Geom::mesh_facet_center( mesh, i ) ;
            index_t index_in_nn_search = 3 * i ;
            fill_nn_search_points( index_in_nn_search, center ) ;
        }
        nn_tree_->set_points( nb_facets, nn_points_ ) ;
    }

    void NNSearch::build_nn_search_cell_facets( const GEO::Mesh& mesh )
    {
        index_t nb_cell_facets = mesh.cell_facets.nb() ;
        nn_points_ = new double[nb_cell_facets * 3] ;
        index_t index_in_nn_search = 0 ;
        for( index_t c = 0; c < mesh.cells.nb(); c++ ) {
            for( index_t f = 0; f < mesh.cells.nb_facets( c ); f++ ) {
                vec3 center = mesh_cell_facet_barycenter( mesh, c, f ) ;
                fill_nn_search_points( index_in_nn_search, center ) ;
                index_in_nn_search += 3 ;
            }
        }
        nn_tree_->set_points( nb_cell_facets, nn_points_ ) ;
    }

    void NNSearch::build_nn_search_cells( const GEO::Mesh& mesh )
    {
        index_t nb_cells = mesh.cells.nb() ;
        if( nb_cells == 0 ) {
            return ;
        }
        nn_points_ = new double[nb_cells * 3] ;
        for( index_t i = 0; i < nb_cells; i++ ) {
            vec3 center = mesh_cell_barycenter( mesh, i ) ;
            index_t index_in_nn_search = 3 * i ;
            fill_nn_search_points( index_in_nn_search, center ) ;
        }
        nn_tree_->set_points( nb_cells, nn_points_ ) ;
    }

    void NNSearch::fill_nn_search_points( index_t index_in_nn_search, const vec3& center )
    {
        nn_points_[index_in_nn_search] = center.x ;
        nn_points_[index_in_nn_search + 1] = center.y ;
        nn_points_[index_in_nn_search + 2] = center.z ;
    }
}
