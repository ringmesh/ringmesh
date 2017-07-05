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

#include <ringmesh/basic/geometry.h>

#include <geogram/mesh/mesh.h>

/*!
 * @file Basic geometrical requests 
 * @author Arnaud Botella
 *
 * @todo Comment on the robustness of the tests
 */

namespace RINGMesh {
    namespace Distance {

        double point_to_triangle(
            const vec3& point,
            const vec3& V0,
            const vec3& V1,
            const vec3& V2,
            vec3& closest_point )
        {
            vec3 diff = V0 - point;
            vec3 edge0 = V1 - V0;
            vec3 edge1 = V2 - V0;
            double a00 = length2( edge0 );
            double a01 = dot( edge0, edge1 );
            double a11 = length2( edge1 );
            double b0 = dot( diff, edge0 );
            double b1 = dot( diff, edge1 );
            double c = length2( diff );
            double det = std::fabs( a00 * a11 - a01 * a01 );
            double s = a01 * b1 - a11 * b0;
            double t = a01 * b0 - a00 * b1;
            double sqrDistance;

            if( s + t <= det ) {
                if( s < 0.0 ) {
                    if( t < 0.0 ) { // region 4
                        if( b0 < 0.0 ) {
                            t = 0.0;
                            if( -b0 >= a00 ) {
                                s = 1.0;
                                sqrDistance = a00 + 2.0 * b0 + c;
                            } else {
                                s = -b0 / a00;
                                sqrDistance = b0 * s + c;
                            }
                        } else {
                            s = 0.0;
                            if( b1 >= 0.0 ) {
                                t = 0.0;
                                sqrDistance = c;
                            } else if( -b1 >= a11 ) {
                                t = 1.0;
                                sqrDistance = a11 + 2.0 * b1 + c;
                            } else {
                                t = -b1 / a11;
                                sqrDistance = b1 * t + c;
                            }
                        }
                    } else { // region 3
                        s = 0.0;
                        if( b1 >= 0.0 ) {
                            t = 0.0;
                            sqrDistance = c;
                        } else if( -b1 >= a11 ) {
                            t = 1.0;
                            sqrDistance = a11 + 2.0 * b1 + c;
                        } else {
                            t = -b1 / a11;
                            sqrDistance = b1 * t + c;
                        }
                    }
                } else if( t < 0.0 ) { // region 5
                    t = 0.0;
                    if( b0 >= 0.0 ) {
                        s = 0.0;
                        sqrDistance = c;
                    } else if( -b0 >= a00 ) {
                        s = 1.0;
                        sqrDistance = a00 + 2.0 * b0 + c;
                    } else {
                        s = -b0 / a00;
                        sqrDistance = b0 * s + c;
                    }
                } else { // region 0
                    // minimum at interior point
                    double invDet = double( 1.0 ) / det;
                    s *= invDet;
                    t *= invDet;
                    sqrDistance = s * ( a00 * s + a01 * t + 2.0 * b0 )
                        + t * ( a01 * s + a11 * t + 2.0 * b1 ) + c;
                }
            } else {
                double tmp0, tmp1, numer, denom;

                if( s < 0.0 ) { // region 2
                    tmp0 = a01 + b0;
                    tmp1 = a11 + b1;
                    if( tmp1 > tmp0 ) {
                        numer = tmp1 - tmp0;
                        denom = a00 - 2.0 * a01 + a11;
                        if( numer >= denom ) {
                            s = 1.0;
                            t = 0.0;
                            sqrDistance = a00 + 2.0 * b0 + c;
                        } else {
                            s = numer / denom;
                            t = 1.0 - s;
                            sqrDistance = s * ( a00 * s + a01 * t + 2.0 * b0 )
                                + t * ( a01 * s + a11 * t + 2.0 * b1 ) + c;
                        }
                    } else {
                        s = 0.0;
                        if( tmp1 <= 0.0 ) {
                            t = 1.0;
                            sqrDistance = a11 + 2.0 * b1 + c;
                        } else if( b1 >= 0.0 ) {
                            t = 0.0;
                            sqrDistance = c;
                        } else {
                            t = -b1 / a11;
                            sqrDistance = b1 * t + c;
                        }
                    }
                } else if( t < 0.0 ) { // region 6
                    tmp0 = a01 + b1;
                    tmp1 = a00 + b0;
                    if( tmp1 > tmp0 ) {
                        numer = tmp1 - tmp0;
                        denom = a00 - 2.0 * a01 + a11;
                        if( numer >= denom ) {
                            t = 1.0;
                            s = 0.0;
                            sqrDistance = a11 + 2.0 * b1 + c;
                        } else {
                            t = numer / denom;
                            s = 1.0 - t;
                            sqrDistance = s * ( a00 * s + a01 * t + 2.0 * b0 )
                                + t * ( a01 * s + a11 * t + 2.0 * b1 ) + c;
                        }
                    } else {
                        t = 0.0;
                        if( tmp1 <= 0.0 ) {
                            s = 1.0;
                            sqrDistance = a00 + 2.0 * b0 + c;
                        } else if( b0 >= 0.0 ) {
                            s = 0.0;
                            sqrDistance = c;
                        } else {
                            s = -b0 / a00;
                            sqrDistance = b0 * s + c;
                        }
                    }
                } else { // region 1
                    numer = a11 + b1 - a01 - b0;
                    if( numer <= 0.0 ) {
                        s = 0.0;
                        t = 1.0;
                        sqrDistance = a11 + 2.0 * b1 + c;
                    } else {
                        denom = a00 - 2.0 * a01 + a11;
                        if( numer >= denom ) {
                            s = 1.0;
                            t = 0.0;
                            sqrDistance = a00 + 2.0 * b0 + c;
                        } else {
                            s = numer / denom;
                            t = 1.0 - s;
                            sqrDistance = s * ( a00 * s + a01 * t + 2.0 * b0 )
                                + t * ( a01 * s + a11 * t + 2.0 * b1 ) + c;
                        }
                    }
                }
            }

            // Account for numerical round-off error.
            if( sqrDistance < 0.0 ) {
                sqrDistance = 0.0;
            }

            closest_point = V0 + s * edge0 + t * edge1;
            return std::sqrt( sqrDistance );
        }

        double point_to_segment(
            const vec2& p,
            const vec2& p0,
            const vec2& p1,
            vec2& nearest_p )
        {
            // The direction vector is not unit length.  The normalization is deferred
            // until it is needed.
            vec2 direction = p1 - p0;
            vec2 diff = p - p1;
            double t = dot( direction, diff );
            if( t >= global_epsilon ) {
                nearest_p = p1;
            } else {
                diff = p - p0;
                t = dot( direction, diff );
                if( t <= global_epsilon ) {
                    nearest_p = p0;
                } else {
                    double sqrLength = dot( direction, direction );
                    if( sqrLength > global_epsilon ) {
                        t /= sqrLength;
                        nearest_p = p0 + t * direction;
                    } else {
                        nearest_p = p0;
                    }
                }
            }

            diff = p - nearest_p;
            return std::sqrt( dot( diff, diff ) );
        }

        double point_to_triangle(
            const vec2& point,
            const vec2& V0,
            const vec2& V1,
            const vec2& V2,
            vec2& closest_point )
        {
            double result = max_float64();
            if( point_inside_triangle( point, V0, V1, V2 ) ) {
                closest_point = point;
                result = 0.0;
            } else {
                vec2 closest[3];
                double distance[3];
                distance[0] = point_to_segment( point, V0, V1, closest[0] );
                distance[1] = point_to_segment( point, V1, V2, closest[1] );
                distance[2] = point_to_segment( point, V2, V0, closest[2] );
                if( distance[0] < distance[1] ) {
                    if( distance[0] < distance[2] ) {
                        result = distance[0];
                        closest_point = closest[0];
                    } else {
                        result = distance[2];
                        closest_point = closest[2];
                    }
                } else {
                    if( distance[1] < distance[2] ) {
                        result = distance[1];
                        closest_point = closest[1];
                    } else {
                        result = distance[2];
                        closest_point = closest[2];
                    }
                }
            }
            return result;
        }

        double point_to_tetra(
            const vec3& p,
            const vec3& p0,
            const vec3& p1,
            const vec3& p2,
            const vec3& p3,
            vec3& nearest_p )
        {
            vec3 vertices[4] = { p0, p1, p2, p3 };
            double dist = max_float64();
            for( index_t f = 0;
                f < GEO::MeshCellDescriptors::tet_descriptor.nb_facets; f++ ) {
                vec3 cur_p;
                double distance =
                    point_to_triangle( p,
                        vertices[GEO::MeshCellDescriptors::tet_descriptor.facet_vertex[f][0]],
                        vertices[GEO::MeshCellDescriptors::tet_descriptor.facet_vertex[f][1]],
                        vertices[GEO::MeshCellDescriptors::tet_descriptor.facet_vertex[f][2]],
                        cur_p );
                if( distance < dist ) {
                    dist = distance;
                    nearest_p = cur_p;
                }
            }
            return dist;
        }

        double point_to_pyramid(
            const vec3& p,
            const vec3& p0,
            const vec3& p1,
            const vec3& p2,
            const vec3& p3,
            const vec3& p4,
            vec3& nearest_p )
        {
            vec3 vertices[5] = { p0, p1, p2, p3, p4 };
            double dist = max_float64();
            for( index_t f = 0;
                f < GEO::MeshCellDescriptors::pyramid_descriptor.nb_facets; f++ ) {
                vec3 cur_p;
                double distance = max_float64();
                index_t nb_vertices =
                    GEO::MeshCellDescriptors::pyramid_descriptor.nb_vertices_in_facet[f];
                if( nb_vertices == 3 ) {
                    distance =
                        point_to_triangle( p,
                            vertices[GEO::MeshCellDescriptors::pyramid_descriptor.facet_vertex[f][0]],
                            vertices[GEO::MeshCellDescriptors::pyramid_descriptor.facet_vertex[f][1]],
                            vertices[GEO::MeshCellDescriptors::pyramid_descriptor.facet_vertex[f][2]],
                            cur_p );
                } else if( nb_vertices == 4 ) {
                    distance =
                        point_to_quad( p,
                            vertices[GEO::MeshCellDescriptors::pyramid_descriptor.facet_vertex[f][0]],
                            vertices[GEO::MeshCellDescriptors::pyramid_descriptor.facet_vertex[f][1]],
                            vertices[GEO::MeshCellDescriptors::pyramid_descriptor.facet_vertex[f][2]],
                            vertices[GEO::MeshCellDescriptors::pyramid_descriptor.facet_vertex[f][3]],
                            cur_p );
                } else {
                    ringmesh_assert_not_reached;
                }
                if( distance < dist ) {
                    dist = distance;
                    nearest_p = cur_p;
                }
            }
            return dist;
        }

        double point_to_prism(
            const vec3& p,
            const vec3& p0,
            const vec3& p1,
            const vec3& p2,
            const vec3& p3,
            const vec3& p4,
            const vec3& p5,
            vec3& nearest_p )
        {
            vec3 vertices[6] = { p0, p1, p2, p3, p4, p5 };
            double dist = max_float64();
            for( index_t f = 0;
                f < GEO::MeshCellDescriptors::prism_descriptor.nb_facets; f++ ) {
                vec3 cur_p;
                double distance = max_float64();
                index_t nb_vertices =
                    GEO::MeshCellDescriptors::prism_descriptor.nb_vertices_in_facet[f];
                if( nb_vertices == 3 ) {
                    distance =
                        point_to_triangle( p,
                            vertices[GEO::MeshCellDescriptors::prism_descriptor.facet_vertex[f][0]],
                            vertices[GEO::MeshCellDescriptors::prism_descriptor.facet_vertex[f][1]],
                            vertices[GEO::MeshCellDescriptors::prism_descriptor.facet_vertex[f][2]],
                            cur_p );
                } else if( nb_vertices == 4 ) {
                    distance =
                        point_to_quad( p,
                            vertices[GEO::MeshCellDescriptors::prism_descriptor.facet_vertex[f][0]],
                            vertices[GEO::MeshCellDescriptors::prism_descriptor.facet_vertex[f][1]],
                            vertices[GEO::MeshCellDescriptors::prism_descriptor.facet_vertex[f][2]],
                            vertices[GEO::MeshCellDescriptors::prism_descriptor.facet_vertex[f][3]],
                            cur_p );
                } else {
                    ringmesh_assert_not_reached;
                }
                if( distance < dist ) {
                    dist = distance;
                    nearest_p = cur_p;
                }
            }
            return dist;
        }

        double point_to_hexa(
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
            vec3 vertices[8] = { p0, p1, p2, p3, p4, p5, p6, p7 };
            double dist = max_float64();
            for( index_t f = 0;
                f < GEO::MeshCellDescriptors::hex_descriptor.nb_facets; f++ ) {
                vec3 cur_p;
                double distance =
                    point_to_quad( p,
                        vertices[GEO::MeshCellDescriptors::hex_descriptor.facet_vertex[f][0]],
                        vertices[GEO::MeshCellDescriptors::hex_descriptor.facet_vertex[f][1]],
                        vertices[GEO::MeshCellDescriptors::hex_descriptor.facet_vertex[f][2]],
                        vertices[GEO::MeshCellDescriptors::hex_descriptor.facet_vertex[f][3]],
                        cur_p );
                if( distance < dist ) {
                    dist = distance;
                    nearest_p = cur_p;
                }
            }
            return dist;
        }

        double point_to_segment(
            const vec3& p,
            const vec3& p0,
            const vec3& p1,
            vec3& nearest_p )
        {
            if( point_segment_projection( p, p0, p1, nearest_p ) ) {
                return length( nearest_p - p );
            } else {
                double p0_sq = length2( p0 - p );
                double p1_sq = length2( p1 - p );
                if( p0_sq < p1_sq ) {
                    nearest_p = p0;
                    return std::sqrt( p0_sq );
                } else {
                    nearest_p = p1;
                    return std::sqrt( p1_sq );
                }
            }
        }

        double point_to_quad(
            const vec3& p,
            const vec3& p0,
            const vec3& p1,
            const vec3& p2,
            const vec3& p3,
            vec3& nearest_p )
        {
            const vec3 center( ( p0 + p1 + p2 + p3 ) * 0.25 );
            vec3 edge0( p1 - p0 );
            vec3 edge1( p3 - p0 );
            vec3 axis[2] = { normalize( edge0 ), normalize( edge1 ) };
            double extent[2] = { 0.5 * edge0.length(), 0.5 * edge1.length() };

            vec3 diff = center - p;
            double b0 = dot( diff, axis[0] );
            double b1 = dot( diff, axis[1] );
            double s0 = -b0;
            double s1 = -b1;
            double sqrDistance = dot( diff, diff );

            if( s0 < -extent[0] ) {
                s0 = -extent[0];
            } else if( s0 > extent[0] ) {
                s0 = extent[0];
            }
            sqrDistance += s0 * ( s0 + 2. * b0 );

            if( s1 < -extent[1] ) {
                s1 = -extent[1];
            } else if( s1 > extent[1] ) {
                s1 = extent[1];
            }
            sqrDistance += s1 * ( s1 + 2. * b1 );

            // Account for numerical round-off error.
            if( sqrDistance < 0 ) {
                sqrDistance = 0;
            }

            double distance = std::sqrt( sqrDistance );
            nearest_p = center;
            nearest_p += s0 * axis[0];
            nearest_p += s1 * axis[1];

            return distance;
        }
    }
}