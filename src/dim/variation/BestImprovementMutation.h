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

#ifndef _VARIATION_BESTIMPROVEMENTMUTATION_H_
#define _VARIATION_BESTIMPROVEMENTMUTATION_H_

#include "Base.h"

namespace dim
{
    namespace variation
    {

	template<typename EOT>
	class BestImprovementMutation : public Base<EOT>
	{
	public:
	    BestImprovementMutation(PartialOp<EOT>& op, IncrementalEval<EOT>& eval) : _op(op), _eval(eval) {}

	    /// The class name.
	    virtual std::string className() const { return "BestImprovementMutation"; }

	    bool operator()(EOT& sol)
	    {
		// select two indices from the initial solution
		size_t i, j;
		i = eo::rng.random(sol.size());

		// keep a best solution with its best delta
		unsigned best_i = i;
		unsigned best_j = j;
		typename EOT::Fitness best_delta = 0;

		DO_MEASURE(

			   for (size_t k = 0; k < sol.size()-1; ++k)
			       {
				   do { j = eo::rng.random(sol.size()); } while (i == j);

				   for (size_t l = 0; l < sol.size()-1; ++l)
				       {
					   DO_MEASURE(

						      typename EOT::Fitness delta = _eval(sol, i, j);

						      if (delta <= best_delta)
							  {
							      best_delta = delta;
							      best_i = i;
							      best_j = j;
							  }

						      , this->_measureFiles, "variation_compute_delta" );

					   // increment j in order to apply the operator with the other indexes
					   do { j = (j+1) % sol.size(); } while (i == j);
				       }

				   // increment i in order to apply the operator with the other indexes
				   i = (i+1) % sol.size();
			       }

			   , this->_measureFiles, "variation_total" );

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
	};

    }
}

#endif /* _VARIATION_BESTIMPROVEMENTINVERSIONMUTATION_H_ */
