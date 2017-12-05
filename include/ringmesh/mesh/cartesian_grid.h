/*
 * Copyright (c) 2012-2017, Association Scientifique pour la Geologie et ses
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

#pragma once

#include <ringmesh/basic/common.h>
#include <ringmesh/basic/frame.h>

#include <ringmesh/mesh/mesh.h>

namespace GEO
{
    class AttributesManager;
} // namespace GEO

namespace RINGMesh
{
    FORWARD_DECLARATION_DIMENSION_CLASS( CartesianGridBuilder );
}

/*!
 * @file Generic class for creating cartesian grids
 * @author Melchior Schuh-Senlis
 */

namespace RINGMesh
{
    /**
     * Template class for Cartesian grids of different dimensions
     */
    template < index_t DIMENSION >
    class RINGMESH_API CartesianGrid
    {
        ringmesh_disable_copy_and_move( CartesianGrid );
        friend class CartesianGridBuilder< DIMENSION >;

    public:
        CartesianGrid( ivecn< DIMENSION > nb_cells_in_each_direction,
            ReferenceFrame< DIMENSION > vec_cartesian_axis )
            : nb_cells_in_each_direction_(
                  std::move( nb_cells_in_each_direction ) ),
	          nb_total_cells_( 1 ),
              cartesian_frame_( std::move( vec_cartesian_axis ) ),
              inverse_cartesian_frame_( ReferenceFrameManipulator< DIMENSION >::
                      reference_frame_from_global_to_local( cartesian_frame_ ) )
        {
            if( !ReferenceFrameManipulator< DIMENSION >::frame_is_orthogonal(
                    cartesian_frame_ ) )
            {
                throw RINGMeshException( "RINGMesh Test",
                    "Warning : the frame of the Cartesian Grid "
                    "you're trying to create is not orthogonal " );
            }
            for( auto i : range( DIMENSION ) )
            {
                if( nb_cells_in_each_direction[i] < 1 )
                {
                    throw RINGMeshException( "RINGMesh Test",
                        "Warning : You're trying to create a Cartesian Grid "
                        "with no cell in direction ",
                        i, ", and Cartesian Grid must have at least one cell "
                           "in each direction" );
                }
                nb_total_cells_ *= nb_cells_in_each_direction[i];
            }
            attributes_manager_.resize( nb_total_cells_ );
        }

        void save_mesh( const std::string& filename ) const
        {
            // TODO
        }

        //        void print_mesh_bounded_attributes( const std::string&
        //        output_location ) const
        //        {
        //        	print_bounded_attributes( attributes_manager_,
        //        output_location );
        //        }

        void resize( ivecn< DIMENSION >& new_size,
            ReferenceFrame< DIMENSION > vec_cartesian_axis )
        {
            Logger::warn( "Warning : You are currently changing the size of "
                          "the Cartesian grid, this will affect the values of "
                          "the attributes in the grid !" );
            nb_cells_in_each_direction_ = std::move( new_size );
            nb_total_cells_ = 1;
            for( auto i : range( DIMENSION ) )
            {
                if( nb_cells_in_each_direction_[i] < 1 )
                {
                    throw RINGMeshException( "RINGMesh Test",
                        "Warning : You're trying to create a Cartesian Grid "
                        "with no cell in direction ",
                        i, ", and Cartesian Grid must have at least one cell "
                           "in each direction" );
                }
                nb_total_cells_ *= nb_cells_in_each_direction_[i];
            }
            if( !ReferenceFrameManipulator< DIMENSION >::frame_is_orthogonal(
                    vec_cartesian_axis ) )
            {
                throw RINGMeshException( "RINGMesh Test",
                    "Warning : the frame of the Cartesian Grid "
                    "you're trying to create is not orthogonal " );
            }
            cartesian_frame_ = std::move( vec_cartesian_axis );
            attributes_manager_.resize( nb_total_cells_ );
        }

        void change_frame( ReferenceFrame< DIMENSION >& vec_cartesian_axis )
        {
            if( !ReferenceFrameManipulator< DIMENSION >::frame_is_orthogonal(
                    vec_cartesian_axis ) )
            {
                throw RINGMeshException( "RINGMesh Test",
                    "Warning : the frame of the Cartesian Grid "
                    "you're trying to create is not orthogonal " );
            }
            Logger::warn( "Warning : You are currently changing the frame of "
                          "the Cartesian grid, this will affect where the "
                          "values of the attributes in the grid are stored in "
                          "space !" );
            cartesian_frame_ = std::move( vec_cartesian_axis );
            inverse_cartesian_frame_ = ReferenceFrameManipulator< DIMENSION >::
                reference_frame_from_global_to_local( cartesian_frame_ );
        }

        void change_attribute_manager(
            GEO::AttributesManager attributes_manager )
        {
            attributes_manager_ = std::move( attributes_manager );
        }

        vecn< DIMENSION >& cell_center_global_coords(
            const ivecn< DIMENSION >& cartesian_coords ) const
        {
            vecn< DIMENSION > cartesian_double_coords;
            for( auto i : range( DIMENSION ) )
            {
                cartesian_double_coords[i] =
                    static_cast< double >( cartesian_coords[i] );
            }
            return ReferenceFrameManipulator< DIMENSION >::
                coords_from_frame_to_global(
                    cartesian_frame_, cartesian_double_coords );
        }

        ivecn< DIMENSION >& containing_cell_from_global_vertex(
            const vecn< DIMENSION >& reference_vertex ) const
        {
            return this->containing_cell_from_local_vertex(
                ReferenceFrameManipulator< DIMENSION >::
                    coords_from_frame_to_global(
                        inverse_cartesian_frame_, reference_vertex ) );
        }

        ivecn< DIMENSION > containing_cell_from_local_vertex(
            const vecn< DIMENSION >& vertex ) const
        {
            ivecn< DIMENSION > coord;
            for( auto i : range( DIMENSION ) )
            {
                coord[i] = std::floor( vertex[i] + 0.5 );
            }
            return coord;
        }

        index_t cell_offset( const ivecn< DIMENSION >& coords ) const
        {
            index_t offset{ 0 };
            index_t mult{ 1 };
            for( auto i : range( DIMENSION ) )
            {
                offset += coords[i] * mult;
                mult *= nb_cells_in_each_direction_[i];
            }
            return offset;
        }

        ivecn< DIMENSION > local_from_offset( const index_t offset ) const
        {
            ivecn< DIMENSION > coords;
            index_t off{ 0 };
            index_t div{ nb_total_cells_
                         / nb_cells_in_each_direction_[DIMENSION - 1] };
            for( auto i : range( DIMENSION ) )
            {
                coords[DIMENSION - 1 - i] = ( offset - off ) / div;
                off += coords[DIMENSION - 1 - i] * div;
                div /= nb_cells_in_each_direction_[DIMENSION - 1 - i];
            }
            return coords;
        }

        index_t nb_cells() const
        {
            return nb_total_cells_;
        }

        GEO::AttributesManager& attributes_manager() const
        {
            return attributes_manager_;
        }

        double cell_volume() const
        {
            double cell_volume{ 1. };
            double vector_norm;
            for( auto i : range( DIMENSION ) )
            {
                vector_norm = 0;
                for( auto j : range( DIMENSION ) )
                {
                    vector_norm +=
                        cartesian_frame_[i][j] * cartesian_frame_[i][j];
                }
                cell_volume *= std::sqrt( vector_norm );
            }
            return cell_volume;
        }

    protected:
        CartesianGrid() = default;

        ivecn< DIMENSION > nb_cells_in_each_direction_;
        index_t nb_total_cells_;

        ReferenceFrame< DIMENSION > cartesian_frame_;
        ReferenceFrame< DIMENSION > inverse_cartesian_frame_;

        GEO::AttributesManager attributes_manager_;
    };
    ALIAS_2D_AND_3D( CartesianGrid );

    //    class RINGMESH_API CartesianGridVolumeMesh : public VolumeMesh3D
    //    {
    //    public:
    //        CartesianGridVolumeMesh() = default;
    //
    //        static MeshType type_name_static()
    //        {
    //            return "CartesianGrid";
    //        }
    //
    //        MeshType type_name() const override
    //        {
    //            return type_name_static();
    //        }
    //
    //        index_t nb_cells() const override
    //        {
    //            return cartesian_grid_.nb_cells();
    //        }
    //
    //        //        GEO::AttributesManager& cell_attribute_manager() const
    //        //        override
    //        //        {
    //        //            return cartesian_grid_.attributes_manager();
    //        //        }
    //
    //        CellType cell_type( index_t cell_id ) const override
    //        {
    //            return static_cast< CellType >( 1 );
    //        }
    //
    //        double cell_volume( index_t cell_id ) const override
    //        {
    //            return cartesian_grid_.cell_volume();
    //        }
    //
    //    private:
    //        CartesianGrid3D cartesian_grid_;
    //    };
}
