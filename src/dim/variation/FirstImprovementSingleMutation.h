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

#ifndef _VARIATION_FIRSTIMPROVEMENTSINGLEMUTATION_H_
#define _VARIATION_FIRSTIMPROVEMENTSINGLEMUTATION_H_

#include "Base.h"

namespace dim
{
    namespace variation
    {

	template<typename EOT>
	    class FirstImprovementSingleMutation : public Base<EOT>
	    {
	    public:
	    FirstImprovementSingleMutation(PartialOp<EOT>& op, IncrementalEval<EOT>& eval) : _op(op), _eval(eval) {}

		/// The class name.
		virtual std::string className() const { return "FirstImprovementSingleMutation"; }

		bool operator()(EOT& sol)
		{
		    std::vector< std::pair< size_t, size_t > > selected;

		    // check for duplicate
		    for (size_t k = 0; k < sol.size()-1; ++k)
			{
			    size_t i, j;
			    do
				{
				    // generate two different indices
				    i=eo::rng.random(sol.size());
				    do { j = eo::rng.random(sol.size()); } while (i == j);
				}
			    while ( i == j || find( selected.begin(), selected.end(), std::make_pair(i,j) ) != selected.end() );
			    selected.push_back(std::make_pair(i,j));
			}

		    for (size_t k = 0; k < sol.size()-1; ++k)
			{
			    size_t i = selected[k].first;
			    size_t j = selected[k].second;

			    // incremental eval
			    typename EOT::Fitness delta = _eval(sol, i, j);

			    if (delta <= 0)
				{
				    _op(sol, i, j);
				    sol.fitness( sol.fitness() + delta );
				    return true;
				}
			}
		    return false;
		}

	    private:
		PartialOp<EOT>& _op;
		IncrementalEval<EOT>& _eval;
	    };

    }
}

#endif /* _VARIATION_FIRSTIMPROVEMENTSINGLEMUTATION_H_ */
