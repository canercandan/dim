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

#ifndef _EVOLVER_EASY_H_
#define _EVOLVER_EASY_H_

#include "Base.h"

namespace dim
{
    namespace evolver
    {
	template <typename EOT>
	class Easy : public Base<EOT>
	{
	public:
	    Easy(eoEvalFunc<EOT>& eval, eoMonOp<EOT>& op) : _eval(eval), _op(op) {}

	    void operator()(core::Pop<EOT>& pop, core::IslandData<EOT>& /*data*/)
	    {
		for (auto &ind : pop)
		    {
			EOT candidate = ind;

			_op( candidate );

			// candidate.invalidate();
			_eval( candidate );

			if ( candidate.fitness() > ind.fitness() )
			    {
				ind = candidate;
			    }
		    }
	    }

	private:
	    eoEvalFunc<EOT>& _eval;
	    eoMonOp<EOT>& _op;
	};
    } // !evolver
} // !dim

#endif /* _EVOLVER_EASY_H_ */
