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

#ifndef _VARIATION_RANDMUTATION_H_
#define _VARIATION_RANDMUTATION_H_

#include "Base.h"

namespace dim
{
    namespace variation
    {

	template<typename EOT>
	class RandMutation : public Base<EOT>
	{
	public:
	    RandMutation(PartialOp<EOT>& op) : _op(op) {}

	    /// The class name.
	    virtual std::string className() const { return "RandMutation"; }

	    bool operator()(EOT& sol)
	    {
		unsigned i, j;

		// generate two different indices
		i=eo::rng.random(sol.size());
		do j = eo::rng.random(sol.size()); while (i == j);

		_op(sol, i, j);

		return true;
	    }

	private:
	    PartialOp<EOT>& _op;
	};

    }
}

#endif /* _VARIATION_RANDMUTATION_H_ */
