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

#ifndef _VARIATION_FIRSTIMPROVEMENTSWAPMUTATION_H_
#define _VARIATION_FIRSTIMPROVEMENTSWAPMUTATION_H_

#include <dim/initialization/TSPLibGraph.h>
#include "Base.h"

namespace dim
{
    namespace variation
    {

	template<typename EOT>
	class FirstImprovementSwapMutation : public Base<EOT>
	{
	public:
	    /// The class name.
	    virtual std::string className() const { return "FirstImprovementSwapMutation"; }

	    /**
	     * Swap two components of the given chromosome.
	     * @param chrom The cromosome which is going to be changed.
	     */
	    bool operator()(EOT& sol)
	    {
		for (size_t k = 0; k < sol.size()-1; ++k)
		    {
			unsigned i, j;

			// generate two different indices
			i=eo::rng.random(sol.size());
			do { j = eo::rng.random(sol.size()); } while (i == j);

			// incremental eval
			double delta =
			    - (D(sol, i-1, i) + D(sol, i, i+1) + D(sol, j-1, j) + D(sol, j, j+1))
			    + (D(sol, i-1, j) + D(sol, j, i+1) + D(sol, j-1, i) + D(sol, i, j+1));

			if (delta <= 0)
			    {
				// swap
				std::swap(sol[i], sol[j]);
				sol.fitness( sol.fitness() + delta );
				return true;
			    }
		    }
		return false;
	    }
	};

    }
}

#endif /* _VARIATION_FIRSTIMPROVEMENTSWAPMUTATION_H_ */
