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

#include <grgmeshlib/mixed_mesh.h>

namespace GRGMesh {

    const CellDescriptor* MixedMesh::cell_descriptor_[7] = {
        &line_descriptor, &trgl_descriptor, &quad_descriptor, &tetra_descriptor,
        &pyramid_descriptor, &prism_descriptor, &hexa_descriptor } ;

    CellType MixedMesh::cell_type( uint32 c, uint32& c_index ) const
    {
        uint8 result = 0 ;
        uint32 size = 0 ;
        for( ; result < 7; result++ ) {
            size += cells_[result].size() ;
            if( c > size ) break ;
        }
        c_index = c - size ;
        return CellType( result ) ;
    }

}

