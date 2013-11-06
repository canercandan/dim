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
		// keep a best solution with its best delta
		size_t best_i = 0;
		size_t best_j = 0;
		typename EOT::Fitness best_delta = 0;

		DO_MEASURE(

			   for (size_t i = 0; i < sol.size()-1; ++i)
			       {
				   for (size_t j = 0; j < sol.size()-1; ++j)
				       {
					   if (i == j) { continue; }

					   DO_MEASURE(

						      typename EOT::Fitness delta = _eval(sol, i, j);

						      if (delta <= best_delta)
							  {
							      best_delta = delta;
							      best_i = i;
							      best_j = j;
							  }

						      , this->_measureFiles, "variation_compute_delta" );
				       }
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
