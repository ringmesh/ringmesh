/*
 *  Copyright (c) 2012-2014, Bruno Levy
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *  this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *  this list of conditions and the following disclaimer in the documentation
 *  and/or other materials provided with the distribution.
 *  * Neither the name of the ALICE Project-Team nor the names of its
 *  contributors may be used to endorse or promote products derived from this
 *  software without specific prior written permission.
 * 
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 *  If you modify this software, you should include a notice giving the
 *  name of the person performing the modification, the date of modification,
 *  and the reason for such modification.
 *
 *  Contact: Bruno Levy
 *
 *     Bruno.Levy@inria.fr
 *     http://www.loria.fr/~levy
 *
 *     ALICE Project
 *     LORIA, INRIA Lorraine, 
 *     Campus Scientifique, BP 239
 *     54506 VANDOEUVRE LES NANCY CEDEX 
 *     FRANCE
 *
 */

#include <geogram/basic/common.h>
#include <geogram/basic/logger.h>
#include <geogram/basic/command_line.h>
#include <geogram/basic/command_line_args.h>
#include <geogram/basic/stopwatch.h>
#include <geogram/basic/file_system.h>
#include <geogram/basic/process.h>
#include <geogram/basic/progress.h>
#include <geogram/basic/matrix.h>
#include <geogram/basic/permutation.h>
#include <geogram/mesh/mesh.h>
#include <geogram/mesh/mesh_io.h>
#include <geogram/mesh/mesh_private.h>
#include <geogram/mesh/mesh_reorder.h>
#include <geogram/mesh/mesh_repair.h>
#include <geogram/mesh/mesh_geometry.h>
#include <geogram/voronoi/CVT.h>
#include <geogram/voronoi/RVD.h>
#include <geogram/voronoi/generic_RVD_vertex.h>
#include <geogram/delaunay/delaunay.h>
#include <geogram/delaunay/delaunay_3d.h>
#include <geogram/points/nn_search.h>
#include <geogram/numerics/optimizer.h>
#include <stack>
#include <iterator>

namespace {
    using namespace GEO;

const char* banner[] = {
" _______        ______    ____        __               ____       _           \n",
"|  ___\\ \\      / /  _ \\  / /\\ \\      / /_ _ _ __ _ __ |  _ \\ _ __(_)_   _____ \n",
"| |_   \\ \\ /\\ / /| | | |/ /  \\ \\ /\\ / / _` | '__| '_ \\| | | | '__| \\ \\ / / _ \\\n",
"|  _|   \\ V  V / | |_| / /    \\ V  V / (_| | |  | |_) | |_| | |  | |\\ V /  __/\n",
"|_|      \\_/\\_/  |____/_/      \\_/\\_/ \\__,_|_|  | .__/|____/|_|  |_| \\_/ \\___|\n",
"                                                |_|                           \n"
};

    /**
     * \brief Computes the linear least squares
     * regression of a function evaluated
     * in 3d.
     */

    // TODO: have a linear solve function that does
    // not require a template argument...

    class LinearLeastSquares {
    public:
        /**
         * \brief Constructs a new LinearLeastSquares
         * \param[in] degree one of 1 (linear), 2 (quadratic)
         */
        LinearLeastSquares(
            index_t degree
        ) :
            degree_(degree)
        {
            switch(degree_) {
                case 1:
                    dim_ = 4;
                    break;
                case 2:
                    dim_ = 10;
                    break;
                default:
                    geo_assert_not_reached;
            }
        }

        /**
         * \brief Starts a new computation.
         */
        void begin() {
            AtA_4_.load_zero();
            AtA_10_.load_zero();
            for(index_t i = 0; i < MAX_DIM; ++i) {
                Atb_[i] = 0.0;
            }
        }

        /**
         * \brief Ends the current computation.
         * \details Computes the current equation
         *  from the set of samples declared with
         *  add_point().
         */
        void end() {
            switch(degree_) {
                case 1:
                {
                    Matrix<double, 4> M = AtA_4_.inverse();
                    mult(M, Atb_, eqn_);
                } break;
                case 2:
                {
                    Matrix<double, 10> M = AtA_10_.inverse();
                    mult(M, Atb_, eqn_);
                } break;
                default:
                {
                    geo_assert_not_reached;
                } break;
            }
        }

        /**
         * \brief Adds a sample to the current computation.
         * \details This function needs to be called between
         *  a begin() / end() pair.
         * \param[in] p 3d coordinates of the point
         * \param[in] v function value associated with \p p_in
         */
        void add_point(const double* p, double v) {
            double b[MAX_DIM];
            eval_basis(p, b);

            for(index_t i = 0; i < dim(); ++i) {
                for(index_t j = 0; j < dim(); ++j) {
                    switch(degree_) {
                        case 1:
                            AtA_4_(i, j) += b[i] * b[j];
                            break;
                        case 2:
                            AtA_10_(i, j) += b[i] * b[j];
                            break;
                        default:
                            geo_assert_not_reached;
                            break;
                    }
                }
                Atb_[i] += b[i] * v;
            }
        }

        /**
         * \brief Evaluates the least-squares linear estimate
         *  at a given point.
         * \details This function beeds to be called after end().
         * \param[in] p 3d coordinates of the point
         * \return the linear estimate at \p p
         */
        double eval(const double* p) const {
            double b[MAX_DIM];
            for(index_t i = 0; i < MAX_DIM; ++i) {
                b[i] = 0.0;
            }
            eval_basis(p, b);
            double result = 0;
            for(index_t i = 0; i < dim(); ++i) {
                result += eqn_[i] * b[i];
            }
            return result;
        }

    protected:
        /**
         * \brief Gets the dimension of the function basis.
         */
        index_t dim() const {
            return dim_;
        }

        /**
         * \brief Evaluates the function basis at a given
         *  point.
         * \param[in] p 3d coordinates of the point
         * \param[out] b array of size dim(), value of the
         *  function basis at \p p
         */
        void eval_basis(const double* p, double* b) const {
            double x = p[0];
            double y = p[1];
            double z = p[2];
            b[0] = 1.0;
            b[1] = x;
            b[2] = y;
            b[3] = z;
            if(degree_ >= 2) {
                b[4] = x * x;
                b[5] = y * y;
                b[6] = z * z;
                b[7] = x * y;
                b[8] = y * z;
                b[9] = z * x;
            }
        }

        /**
         * \brief Maximum dimension of the function basis
         */
        static const int MAX_DIM = 10;

    private:
        index_t degree_;
        index_t dim_;
        Matrix<double, 4> AtA_4_;
        Matrix<double, 10> AtA_10_;
        double Atb_[MAX_DIM];
        double eqn_[MAX_DIM];
    };

    /*************************************************************************/

    /**
     * \brief Computes the contribution of an integration simplex
     *  to the objective function minimized by a semi-discrete
     *  optimal transport map.
     */
    class OTMIntegrationSimplex : public IntegrationSimplex {
    public:

        /**
         * \brief Constructs a new OTMIntegrationSimplex.
         * \param[in] M the input mesh
         */
        OTMIntegrationSimplex(
            const Mesh& M
        ) : IntegrationSimplex(
            M, true, 0, 0, nil
        ), w_(nil) {
        }
        
        /**
         * \brief Sets the weight vector
         * \param[in] w a const pointer to the weight vector.
         */
        void set_w(const double* w) { 
            w_ = w ; 
        }

        virtual double eval(
            index_t center_vertex_index,
            const GEOGen::Vertex& v1,
            const GEOGen::Vertex& v2,
            const GEOGen::Vertex& v3,
            index_t frame_index
        ) {
            geo_argused(frame_index);
            const double* p0 = point(center_vertex_index);
            const double* p1 = v1.point();
            const double* p2 = v2.point();
            const double* p3 = v3.point();
            double m = Geom::tetra_signed_volume(p0, p1, p2, p3);
            double fT = 0.0;
            for(coord_index_t c = 0; c < 3; ++c) {
                double Uc = p1[c] - p0[c];
                double Vc = p2[c] - p0[c];
                double Wc = p3[c] - p0[c];
                fT += Uc * Uc + Vc * Vc + Wc * Wc + Uc * Vc + Vc * Wc + Wc * Uc;
            }
            fT = m * (fT / 10.0 - w_[center_vertex_index]);
            if(spinlocks_ != nil) {
                spinlocks_->acquire_spinlock(center_vertex_index);
            }
            g_[center_vertex_index] -= m;
            if(spinlocks_ != nil) {
                spinlocks_->release_spinlock(center_vertex_index);
            }
            return -fT;
        }

    private:
        const double* w_;
    } ;

    /*************************************************************************/

    /**
     * \brief Computes semi-discrete optimal transport maps.
     * \details Computes an optimal transport map between two
     *  distributions in 3D. The first distribution is represented
     *  by a 3D tetrahedral mesh. The second distribution is a sum
     *  of Diracs.
     *  The algorithm is described in the following references:
     *   - 3D algorithm: http://arxiv.org/abs/1409.1279
     *   - Earlier 2D version by Quentin M\'erigot: 
     *    Q. Merigot. A multiscale approach to optimal transport.
     *    Computer Graphics Forum 30 (5) 1583–1592, 2011 (Proc SGP 2011).
     *   - Earlier article on OT and power diagrams: 
     *    F. Aurenhammer, F. Hoffmann, and B. Aronov. Minkowski-type theorems 
     *    and least-squares clustering. Algorithmica, 20:61-76, 1998.
     */
    class OptimalTransportMap {
    public:
        /**
         * \brief Initializes a new OptimalTransportMap.
         * \param[in] mesh the source distribution, represented as a 3d mesh
         * \param[in] delaunay factory name of the Delaunay triangulation
         */
        OptimalTransportMap(
            Mesh* mesh,
            const std::string& delaunay = "default"
        ) :
            mesh_(mesh)
        {
            // Note: we represent power diagrams as 4d Voronoi diagrams
            delaunay_ = Delaunay::create(4, delaunay);
            RVD_ = RestrictedVoronoiDiagram::create(delaunay_, mesh_);
            RVD_->set_volumetric(true);
            RVD_->set_check_SR(true);
            RVD_->create_threads();

            //   No need to reorder vertices if BRIO is activated since
            // vertices are then already reordered.
            if(CmdLine::get_arg_bool("BRIO")) {
                RVD_->delaunay()->set_reorder(false);
            }

            instance_ = nil;
            lambda_p_ = 0.0;
            total_volume_ = 0.0;
            current_call_iter_ = 0;
            epsilon_ = 0.01;
            level_ = 0;
            
            simplex_func_ = new OTMIntegrationSimplex(*mesh);
        }

        /**
         * \brief Sets the points that define the target distribution.
         * \param[in] nb_points number of points in the target distribution
         * \param[in] points coordinates of the Diracs centers in the target
         *  distribution.
         */
        void set_points(index_t nb_points, const double* points) {
            // Note: we represent power diagrams as 4d Voronoi diagrams.
            // The target points are lifted to 4d.
            points_4d_.resize(nb_points * 4);
            for(index_t i = 0; i < nb_points; ++i) {
                points_4d_[i * 4] = points[i * 3];
                points_4d_[i * 4 + 1] = points[i * 3 + 1];
                points_4d_[i * 4 + 2] = points[i * 3 + 2];
                points_4d_[i * 4 + 3] = 0.0;
            }
            weights_.assign(nb_points, 0);
            total_volume_ = 0.0;
            for(index_t t = 0; t < mesh_->nb_tets(); ++t) {
                total_volume_ += Geom::tetra_volume<3>(
                    mesh_->vertex_ptr(mesh_->tet_vertex_index(t, 0)),
                    mesh_->vertex_ptr(mesh_->tet_vertex_index(t, 1)),
                    mesh_->vertex_ptr(mesh_->tet_vertex_index(t, 2)),
                    mesh_->vertex_ptr(mesh_->tet_vertex_index(t, 3))
                );
            }
            lambda_p_ = total_volume_ / double(nb_points);
        }

        /**
         * \brief Sets the maximum error.
         * \param eps acceptable relative deviation for the measure of a
         *   Voronoi cell.
         */
        void set_epsilon(double eps) {
            epsilon_ = eps;
        }

        /**
         * \brief Computes the weights that realize the optimal
         *  transport map between the source mesh and the target
         *  pointset.
         * \param[in] max_iterations maximum number of solver iterations.
         */
        void optimize(index_t max_iterations) {
            level_ = 0;
            index_t n = index_t(points_4d_.size() / 4);
            index_t m = 7;
            Optimizer_var optimizer = Optimizer::create("HLBFGS");
            optimizer->set_epsg(gradient_threshold(n));
            optimizer->set_epsf(0.0);
            optimizer->set_epsx(0.0);
            optimizer->set_newiteration_callback(newiteration_CB);
            optimizer->set_funcgrad_callback(funcgrad_CB);
            optimizer->set_N(n);
            optimizer->set_M(m);
            optimizer->set_max_iter(max_iterations);
            instance_ = this;
            current_call_iter_ = 0;
            optimizer->optimize(&weights_[0]);
            instance_ = nil;
            // To make sure everything is reset properly
            double dummy = 0;
            funcgrad(n, &weights_[0], dummy, nil);
            Logger::out("OTM")
                << "Used " << current_call_iter_ << "iterations" << std::endl;
        }

        /**
         * \brief Optimizes one level of the multilevel algorithm.
         * \details The function supposes that the sequence [0,b)
         *  has been previously optimized. It is used to initialize
         *  the sequence [b,e). The whole sequence [0,e) is then
         *  optimized.
         * \param[in] b index fo the first point in the level
         * \param[in] e one position past the last index of the level
         * \param[in] max_iterations maximum number of iterations
         */
        void optimize_level(index_t b, index_t e, index_t max_iterations) {

            // If this is not the first level, propagate the weights from
            // the lower levels.
            if(b != 0) {

                //   Create a nearest neighbor search data structure
                // and insert the [0..b) samples into it (they were
                // initialized at previous calls).
                NearestNeighborSearch_var NN = NearestNeighborSearch::create(3);
                NN->set_points(b, &points_4d_[0], 4);
                index_t degree = CmdLine::get_arg_uint("fitting_degree");

                // If degree \notin {1,2}, use weight of nearest sample
                if(degree < 1 || degree > 2) {
                    for(index_t i = b; i < e; ++i) {
                        weights_[i] =
                            weights_[
                            NN->get_nearest_neighbor(&points_4d_[4 * i])
                            ];
                    }
                } else {

                    //   If degree \in {1,2} use linear least squares to
                    // compute an estimate of the weight function.
                    LinearLeastSquares LLS(degree);
                    for(index_t i = b; i < e; ++i) {
                        const index_t nb = 10 * degree;
                        index_t neighbor[100];
                        double dist[100];
                        NN->get_nearest_neighbors(
                            nb, &points_4d_[4 * i], neighbor, dist
                        );
                        LLS.begin();
                        for(index_t jj = 0; jj < nb; ++jj) {
                            if(dist[jj] != 0.0) {
                                index_t j = neighbor[jj];
                                LLS.add_point(&points_4d_[4 * j], weights_[j]);
                            }
                        }
                        LLS.end();
                        weights_[i] = LLS.eval(&points_4d_[4 * i]);
                    }
                }
            }

            // Optimize the weights associated with the sequence [0,e)
            index_t n = e;

            // Important! lambda_p_ (target measure of a cell) needs
            // to be updated, since it depends on the number of samples
            // (that varies at each level).
            lambda_p_ = total_volume_ / double(n);

            index_t m = 7;
            Optimizer_var optimizer = Optimizer::create("HLBFGS");
            optimizer->set_epsg(gradient_threshold(n));
            optimizer->set_epsf(0.0);
            optimizer->set_epsx(0.0);
            optimizer->set_newiteration_callback(newiteration_CB);
            optimizer->set_funcgrad_callback(funcgrad_CB);
            optimizer->set_N(n);
            optimizer->set_M(m);
            optimizer->set_max_iter(max_iterations);
            instance_ = this;
            current_call_iter_ = 0;
            optimizer->optimize(&weights_[0]);
            instance_ = nil;

            // To make sure everything is reset properly
            double dummy = 0;
            funcgrad(n, &weights_[0], dummy, nil);
        }

        /**
         * \brief Multi-level optimization.
         * \details The points specified by set_points() need to have
         *   a hierarchical structure. They can be constructed by
         *   compute_hierarchical_sampling().
         * \param[in] levels sample indices that correspond to level l are
         *   in the range levels[l] (included) ... levels[l+1] (excluded)
         * \param[in] max_iterations maximum number of iterations
         * \see compute_hierarchical_sampling()
         */
        void optimize_levels(
            const vector<index_t>& levels, index_t max_iterations
        ) {
            if(levels.size() > 2) {
                Logger::out("OTM") << "Using " << levels.size()-1
                                   << " levels" << std::endl;
            } else {
                Logger::out("OTM") << "Using 1 level" << std::endl;
            }
            for(index_t l = 0; l + 1 < levels.size(); ++l) {
                level_ = l+1;
                index_t b = levels[l];
                index_t e = levels[l + 1];
                vector<index_t> brio_levels;
                for(index_t i=0; i<=l+1; ++i) {
                    brio_levels.push_back(levels[i]);
                }
                RVD_->delaunay()->set_BRIO_levels(brio_levels);
                optimize_level(b, e, max_iterations);
            }
        }

        /**
         * \brief Gets the number of points.
         * \return The number of points, that was previously defined
         *  by set_points()
         */
        index_t nb_points() const {
            return weights_.size();
        }

        /**
         * \brief Gets a point.
         * \param[in] i index of the point
         * \return a const pointer to the point \p i
         */
        const double* point_ptr(index_t i) const {
            geo_debug_assert(i < nb_points());
            return &(points_4d_[4 * i]);
        }

        /**
         * \brief Gets a weight.
         * \param[in] i index of the point
         * \return the weight that was computed for point \p i
         */
        double weight(index_t i) const {
            return weights_[i];
        }

        /**
         * \brief Gets the value of Kantorowich potential 
         *  at a given point.
         * \param[in] i index of the point
         * \return the potential that was computed for point \p i
         */
        double potential(index_t i) const {
            return points_4d_[4 * i + 3];
        }

        /**
         * \brief Callback for the numerical solver.
         * \details Evaluates the objective function and its gradient.
         * \param[in] n number of variables
         * \param[in] x current value of the variables
         * \param[out] f current value of the objective function
         * \param[out] g gradient of the objective function
         */
        static void funcgrad_CB(
            index_t n, double* x, double& f, double* g
        ) {
            instance_->funcgrad(n, x, f, g);
        }

        /**
         * \brief Callback for the numerical solver.
         * \param[in] n number of variables
         * \param[in] x current value of the variables
         * \param[in] f current value of the objective function
         * \param[in] g gradient of the objective function
         * \param[in] gnorm norm of the gradient of the objective function
         */
        static void newiteration_CB(
            index_t n, const double* x, double f, const double* g, double gnorm
        ) {
            geo_argused(n);
            geo_argused(x);
            geo_argused(f);
            geo_argused(g);
            geo_argused(gnorm);
            instance_->newiteration();
        }

        /**
         * \brief Gets the restricted Voronoi diagram.
         * \return a pointer to the restricted Voronoi diagram
         */
        RestrictedVoronoiDiagram* RVD() {
            return RVD_;
        }

    protected:
        /**
         * \brief Callback for the numerical solver.
         */
        void newiteration() {
        }

        /**
         * \brief Computes the objective function and its gradient.
         * \param[in] n number of variables
         * \param[in] w current value of the variables
         * \param[out] f current value of the objective function
         * \param[out] g gradient of the objective function
         */
        void funcgrad(index_t n, double* w, double& f, double* g) {

            // Step 1: determine the 4d embedding from the weights
            double W = 0.0;
            for(index_t p = 0; p < n; ++p) {
                W = geo_max(W, w[p]);
            }
            for(index_t p = 0; p < n; ++p) {
                points_4d_[4 * p + 3] = ::sqrt(W - w[p]);
            }

            // Step 2: compute function and gradient 
            delaunay_->set_vertices(n, &points_4d_[0]);

            if(g == nil) {
                CmdLine::ui_clear_line();
                CmdLine::ui_message(last_stats_ + "\n");
                return;
            }

            index_t nb_empty_cells = 0;

            f = 0.0;
            for(index_t p = 0; p < n; ++p) {
                g[p] = 0.0;
            }

            OTMIntegrationSimplex* simplex_func_otm = 
                dynamic_cast<OTMIntegrationSimplex*>(simplex_func_.get());
            simplex_func_otm->set_w(w);
            RVD_->compute_integration_simplex_func_grad(f, g, simplex_func_otm);

            double max_diff = 0.0;
            double avg_diff = 0.0;
            for(index_t p = 0; p < n; ++p) {
                f += lambda_p_ * w[p];

                // Note: we minimize -f instead of maximizing f,
                // therefore, in the paper:
                //    g[p] = lambda_p - mesure(power cell associated with p)
                //
                // What is programmed:
                //    g[p] = mesure(power cell associated with p) - lambda_p

                if(::fabs(g[p]) < 1e-10) {
                    nb_empty_cells++;
                }

                g[p] = -g[p] - lambda_p_;

                double cur_diff = ::fabs(g[p]);
                max_diff = geo_max(max_diff, cur_diff);
                avg_diff += cur_diff / double(n);
            }

            double gNorm = 0.0;
            for(index_t i = 0; i < n; ++i) {
                gNorm += geo_sqr(g[i]);
            }
            gNorm = ::sqrt(gNorm);

            std::ostringstream str;
            if(level_ == 0) {
                str << "o-[OTM         ] " ;
            } else {
                str << "o-[OTM Lvl." << level_ << "   ] " ;
            }
            str << "iter=" << current_call_iter_
                << " nbZ=" << nb_empty_cells
                //                << " f=" << f
                //                << " avg_diff=" << avg_diff
                //                << " max_diff=" << max_diff
                << " g=" << gNorm
                << " threshold=" << gradient_threshold(n);
            last_stats_ = str.str();

            // "custom task progress" (clears the standard message
            // and replaces it with another one). 
            if(current_call_iter_ != 0) {
                CmdLine::ui_clear_line();
            }
            CmdLine::ui_message(str.str());
            ++current_call_iter_;
        }

        /**
         * \brief Computes the stopping criterion of the solver.
         * \details The stopping criterion is determined from
         *  the user-specified epsilon, number of samples and
         *  target measure of a cell (lambda_p_).
         * \param n number of samples
         * \return the gradient threshold
         * \see set_epsilon()
         */
        double gradient_threshold(index_t n) const {
            return ::sqrt(double(n) * geo_sqr(epsilon_ * lambda_p_));
        }

    private:
        static OptimalTransportMap* instance_;
        Mesh* mesh_;
        Delaunay_var delaunay_;
        RestrictedVoronoiDiagram_var RVD_;
        vector<double> points_4d_;
        vector<double> weights_;
        double total_volume_;
        double lambda_p_; /**< \brief Value of one of the Diracs */
        double epsilon_;
        /**< \brief Acceptable relative deviation for the measure of a cell */
        index_t current_call_iter_;
        IntegrationSimplex_var simplex_func_;
        std::string last_stats_;
        index_t level_;
    };

    OptimalTransportMap* OptimalTransportMap::instance_ = nil;

    /**
     * \brief Gets the triangles of the restricted Delaunay triangulation.
     * \param[in] RVD the restricted Voronoi diagram
     * \param[out] RDT all the facets of the restricted Delaunay triangulation
     * \param[in] volumetric if true, the facets of the (volumetric) Delaunay
     *   tetrahedralization are returned, else the facets of the (surfacic)
     *   restricted Delaunay triangulation are returned
     * \param[in] insert_flipped_triangles if true, in surfacic mode, for each
     *  pair of adjacent triangles, the corresponding pair of flipped triangles
     *  is inserted as well
     */
    void get_RDT(
        RestrictedVoronoiDiagram* RVD,
        std::set<trindex>& RDT,
        bool volumetric = true,
        bool insert_flipped_triangles = false
    ) {
        RDT.clear();
        if(volumetric) {
            vector<index_t> simplices;
            vector<double> embedding;
            RVD->compute_RDT(
                simplices,
                embedding,
                RestrictedVoronoiDiagram::RDT_SEEDS_ALWAYS
            );
            for(index_t i = 0; i < simplices.size(); i += 4) {
                index_t a = simplices[i];
                index_t b = simplices[i + 1];
                index_t c = simplices[i + 2];
                index_t d = simplices[i + 3];
                RDT.insert(trindex(a, b, c));
                RDT.insert(trindex(a, b, d));
                RDT.insert(trindex(a, c, d));
                RDT.insert(trindex(b, c, d));
            }
        }
        if(!volumetric || insert_flipped_triangles) {
            RVD->set_volumetric(false);
            vector<index_t> simplices;
            vector<double> embedding;
            RVD->compute_RDT(
                simplices,
                embedding,
                RestrictedVoronoiDiagram::RDT_SEEDS_ALWAYS
            );
            RVD->set_volumetric(true);

            for(index_t i = 0; i < simplices.size(); i += 3) {
                index_t a = simplices[i];
                index_t b = simplices[i + 1];
                index_t c = simplices[i + 2];
                RDT.insert(trindex(a, b, c));
            }

            if(insert_flipped_triangles) {
                // Terribly unefficient implementation (for now)
                //   Determine for each edge the list of triangle vertices
                // opposite to the edge.
                typedef std::map<bindex, std::vector<index_t> > E2T;
                E2T e2t;
                for(index_t i = 0; i < simplices.size(); i += 3) {
                    index_t a = simplices[i];
                    index_t b = simplices[i + 1];
                    index_t c = simplices[i + 2];
                    e2t[bindex(a, b)].push_back(c);
                    e2t[bindex(b, c)].push_back(a);
                    e2t[bindex(c, a)].push_back(b);
                }

                //  Insert the flipped edges into the list of triangles
                for(
                    E2T::const_iterator it = e2t.begin(); it != e2t.end(); ++it
                ) {
                    index_t a = it->first.indices[0];
                    index_t b = it->first.indices[1];
                    if(it->second.size() == 2) {
                        index_t c1 = it->second[0];
                        index_t c2 = it->second[1];
                        RDT.insert(trindex(a, c1, c2));
                        RDT.insert(trindex(b, c1, c2));
                    }
                }
            }
        }
    }

    /**
     * \brief Gets the number of connected components
     *  of each tetrahedra regions in a mesh.
     * \param[in] RVD a const reference to the mesh
     * \param[out] nb_cnx_comps nb_cnx_comps[r]
     *  contains the number of connected components of
     *  region r.
     */
    void get_nb_connected_components(
        const Mesh& RVD, vector<index_t>& nb_cnx_comps
    ) {
        vector<bool> marked(RVD.nb_tets(), false);
        std::stack<index_t> S;
        for(index_t t = 0; t < RVD.nb_tets(); ++t) {
            if(!marked[t]) {
                index_t cur_v = index_t(RVD.tet_region(t));
                marked[t] = true;
                S.push(t);
                while(!S.empty()) {
                    index_t cur_t = S.top();
                    S.pop();
                    for(index_t lf = 0; lf < 4; ++lf) {
                        signed_index_t sneigh = RVD.tet_adjacent(cur_t, lf);
                        if(sneigh >= 0.0) {
                            index_t neigh = index_t(sneigh);
                            if(
                                RVD.tet_region(neigh) == 
                                signed_index_t(cur_v) &&
                                !marked[neigh]
                            ) {
                                marked[neigh] = true;
                                S.push(neigh);
                            }
                        }
                    }
                }
                if(cur_v >= nb_cnx_comps.size()) {
                    nb_cnx_comps.resize(cur_v,0);
                }
                ++nb_cnx_comps[cur_v];
            }
        }
    }

    /**
     * \brief Computes a shape that interpolates the two input tet
     *  meshes.
     * \param [in] CVT the Centroidal Voronoi Tesselation
     *   used to sample the second shape
     * \param [in] OTM the Optimal Transport Map
     * \param [in] filenalme where to store the morphing shape
     *  (Graphite .eobj file format)
     * \param [in] volumetric if true, internal triangles are also saved
     */
    void compute_morph(
        CentroidalVoronoiTesselation& CVT,
        OptimalTransportMap& OTM,
        const std::string& filename,
        bool volumetric = true
    ) {
        std::ofstream out(filename.c_str());

        std::set<trindex> A,A1,A2;
        get_RDT(CVT.RVD(), A1, volumetric, true);
        get_RDT(OTM.RVD(), A2, true, false);
        std::set_intersection(
            A1.begin(), A1.end(), A2.begin(), A2.end(),
            std::inserter(A, A.begin())
        );

        std::set<trindex> B,B1,B2;
        get_RDT(OTM.RVD(), B1, volumetric, true);
        get_RDT(CVT.RVD(), B2, true, false);
        std::set_intersection(
            B1.begin(), B1.end(), B2.begin(), B2.end(),
            std::inserter(B, B.begin())
        );

        std::set<trindex> AUB;
        std::set_union(
            A.begin(), A.end(), B.begin(), B.end(),
            std::inserter(AUB, AUB.begin())
        );

        index_t nb_vertices = OTM.RVD()->delaunay()->nb_vertices();
        vector<vec3> M1_vertices(nb_vertices);
        vector<vec3> M2_vertices(nb_vertices);
        vector<index_t> nb_cnx_comps(nb_vertices, 0);
        vector<index_t> nb_cnx_comps2(nb_vertices, 0);

        {
            for(index_t v = 0; v < nb_vertices; ++v) {
                const double* p = CVT.RVD()->delaunay()->vertex_ptr(v);
                M1_vertices[v] = vec3(p[0], p[1], p[2]);
            }
        }

        {
            vector<vec3> mg(nb_vertices, vec3(0.0, 0.0, 0.0));
            vector<double> m(nb_vertices, 0.0);

            Mesh RVD;
            MeshIOFlags flags;
            flags.set_element(MESH_CELLS);
            flags.set_attribute(MESH_CELL_REGION);
            OTM.RVD()->compute_RVD(
                RVD,
                0,     // dim (0 means use default)
                false, // cells_borders_only
                true   // integration_simplices
            );
            RVD.set_dimension(3);
            RVD.connect_tets();
            if(CmdLine::get_arg_bool("RVD")) {
                mesh_save(RVD, "RVD.meshb", flags);
            }

            for(index_t t = 0; t < RVD.nb_tets(); ++t) {
                index_t v =  index_t(RVD.tet_region(t));
                index_t v0 = RVD.tet_vertex_index(t, 0);
                index_t v1 = RVD.tet_vertex_index(t, 1);
                index_t v2 = RVD.tet_vertex_index(t, 2);
                index_t v3 = RVD.tet_vertex_index(t, 3);
                vec3 p0(RVD.vertex_ptr(v0));
                vec3 p1(RVD.vertex_ptr(v1));
                vec3 p2(RVD.vertex_ptr(v2));
                vec3 p3(RVD.vertex_ptr(v3));
                double mt = Geom::tetra_signed_volume(p0, p1, p2, p3);
                mg[v] += (mt / 4.0) * (p0 + p1 + p2 + p3);
                m[v] += mt;
            }
            for(index_t v = 0; v < nb_vertices; ++v) {
                double s = ::fabs(m[v]);
                if(s != 0.0) {
                    s = 1.0 / s;
                }
                M2_vertices[v] = s * mg[v];
            }
            get_nb_connected_components(RVD, nb_cnx_comps);
        }

        out << "# attribute geom2 vertex vec3" << std::endl;
        out << "# attribute potential vertex real" << std::endl;

        index_t cur = 1;
        for(
            std::set<trindex>::iterator it = AUB.begin();
            it != AUB.end(); ++it
        ) {
            const trindex& T = *it;
            
            if( 
                nb_cnx_comps[T.indices[0]] != 1 ||
                nb_cnx_comps[T.indices[1]] != 1 ||
                nb_cnx_comps[T.indices[2]] != 1 
            ) {
                continue;
            }

            const vec3& p1 = M1_vertices[T.indices[0]];
            const vec3& p2 = M1_vertices[T.indices[1]];
            const vec3& p3 = M1_vertices[T.indices[2]];
            const vec3& q1 = M2_vertices[T.indices[0]];
            const vec3& q2 = M2_vertices[T.indices[1]];
            const vec3& q3 = M2_vertices[T.indices[2]];
            double phi1 = OTM.potential(T.indices[0]);
            double phi2 = OTM.potential(T.indices[1]);
            double phi3 = OTM.potential(T.indices[2]);
            out << "v " << p1 << std::endl;
            out << "v " << p2 << std::endl;
            out << "v " << p3 << std::endl;
            out << "# attrs v "
                << cur << " " << q1 << " " << phi1 << std::endl;
            out << "# attrs v "
                << cur + 1 << " " << q2 << " " << phi2 << std::endl;
            out << "# attrs v "
                << cur + 2 << " " << q3 << " " << phi3 << std::endl;
            out << "f " << cur + 0 << " " << cur + 1 << " " << cur + 2
                << std::endl;
            cur += 3;
        }
    }

    /**
     * \brief Computes the surface that corresponds to discontinuities
     *  in the optimal transport map.
     * \details The surface is determined as the facets of Voronoi cells
     *  that are adjacent in Pow(X)|M1 but not in Vor(X)|M2 
     * \param [in] CVT the Centroidal Voronoi Tesselation
     *   used to sample the second shape M2
     * \param [in] OTM the Optimal Transport Map with the
     *   power diagram that samples the first shape M1
     * \param [in] filename where to store the singular surface
     *  (Graphite .obj file format)
     */
    void compute_singular_surface(        
        CentroidalVoronoiTesselation& CVT,
        OptimalTransportMap& OTM,
        const std::string& filename
    ) {
        std::ofstream out(filename.c_str());
        if(!out) {
            Logger::err("Singular") 
                << filename << ":could not create file" 
                << std::endl;
            return;
        } else {
            Logger::out("Singular") 
                << "saving singular surface to:" << filename 
                << std::endl;
        }

        std::set<bindex> edges;
        {
            vector<index_t> simplices;
            vector<double> embedding;
            CVT.RVD()->compute_RDT(
                simplices, 
                embedding, 
                RestrictedVoronoiDiagram::RDT_SEEDS_ALWAYS
            );
            for(index_t t=0; t*4<simplices.size(); ++t) {
                index_t v1 = simplices[t*4];
                index_t v2 = simplices[t*4+1];
                index_t v3 = simplices[t*4+2];
                index_t v4 = simplices[t*4+3];
                edges.insert(bindex(v1,v2));
                edges.insert(bindex(v1,v3));
                edges.insert(bindex(v1,v4));
                edges.insert(bindex(v2,v3));
                edges.insert(bindex(v2,v4));
                edges.insert(bindex(v3,v4));
            }
        }

        Mesh RVD;
        MeshIOFlags flags;
        flags.set_element(MESH_CELLS);
        flags.set_attribute(MESH_CELL_REGION);
        OTM.RVD()->compute_RVD(
            RVD,
            0,
            false, // cells_borders_only
            true   // integration_simplices
        );
        RVD.set_dimension(3);
        RVD.connect_tets();

        Mesh singular;
        vector<index_t> triangles;

        for(index_t t=0; t<RVD.nb_tets(); ++t) {
            index_t v1 = index_t(RVD.tet_region(t));
            for(index_t f=0; f<4; ++f) {
                signed_index_t nt = RVD.tet_adjacent(t,f);
                if(nt != -1) {
                    index_t v2 = index_t(RVD.tet_region(index_t(nt)));
                    if(v1 != v2 && edges.find(bindex(v1,v2)) == edges.end()) {
                        for(index_t i=0; i<3; ++i) {
                            index_t lv=RVD.local_tet_facet_vertex_index(f,i);
                            index_t v = RVD.tet_vertex_index(t,lv);
                            triangles.push_back(v);
                        }
                    }
                }
            }
        }

        // Copy RVD vertices
        vector<double> vertices = MeshMutator::vertices(RVD);
        singular.assign_triangle_mesh(
            RVD.dimension(),vertices,triangles,
            true // steal args
        );


        mesh_repair(singular);

        mesh_save(singular,filename);
        
    }

    /**
     * \brief Internal implementation function for
     *   compute_hierarchical_sampling().
     * \param[in,out] CVT the CentroidalVoronoiTesselation, initialized
     *  with the volume to be sampled. On output, it stores the samples
     * \param[in] nb_samples total number of samples to generate
     * \param[out] levels sample indices that correspond to level l are
     *   in the range levels[l] (included) ... levels[l+1] (excluded)
     * \param[in] ratio number of samples ratio between two consecutive
     *   levels
     * \param[in] threshold minimum number of samples in a level
     * \param[in] b first element of the level to be generated
     * \param[in] e one position past the last element of the
     *  level to be generated
     * \param[in,out] points work vector allocated by caller,
     *  of size 3*nb_samples
     */
    void compute_hierarchical_sampling_recursive(
        CentroidalVoronoiTesselation& CVT,
        index_t nb_samples,
        vector<index_t>& levels,
        double ratio,
        index_t threshold,
        index_t b, index_t e,
        vector<double>& points
    ) {
        index_t m = b;

        // Recurse in [b...m) range
        if(e - b > threshold) {
            m = b + index_t(double(e - b) * ratio);
            compute_hierarchical_sampling_recursive(
                CVT, nb_samples, levels, ratio, threshold, b, m, points
            );
        }

        // Initialize random points in [m...e) range
        CVT.RVD()->compute_initial_sampling(&points[3 * m], e - m);

        //  Set the points in [b...e) range
        CVT.set_points(e - b, &points[0]);

        // Lock [b...m) range
        if(CmdLine::get_arg_bool("lock")) {
            for(index_t i = b; i < e; ++i) {
                if(i < m) {
                    CVT.lock_point(i);
                } else {
                    CVT.unlock_point(i);
                }
            }
        }

        Logger::div(
            std::string("Generating level ") +
            String::to_string(levels.size())
        );

        Logger::out("Sample") << " generating a level with " << e - m
            << " samples" << std::endl;

        try {
            ProgressTask progress("Lloyd", 100);
            CVT.set_progress_logger(&progress);
            CVT.Lloyd_iterations(CmdLine::get_arg_uint("opt:nb_Lloyd_iter"));
        }
        catch(const TaskCanceled&) {
        }

        try {
            ProgressTask progress("Newton", 100);
            CVT.set_progress_logger(&progress);
            CVT.Newton_iterations(CmdLine::get_arg_uint("opt:nb_Newton_iter"));
        }
        catch(const TaskCanceled&) {
        }

        levels.push_back(e);
    }

    /**
     * \brief Computes a hierarchical sampling of a volume.
     * \param[in,out] CVT the CentroidalVoronoiTesselation, initialized
     *  with the volume to be sampled. On output, it stores the samples
     * \param[in] nb_samples total number of samples to generate
     * \param[out] levels sample indices that correspond to level l are
     *   in the range levels[l] (included) ... levels[l+1] (excluded)
     * \param[in] ratio number of samples ratio between two consecutive
     *   levels
     * \param[in] threshold minimum number of samples in a level
     */
    void compute_hierarchical_sampling(
        CentroidalVoronoiTesselation& CVT,
        index_t nb_samples,
        vector<index_t>& levels,
        double ratio = 0.125,
        index_t threshold = 300
    ) {
        levels.push_back(0);
        vector<double> points(nb_samples * 3);
        compute_hierarchical_sampling_recursive(
            CVT, nb_samples, levels, ratio, threshold,
            0, nb_samples,
            points
        );
        CVT.unlock_all_points();
    }

    /**
     * \brief Computes a sampling of a volume.
     * \param[in,out] CVT the CentroidalVoronoiTesselation, initialized
     *  with the volume to be sampled. On output, it stores the samples
     * \param[in] nb_samples total number of samples to generate
     */
    void compute_single_level_sampling(
        CentroidalVoronoiTesselation& CVT,
        index_t nb_samples
    ) {

        CVT.compute_initial_sampling(nb_samples);

        try {
            ProgressTask progress("Lloyd", 100);
            CVT.set_progress_logger(&progress);
            CVT.Lloyd_iterations(CmdLine::get_arg_uint("opt:nb_Lloyd_iter"));
        }
        catch(const TaskCanceled&) {
        }

        try {
            ProgressTask progress("Newton", 100);
            CVT.set_progress_logger(&progress);
            CVT.Newton_iterations(CmdLine::get_arg_uint("opt:nb_Newton_iter"));
        }
        catch(const TaskCanceled&) {
        }
    }

    /**
     * \brief Projects the points of a volumetric sampling
     *  onto the border of the volume.
     */
    void project_sampling_on_border(
        CentroidalVoronoiTesselation& CVT
    ) {
        try {
            ProgressTask progress("Surf. Lloyd", 100);
            CVT.set_progress_logger(&progress);
            CVT.set_volumetric(false);
            CVT.Lloyd_iterations(
                CmdLine::get_arg_uint("opt:nb_Lloyd_iter") * 2
            );
        }
        catch(const TaskCanceled&) {
        }

        if(CmdLine::get_arg_bool("feature_sensitive")) {

  // Note: deactivated for now, since LpCVT is in vorpaline
  // and not in geogram...
  /*            
            try {
                ProgressTask progress("LpCVT", 100);
                CVT.set_progress_logger(&progress);
                CVT.set_normal_anisotropy(5.0);
                CVT.Newton_iterations(30, 7);
            }
            catch(const TaskCanceled&) {
            }
            CVT.set_normal_anisotropy(1.0);
  */
        }

        vector<double> mg(3 * CVT.nb_points());
        vector<double> m(CVT.nb_points());
        CVT.RVD()->compute_centroids(&mg[0], &m[0]);
        for(index_t i = 0; i < CVT.nb_points(); ++i) {
            if(m[i] == 0.0) {
                CVT.unlock_point(i);
            } else {
                CVT.lock_point(i);
            }
        }

        CVT.set_volumetric(true);

        try {
            ProgressTask progress("Relax. vol.", 100);
            CVT.set_progress_logger(&progress);
            CVT.Lloyd_iterations(
                CmdLine::get_arg_uint("opt:nb_Lloyd_iter") * 2
            );
        }
        catch(const TaskCanceled&) {
        }
    }

    /**
     * \brief Reorders the points in a Centroidal Voronoi Tesselation
     *  in such a way that continguous index ranges correspond to
     *  multiple resolutions.
     * \param[in,out] CVT the CentroidalVoronoiTesselation
     * \param[out] levels sample indices that correspond to level l are
     *   in the range levels[l] (included) ... levels[l+1] (excluded)
     * \param[in] ratio number of samples ratio between two consecutive
     *   levels
     * \param[in] threshold minimum number of samples in a level
     */
    void BRIO_reorder(
        CentroidalVoronoiTesselation& CVT,
        vector<index_t>& levels,
        double ratio,
        index_t threshold
    ) {
        vector<index_t> sorted_indices;
        compute_BRIO_order(
            CVT.nb_points(), CVT.embedding(0), sorted_indices,
            CVT.dimension(), threshold, ratio, &levels
        );
        Permutation::apply(
            CVT.embedding(0), sorted_indices, 
            index_t(CVT.dimension() * sizeof(double))
        );
    }

    /**
     * \brief Translates a mesh in such a way that its center matches
     *  the center of another mesh.
     * \param[in] M1 a const reference to the reference mesh
     * \param[in,out] M2 a reference to the mesh that will be recentered
     */
    void recenter_mesh(const Mesh& M1, Mesh& M2) {
        double xyzmin1[3];
        double xyzmax1[3];
        double xyzmin2[3];
        double xyzmax2[3];
        double xlat[3];
        get_bbox(M1, xyzmin1, xyzmax1);
        get_bbox(M2, xyzmin2, xyzmax2);
        for(coord_index_t c=0; c<3; ++c) {
            xlat[c] = 0.5* ((xyzmin1[c] + xyzmax1[c]) - (xyzmin2[c] + xyzmax2[c]));
        }
        for(index_t v=0; v<M2.nb_vertices(); ++v) {
            for(coord_index_t c=0; c<3; ++c) {
                M2.vertex_ptr(v)[c] += xlat[c];
            }
        }
    }


    /**
     * \brief Computes the volume of a tetrahedral mesh.
     * \param[in] M a const reference to the mesh
     * \return the volume of the tetrahedra of M
     */
    double mesh_tets_volume(const Mesh& M) {
        double result = 0.0;
        for(index_t t = 0; t < M.nb_tets(); ++t) {
            result += Geom::tetra_volume<3>(
                M.vertex_ptr(M.tet_vertex_index(t, 0)),
                M.vertex_ptr(M.tet_vertex_index(t, 1)),
                M.vertex_ptr(M.tet_vertex_index(t, 2)),
                M.vertex_ptr(M.tet_vertex_index(t, 3))
            );
        }
        return result;
    }

    /**
     * \brief Rescales a mesh in such a way that its total volume
     *  matches the volume of a reference mesh.
     * \param[in] M1 a const reference to the reference mesh
     * \param[in,out] M2 a reference to the mesh that will be rescaled
     */
    void rescale_mesh(const Mesh& M1, Mesh& M2) {
        double xyzmin[3];
        double xyzmax[3];
        get_bbox(M2, xyzmin, xyzmax);
        double s = pow(mesh_tets_volume(M1)/mesh_tets_volume(M2), 1.0/3.0);
        for(unsigned int v=0; v<M2.nb_vertices(); ++v) {
            for(index_t c=0; c<3; ++c) {
                double gc = 0.5*(xyzmin[c]+xyzmax[c]);
                M2.vertex_ptr(v)[c] = gc + s * (M2.vertex_ptr(v)[c] - gc);
            }
        }
    }

}

int main(int argc, char** argv) {
    using namespace GEO;

    GEO::initialize();

    try {
        
        std::vector<std::string> filenames;

        CmdLine::import_arg_group("standard");
        CmdLine::import_arg_group("algo");
        CmdLine::import_arg_group("opt");
        CmdLine::declare_arg("nb_pts", 1000, "number of points");
        CmdLine::declare_arg("nb_iter", 1000, "number of iterations for OTM");
        CmdLine::declare_arg("RDT", false, "save regular triangulation");
        CmdLine::declare_arg("RVD", false, "save restricted Voronoi diagram");
        CmdLine::declare_arg("volumetric", true, "show internal triangles");
        CmdLine::declare_arg("multilevel", true, "use multilevel algorithm");
        CmdLine::declare_arg("BRIO", true, 
                             "use BRIO reordering to compute the levels"
        );
        CmdLine::declare_arg("ratio", 0.125, "ratio between levels");
        CmdLine::declare_arg(
            "epsilon", 0.01, "relative measure error in a cell"
        );
        CmdLine::declare_arg(
            "lock", true, "Lock lower levels when sampling shape"
        );
        CmdLine::declare_arg(
            "fitting_degree", 2, "degree for interpolating weights"
        );
        CmdLine::declare_arg(
            "project", true, "project sampling on border"
        );
        CmdLine::declare_arg(
            "feature_sensitive", true, "attempt to recover hard edges"
        );
        CmdLine::declare_arg(
            "singular", false, "compute and save singular surface"
        );
        CmdLine::set_arg("algo:delaunay", "BPOW");
        CmdLine::declare_arg(
            "recenter", false, "recenter target onto source mesh"
        );
        CmdLine::declare_arg(
            "rescale", false, "rescale target to match source volume"
        );
        
        Logger::div("Fast Wasserstein Distance / WarpDrive");
        for(index_t i=0; i<6; ++i) {
            CmdLine::ui_message(banner[i]);
        }
        
        if(
            !CmdLine::parse(
                argc, argv, filenames, "mesh1 mesh2"
            )
        ) {
            return 1;
        }

        std::string mesh1_filename = filenames[0];
        std::string mesh2_filename = filenames[1];

        Logger::div("Loading data");

        Mesh M1;
        Mesh M2;
        Mesh M2_samples;
        MeshIOFlags flags;
        flags.set_element(MESH_CELLS);
        flags.set_attribute(MESH_CELL_REGION);
        if(!mesh_load(mesh1_filename, M1, flags)) {
            return 1;
        }
        if(!mesh_load(mesh2_filename, M2, flags)) {
            return 1;
        }

        if(CmdLine::get_arg_bool("recenter")) {
            recenter_mesh(M1,M2);
        }

        if(CmdLine::get_arg_bool("rescale")) {
            rescale_mesh(M1,M2);
        }

        if(M1.nb_tets() == 0) {
            Logger::err("Mesh") << "M1 does not have any tetrahedron, exiting"
                << std::endl;
            return 1;
        }

        if(M2.nb_tets() == 0) {
            Logger::err("Mesh") << "M2 does not have any tetrahedron, exiting"
                << std::endl;
            return 1;
        }
        
        Logger::div("Sampling target shape");

        CentroidalVoronoiTesselation CVT(&M2, 0, "NN");
        vector<index_t> levels;
        CVT.set_volumetric(true);

        bool multilevel =
            CmdLine::get_arg_bool("multilevel") || 
            CmdLine::get_arg_bool("BRIO");

        if(multilevel) {
            if(CmdLine::get_arg_bool("BRIO")) {
                compute_single_level_sampling(
                    CVT,
                    CmdLine::get_arg_uint("nb_pts")
                );
                BRIO_reorder(
                    CVT, levels, CmdLine::get_arg_double("ratio"), 300
                );
            } else {
                compute_hierarchical_sampling(
                    CVT,
                    CmdLine::get_arg_uint("nb_pts"),
                    levels,
                    CmdLine::get_arg_double("ratio")
                );
            }
        } else {
            compute_single_level_sampling(
                CVT,
                CmdLine::get_arg_uint("nb_pts")
            );
        }

        if(CmdLine::get_arg_bool("project")) {
            project_sampling_on_border(CVT);
        }

        M2_samples.set_dimension(CVT.dimension());
        MeshMutator::vertices(M2_samples).resize(
            CVT.dimension() * CVT.nb_points()
        );
        MeshMutator::set_nb_vertices(M2_samples, CVT.nb_points());
        M2_samples.update_cached_variables();
        Memory::copy(
            M2_samples.vertex_ptr(0), CVT.embedding(0),
            sizeof(double) * CVT.dimension() * CVT.nb_points()
        );

        Logger::div("Optimal transport");
        // Everything happens in dimension 4 (power diagram is seen
        // as Voronoi diagram in dimension 4), therefore the dimension
        // of M1 needs to be changed as well (even if it is not used).
        M1.set_dimension(4);
        OptimalTransportMap OTM(&M1);
        OTM.set_points(M2_samples.nb_vertices(), M2_samples.vertex_ptr(0));
        OTM.set_epsilon(CmdLine::get_arg_double("epsilon"));
        index_t nb_iter = CmdLine::get_arg_uint("nb_iter");

        {
            Stopwatch W("OTM Total");
            if(multilevel) {
                OTM.optimize_levels(levels, nb_iter);
            } else {
                OTM.optimize(nb_iter);
            }
        }

        Logger::div("Morphing");
        Logger::out("OTM") <<  "Time-coherent triangulation." << std::endl;
        compute_morph(
            CVT, OTM, "morph.eobj",
            CmdLine::get_arg_bool("volumetric")
        );

        if(CmdLine::get_arg_bool("singular")) {
            Logger::out("OTM") << "Computing singular set." << std::endl;
            compute_singular_surface(CVT,OTM,"singular.obj");
        }
    }
    catch(const std::exception& e) {
        std::cerr << "Received an exception: " << e.what() << std::endl;
        return 1;
    }

    Logger::out("") << "Everything OK, Returning status 0" << std::endl;
    return 0;
}

