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

#ifndef _VARIATION_BESTIMPROVEMENTSWAPMUTATION_H_
#define _VARIATION_BESTIMPROVEMENTSWAPMUTATION_H_

#include <dim/initialization/TSPLibGraph.h>
#include "Base.h"
#include <dim/utils/Measure.h>

namespace dim
{
    namespace variation
    {
#if __cplusplus > 199711L
	namespace std_or_boost = std;
#else
	namespace std_or_boost = boost;
#endif

	template<typename EOT>
	class BestImprovementSwapMutation : public Base<EOT>
	{
	public:
	    virtual void firstCall()
	    {
		std::ostringstream ss;

#ifdef MEASURE
		ss.str(""); ss << this->monitorPrefix << ".best_improvement_swap_total.time." << this->rank;
		_measureFiles["best_improvement_swap_total"] = new std::ofstream(ss.str().c_str());

		ss.str(""); ss << this->monitorPrefix << ".best_improvement_swap_compute_delta.time." << this->rank;
		_measureFiles["best_improvement_swap_compute_delta"] = new std::ofstream(ss.str().c_str());
#endif // !MEASURE
	    }

	    /// The class name.
	    virtual std::string className() const { return "BestImprovementSwapMutation"; }

	    /**
	     * Swap two components of the given chromosome.
	     * @param chrom The cromosome which is going to be changed.
	     */
	    bool operator()(EOT& sol)
	    {
		// select two indices from the initial solution
		size_t i;
		size_t j;
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
						      - (D(sol, i-1, i) + D(sol, i, i+1) + D(sol, j-1, j) + D(sol, j, j+1))
						      + (D(sol, i-1, j) + D(sol, j, i+1) + D(sol, j-1, i) + D(sol, i, j+1));

						      if (delta < best_delta)
							  {
							      best_delta = delta;
							      best_i = i;
							      best_j = j;
							  }

						      , _measureFiles, "best_improvement_swap_compute_delta" );

					   // increment j in order to swap with the other indexes
					   do { j = (j+1) % sol.size(); } while (i == j);
				       }

				   // increment i in order to swap with the other indexes
				   i = (i+1) % sol.size();
			       }

			   , _measureFiles, "best_improvement_swap_total" );

		// if the best delta is negative, we swap the solution with the best indicies
		if ( best_delta < 0 )
		    {
			std::swap(sol[best_i], sol[best_j]);
			sol.fitness( sol.fitness() + best_delta );
			return true;
		    }

		return false;
	    }

	private:
#ifdef MEASURE
	    std::map<std::string, std::ofstream*> _measureFiles;
#endif // !MEASURE
	};

    }
}

#endif /* _VARIATION_BESTIMPROVEMENTSWAPMUTATION_H_ */
