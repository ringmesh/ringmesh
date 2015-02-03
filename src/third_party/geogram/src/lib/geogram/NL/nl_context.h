/*
 *  Copyright (c) 2004-2010, Bruno Levy
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
 *     levy@loria.fr
 *
 *     ALICE Project
 *     LORIA, INRIA Lorraine, 
 *     Campus Scientifique, BP 239
 *     54506 VANDOEUVRE LES NANCY CEDEX 
 *     FRANCE
 *
 */

#ifndef __NL_CONTEXT__
#define __NL_CONTEXT__

#include "nl_private.h"
#include "nl_matrix.h"

/**
 * \file geogram/NL/nl_context.h
 * \brief Internal OpenNL functions to manipulate contexts.
 */

/******************************************************************************/
/* NLContext data structure */

/**
 * \brief The callback type for user-defined matrix vector product
 *   routines.
 * \details Used to specify matrix vector product and preconditioners,
 *  as in:
 *  - nlSetFunction(NL_FUNC_MATRIX,f)
 *  - nlSetFunction(NL_FUNC_PRECONDITIONER,f)
 */
typedef void(*NLMatrixFunc)(const double* x, double* y);

/**
 * \brief The callback type for solver routines.
 * \details Used by nlSetFunction(NL_FUNC_SOLVER,f)
 */
typedef NLboolean(*NLSolverFunc)();

/**
 * \brief The callback type for displaying progress.
 * \details Used by nlSetFunction(NL_FUNC_PROGRESS,f)
 */
typedef void(*NLProgressFunc)(
    NLuint cur_iter, NLuint max_iter, double cur_err, double max_err
);

/**
 * \brief The structure that describe the variables
 *  of the system
 */
typedef struct {
    /** 
     * \brief The value of the variable.
     */
    NLdouble  value ;
    /** 
     * \brief Each variable can be locked.
     */
    NLboolean locked ;
    /**
     * \brief Each variable that is not locked
     *  has a unique index.
     */
    NLuint    index ;
} NLVariable ;

#define NL_STATE_INITIAL                0
#define NL_STATE_SYSTEM                 1
#define NL_STATE_MATRIX                 2
#define NL_STATE_ROW                    3
#define NL_STATE_MATRIX_CONSTRUCTED     4
#define NL_STATE_SYSTEM_CONSTRUCTED     5
#define NL_STATE_SOLVED                 6

typedef struct {
    /**
     * \brief State of the finite-state automaton.
     * \details Used to check that OpenNL functions
     *  were called in the correct order.
     */
    NLenum           state ;

    /**
     * \brief The array of nb_variables variables.
     */
    NLVariable*      variable ;

    /**
     * \brief The number of not locked variables.
     */
    NLuint           n ;

    /**
     * \brief The sparse matrix of the system.
     */
    NLSparseMatrix   M ;

    /**
     * \brief The coefficients that correspond to the
     *  free variables in the row being built.
     */
    NLRowColumn      af ;

    /**
     * \brief The coefficients that correspond to the
     *  locked variables in the row being built.
     */
    NLRowColumn      al ;

    /**
     * \brief The (constant) vector of locked variables.
     */
    NLRowColumn      xl ;

    /**
     * \brief The vector of free variables, solution of
     *  the system.
     */
    NLdouble*        x ;

    /**
     * \brief The vector of right hand sides.
     */
    NLdouble*        b ;

    /**
     * \brief The right hand side of the row being 
     *  build.
     * \details Specified by nlSetRowParameter(NL_RIGHT_HAND_SIDE, rhs)
     */
    NLdouble         right_hand_side ;

    /**
     * \brief Indicates whether the right hand side
     *  was set in the current row.
     */
    NLboolean        right_hand_side_set ;
    
    /**
     * \brief The scaling coefficient for the row being 
     *  build.
     * \details Specified by nlSetRowParameter(NL_ROW_SCALING, rhs)
     */
    NLdouble         row_scaling ;

    /**
     * \brief The used solver, as a symbolic constant.
     */
    NLenum           solver ;

    /**
     * \brief The used preconditioner, as a symbolic constant.
     */
    NLenum           preconditioner ;

    /**
     * \brief The number of variables.
     */
    NLuint           nb_variables ;

    /**
     * \brief The index of the current row
     */
    NLuint           current_row ;

    /**
     * \brief Indicates whether a least squares system
     *  is constructed.
     */
    NLboolean        least_squares ;

    /**
     * \brief Indicates whether the matrix is symmetric.
     */
    NLboolean        symmetric ;

    /**
     * \brief Maximum number of iterations.
     */
    NLuint           max_iterations ;

    /**
     * \brief Maximum number of inner iterations.
     * \details used by GMRES.
     */
    NLuint           inner_iterations ;

    /**
     * \brief Convergence threshold.
     * \details Iterations are stopped whenever
     *  \$ \| A x - b \| / \| b \| < \mbox{threshold} \$
     */
    NLdouble         threshold ;

    /**
     * \brief Relaxation parameter for the SSOR 
     *  preconditioner.
     */
    NLdouble         omega ;

    /**
     * \brief If true, all the rows are normalized.
     */
    NLboolean        normalize_rows ;

    /**
     * \brief Indicates that M was allocated.
     */
    NLboolean        alloc_M ;

    /**
     * \brief Indicates that af was allocated.
     */
    NLboolean        alloc_af ;

    /**
     * \brief Indicates that al was allocated.
     */
    NLboolean        alloc_al ;

    /**
     * \brief Indicates that xl was allocated.
     */
    NLboolean        alloc_xl ;

    /**
     * \brief Indicates that variables were allocated.
     */
    NLboolean        alloc_variable ;

    /**
     * \brief Indicates that x was allocated.
     */
    NLboolean        alloc_x ;

    /**
     * \brief Indicates that b was allocated.
     */
    NLboolean        alloc_b ;

    /**
     * \brief used number of iterations during latest solve.
     */
    NLuint           used_iterations ;

    /**
     * \brief error obtained after latest solve.
     */
    NLdouble         error ;

    /**
     * \brief elapsed time for latest solve.
     */
    NLdouble         elapsed_time ;

    /**
     * \brief the function pointer for matrix vector product.
     */
    NLMatrixFunc     matrix_vector_prod ;

    /**
     * \brief the function pointer for preconditioner vector product.
     */
    NLMatrixFunc     precond_vector_prod ;

    /**
     * \brief the function pointer for the solver.
     */
    NLSolverFunc     solver_func ;

    /**
     * \brief the function pointer for logging progress.
     */
    NLProgressFunc   progress_func ;

    /**
     * \brief if true, some logging information is 
     *  displayed during solve.
     */
    NLboolean        verbose;

    /**
     * \brief Total number of floating point operations
     *  used during latest solve.
     */
    NLulong          flops;
} NLContextStruct ;

/**
 * \brief Pointer to the current context.
 */
extern NLContextStruct* nlCurrentContext ;

/**
 * \brief Makes sure that the finite state automaton is
 *  in the expected state.
 * \details If expected state and current state differ,
 *  then the program is aborted with an error message.
 * \param[in] state the expected state.
 */
void nlCheckState(NLenum state) ;

/**
 * \brief Implements a transition of the finite state automaton.
 * \details If the current state does not match \p state, then
 *  the program is aborted with an error message. The current 
 *  state is replaced by \p to_state. 
 * \param[in] state the expected current state
 * \param[in] to_state the new state
 */
void nlTransition(NLenum from_state, NLenum to_state) ;

/**
 * \brief Implements the default matrix vector product.
 * \details Uses the sparse matrix stored in the current
 *  context.
 * \param[in] x the constant right hand side (size = nlCurrentContext->m)
 * \param[out] y the result (size = nlCurrentContext->n)
 */
void nlMatrixVectorProd_default(const NLdouble* x, NLdouble* y) ;

/**
 * \brief Implements the default solver
 * \details Calls the right solver according to 
 *  nlCurrentContext->solver.
 * \retval NL_TRUE if solve was successful
 * \retval NL_FALSE otherwise
 */
NLboolean nlDefaultSolver() ;

#endif
