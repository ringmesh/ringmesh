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

#ifndef __GEOGRAM_BASIC_ALGORITHM__
#define __GEOGRAM_BASIC_ALGORITHM__

#include <geogram/basic/common.h>

#if defined(GEO_OS_LINUX) && defined(GEO_OPENMP)
#include <parallel/algorithm>
#elif defined(GEO_OS_WINDOWS)
#if (_MSC_VER >= 1700)
#include <ppl.h>
#endif
#endif

#include <algorithm>

/**
 * \file geogram/basic/algorithm.h
 * \brief Wrappers around parallel implementation of STL
 */

namespace GEO {

    /**
     * \brief Checks whether parallel algorithms are used.
     * \details Some algorithms such as sort() can be used
     *  in parallel or sequential mode. Behavior is toggled
     *  by the "algo:parallel" environment variable.
     * \retval true if parallel algorithms are used.
     * \retval false if sequential algorithms are used.
     */
    bool GEOGRAM_API uses_parallel_algorithm();

    /**
     * \brief Sorts elements in parallel
     * \details Sorts elements in the iterator range [\p begin..\p end) using
     * a parallel version of the standard \c std::sort() algorithm (if
     * possible). The elements are compared using operator<().
     * Whether to use the parallel or the standard version of the std::sort()
     * algorithm is controlled by the "algo:parallel" environment property.
     * \param[in] begin first element to sort
     * \param[in] end one position past the last element to sort
     * \tparam ITERATOR the type of the iterator
     * \see uses_parallel_algorithm()
     */
    template <typename ITERATOR>
    inline void sort(
        const ITERATOR& begin, const ITERATOR& end
    ) {
        if(uses_parallel_algorithm()) {
#if defined(GEO_OS_LINUX) && defined(GEO_OPENMP)
            __gnu_parallel::sort(begin, end);
#elif defined(GEO_OS_WINDOWS) && (_MSC_VER >= 1700)
            concurrency::parallel_sort(begin, end);
#else
            std::sort(begin, end);
#endif
        } else {
            std::sort(begin, end);
        }
    }

    /**
     * \brief Sorts elements in parallel
     * \details Sorts elements in the iterator range [\p begin..\p end) using
     * a parallel version of the standard \c std::sort() algorithm (if
     * possible). The elements are compared using comparator \p cmp.
     * Comparator \p cmp must implement an operator() with the following
     * signature:
     * \code
     * bool operator(T a, T b) const;
     * \endcode
     * Whether to use the parallel or the standard version of the std::sort()
     * algorithm is controlled by the "algo:parallel" environment property.
     * \param[in] begin first element to sort
     * \param[in] end one position past the last element to sort
     * \param[in] cmp comparison object.
     * \tparam ITERATOR the type of the iterator
     * \tparam CMP the type of the comparator
     * \see uses_parallel_algorithm()
     */
    template <typename ITERATOR, typename CMP>
    inline void sort(
        const ITERATOR& begin, const ITERATOR& end, const CMP& cmp
    ) {
        if(uses_parallel_algorithm()) {
#if defined(GEO_OS_LINUX) && defined(GEO_OPENMP)
            __gnu_parallel::sort(begin, end, cmp);
#elif defined(GEO_OS_WINDOWS) && (_MSC_VER >= 1700)
            concurrency::parallel_sort(begin, end, cmp);
#else
            std::sort(begin, end, cmp);
#endif
        } else {
            std::sort(begin, end, cmp);
        }
    }
}

#endif

