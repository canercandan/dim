// -*- mode: c++; c-indent-level: 4; c++-member-init-indent: 8; comment-column: 35; -*-

/* This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * Authors:
 * Caner Candan <caner.candan@univ-angers.fr>
 */

#ifndef _EVALUATION_ONEMAX_H_
#define _EVALUATION_ONEMAX_H_

#include <eoEvalFunc.h>

namespace dim
{
    namespace evaluation
    {

	template< class EOT >
	class OneMax : public eoEvalFunc<EOT>
	{
	public:
	    /**
	     * Count the number of 1 in a bitString
	     * @param _sol the solution to evaluate
	     */
	    void operator() (EOT& _sol)
	    {
		unsigned int sum = 0;

#if __cplusplus > 199711L
		for (auto i : _sol)
#else
		for (size_t i = 0; i < _sol.size(); ++i)
#endif
		    {
#if __cplusplus > 199711L
			sum += i;
#else
			sum += _sol[i];
#endif
		    }
		_sol.fitness(sum);
	    }
	};

    } // !evaluation
} // !dim

#endif /* _EVALUATION_ONEMAX_H_ */
