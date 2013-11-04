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

#ifndef _VARIATION_BESTIMPROVEMENTSHIFTMUTATION_H_
#define _VARIATION_BESTIMPROVEMENTSHIFTMUTATION_H_

#include <dim/initialization/TSPLibGraph.h>
#include "Base.h"

namespace dim
{
    namespace variation
    {

	template<typename EOT>
	class BestImprovementShiftMutation : public Base<EOT>
	{
	public:
	    /// The class name.
	    virtual std::string className() const { return "BestImprovementShiftMutation"; }

	    void shift(EOT& _eo, size_t i, size_t j)
	    {
		unsigned from, to;
		typename EOT::AtomType tmp;

		// indexes
		from=std::min(i,j);
		to=std::max(i,j);

		// keep the first component to change
		tmp=_eo[to];

		// shift
		for(unsigned int k=to ; k > from ; k--)
		    {
			_eo[k]=_eo[k-1];
		    }

		// shift the first component
		_eo[from]=tmp;
	    }

	    /**
	     * Shift two components of the given chromosome.
	     * @param chrom The cromosome which is going to be changed.
	     */
	    bool operator()(EOT& sol)
	    {
		// select two indices from the initial solution
		size_t i, j;
		i = eo::rng.random(sol.size());

		// keep a best solution with its best delta
		unsigned best_i = i;
		unsigned best_j = j;
		double best_delta = 0;

		DO_MEASURE(

			   for (size_t k = 0; k < sol.size()-1; ++k)
			       {
				   do { j = eo::rng.random(sol.size()); } while (i == j);

				   for (size_t l = 0; l < sol.size()-1; ++l)
				       {
					   DO_MEASURE(

						      // incremental eval
						      double delta =
						      - (D(sol, j-1, j) + D(sol, j, j+1) + D(sol, i-1, i))
						      + (D(sol, i-1, j) + D(sol, i, j) + D(sol, j-1, j+1));

						      if (delta <= best_delta)
							  {
							      best_delta = delta;
							      best_i = i;
							      best_j = j;
							  }

						      , this->_measureFiles, "variation_compute_delta" );

					   // increment j in order to shift with the other indexes
					   do { j = (j+1) % sol.size(); } while (i == j);
				       }

				   // increment i in order to shift with the other indexes
				   i = (i+1) % sol.size();
			       }

			   , this->_measureFiles, "variation_total" );

		// if the best delta is negative, we shift the solution with the best indicies
		if ( best_delta < 0 )
		    {
			this->shift(sol, best_i, best_j);
			sol.fitness( sol.fitness() + best_delta );
			return true;
		    }

		return false;
	    }
	};

    }
}

#endif /* _VARIATION_BESTIMPROVEMENTSHIFTMUTATION_H_ */
