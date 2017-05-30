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
 *     http://www.ring-team.org
 *
 *     RING Project
 *     Ecole Nationale Superieure de Geologie - GeoRessources
 *     2 Rue du Doyen Marcel Roubault - TSA 70605
 *     54518 VANDOEUVRE-LES-NANCY
 *     FRANCE
 */

namespace {

    struct RINGMesh2Feflow {
        index_t entity_type ;
        index_t vertices[8] ;
    } ;

    static RINGMesh2Feflow feflow_tet_descriptor = { 6, { 0, 1, 2, 3 } } ;

    static RINGMesh2Feflow feflow_hex_descriptor = { 8, { 2, 6, 7, 3, 0, 4, 5, 1 } } ;

    static RINGMesh2Feflow feflow_prism_descriptor = { 7, { 3, 4, 5, 0, 1, 2 } } ;

    static RINGMesh2Feflow feflow_pyramid_descriptor = { 9, { 0, 1, 2, 3, 4 } } ;

    static RINGMesh2Feflow* cell_type_to_feflow_cell_descriptor[4] = {
        &feflow_tet_descriptor, &feflow_hex_descriptor, &feflow_prism_descriptor,
        &feflow_pyramid_descriptor } ;

    class FeflowIOHandler: public GeoModelIOHandler {
    public:
        static const index_t STARTING_OFFSET = 1 ;

        virtual bool load( const std::string& filename, GeoModel& geomodel )
        {
            throw RINGMeshException( "I/O",
                "Loading of a GeoModel from Feflow not implemented yet" ) ;
            return false ;
        }
        virtual void save( const GeoModel& geomodel, const std::string& filename )
        {
            std::ofstream out( filename.c_str() ) ;
            out.precision( 16 ) ;

            write_header( out ) ;
            write_dimensions( geomodel, out ) ;
            write_elements( geomodel, out ) ;
            write_vertices( geomodel, out ) ;
            write_regions( geomodel, out ) ;
            write_wells( geomodel, out ) ;

            out << "END" << std::endl ;
        }

    private:

        void write_header( std::ofstream& out ) const
        {
            out << "PROBLEM:\n" ;
            out << "CLASS (v.7.006.14742)\n" ;
            // 3 is for defining a 3D problem
            // 8 is for using double precision
            // More information on the CLASS keyword Feflow documentation
            out << "   0    0    0    3    0    0    8    8    0    0\n" ;
        }
        void write_dimensions( const GeoModel& geomodel, std::ofstream& out ) const
        {
            const GeoModelMesh& mesh = geomodel.mesh ;
            out << "DIMENS\n" ;
            out << SPACE << mesh.vertices.nb() << SPACE << mesh.cells.nb()
                << " 0 1 0 0 0 0 0 1 0 0 0 0 0 0 0\n" ;
            out << "SCALE\n\n" ;
        }
        void write_elements( const GeoModel& geomodel, std::ofstream& out ) const
        {
            const GeoModelMeshCells& cells = geomodel.mesh.cells ;
            out << "VARNODE\n" ;
            out << SPACE << cells.nb() ;
            index_t min_nb_vertices_per_element ;
            index_t max_nb_vertices_per_element ;
            if( cells.nb_tet() > 0 ) {
                min_nb_vertices_per_element = 4 ;
            } else if( cells.nb_pyramid() > 0 ) {
                min_nb_vertices_per_element = 5 ;
            } else if( cells.nb_prism() > 0 ) {
                min_nb_vertices_per_element = 6 ;
            } else if( cells.nb_hex() > 0 ) {
                min_nb_vertices_per_element = 8 ;
            } else {
                ringmesh_assert_not_reached ;
            }
            if( cells.nb_hex() > 0 ) {
                max_nb_vertices_per_element = 8 ;
            } else if( cells.nb_prism() > 0 ) {
                max_nb_vertices_per_element = 6 ;
            } else if( cells.nb_pyramid() > 0 ) {
                max_nb_vertices_per_element = 5 ;
            } else if( cells.nb_tet() > 0 ) {
                max_nb_vertices_per_element = 4 ;
            } else {
                ringmesh_assert_not_reached ;
            }
            out << SPACE << min_nb_vertices_per_element << SPACE
                << max_nb_vertices_per_element << "\n" ;

            for( index_t c = 0; c < cells.nb(); c++ ) {
                const RINGMesh2Feflow& descriptor =
                    *cell_type_to_feflow_cell_descriptor[cells.type( c )] ;
                out << SPACE << descriptor.entity_type ;
                for( index_t v = 0; v < cells.nb_vertices( c ); v++ ) {
                    out << SPACE
                        << cells.vertex( c, descriptor.vertices[v] )
                            + STARTING_OFFSET ;
                }
                out << "\n" ;
            }
        }
        void write_vertices( const GeoModel& geomodel, std::ofstream& out ) const
        {
            const GeoModelMeshVertices& vertices = geomodel.mesh.vertices ;
            out << "XYZCOOR\n" << std::scientific ;
            for( index_t v = 0; v < vertices.nb(); v++ ) {
                const vec3& point = vertices.vertex( v ) ;
                std::string sep = "";
                for( index_t i = 0; i < 3; i++ ) {
                    out << sep << SPACE << point[i] ;
                    sep = "," ;
                }
                out << "\n" ;
            }
            out << std::fixed ;
        }
        void write_regions( const GeoModel& geomodel, std::ofstream& out ) const
        {
            out << "ELEMENTALSETS\n" ;
            index_t offset = 0 ;
            for( index_t r = 0; r < geomodel.nb_regions(); r++ ) {
                const Region< 3 >& region = geomodel.region( r ) ;
                out << SPACE << region.name() << SPACE << offset + STARTING_OFFSET ;
                offset += region.nb_mesh_elements() ;
                out << "-" << offset << "\n" ;
            }
        }
        void write_wells( const GeoModel& geomodel, std::ofstream& out ) const
        {
            const WellGroup* wells = geomodel.wells() ;
            if( !wells ) {
                return ;
            }
            out << "DFE\n" ;
            out
                << " <?xml version=\"1.0\" encoding=\"utf-8\" standalone=\"no\" ?>\n" ;
            out << " <fractures>\n" ;
            write_well_edges( geomodel, out ) ;
            out << " <properties>\n" ;
            out << " </properties>\n" ;
            write_well_groups( geomodel, out ) ;
            out << " </fractures>\n" ;
        }
        void write_well_edges( const GeoModel& geomodel, std::ofstream& out ) const
        {
            const GeoModelMeshEdges& edges = geomodel.mesh.edges ;
            out << " <nop count=\"" << edges.nb_edges() << "\">\n" ;
            out << " <![CDATA[" ;
            for( index_t w = 0; w < edges.nb_wells(); w++ ) {
                for( index_t e = 0; e < edges.nb_edges( w ); e++ ) {
                    out << "\n 0, 2, " << edges.vertex( w, e, 0 ) + STARTING_OFFSET
                        << ", " << edges.vertex( w, e, 1 ) + STARTING_OFFSET ;
                }
            }
            out << "]]>\n" ;
            out << " </nop>\n" ;
        }
        void write_well_groups( const GeoModel& geomodel, std::ofstream& out ) const
        {
            const GeoModelMeshEdges& edges = geomodel.mesh.edges ;
            const WellGroup* wells = geomodel.wells() ;
            index_t offset = 0 ;
            out << " <groups count=\"" << edges.nb_wells() << "\">\n" ;
            for( index_t w = 0; w < edges.nb_wells(); w++ ) {
                out << " <group name=\"" << wells->well( w ).name()
                    << "\" mode=\"unstructured\">\n" ;
                out << " <elements count=\"" << edges.nb_edges( w ) << "\">\n" ;
                out << " <![CDATA[ " << offset + STARTING_OFFSET ;
                offset += edges.nb_edges( w ) ;
                out << "-" << offset << "]]>\n" ;
                out << " </elements>\n" ;
                out << " </group>\n" ;
            }
            out << " </groups>\n" ;
        }
    } ;

}
