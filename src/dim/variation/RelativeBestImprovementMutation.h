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

#ifndef _VARIATION_RELATIVEBESTIMPROVEMENTMUTATION_H_
#define _VARIATION_RELATIVEBESTIMPROVEMENTMUTATION_H_

#include "Base.h"

namespace dim
{
    namespace variation
    {

	template<typename EOT>
	class RelativeBestImprovementMutation : public Base<EOT>
	{
	public:
	    RelativeBestImprovementMutation(PartialOp<EOT>& op, IncrementalEval<EOT>& eval, ComparisonOp<EOT>& comp) : _op(op), _eval(eval), _comp(comp) {}

	    /// The class name.
	    virtual std::string className() const { return "RelativeBestImprovementMutation"; }

	    bool operator()(EOT& sol)
	    {
		// select one indice from the initial solution
		size_t i;
		i = eo::rng.random(sol.size());

		// keep a best solution with its best delta
		size_t best_i = i;
		size_t best_j = 0;
		double best_delta = 0;

		for (size_t j = 0; j < sol.size()-1; ++j)
		    {
			if ( i == j ) { continue; }

			// incremental eval
			double delta = _eval(sol, i, j);

			if (_comp(delta, best_delta))
			    {
				best_delta = delta;
				best_i = i;
				best_j = j;
			    }
		    }

		// if the best delta is negative, we apply the operator to the solution with the best indicies
		if ( best_delta < 0 )
		    {
			_op(sol, best_i, best_j);
			sol.fitness( sol.fitness() + best_delta );
			return true;
		    }

		return false;
	    }

	private:
	    PartialOp<EOT>& _op;
	    IncrementalEval<EOT>& _eval;
	    ComparisonOp<EOT>& _comp;
	};

    }
}

#endif /* _VARIATION_RELATIVEBESTIMPROVEMENTMUTATION_H_ */
