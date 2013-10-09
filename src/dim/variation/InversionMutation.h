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

#ifndef _VARIATION_INVERSIONMUTATION_H_
#define _VARIATION_INVERSIONMUTATION_H_

#include "Base.h"

namespace dim
{
    namespace variation
    {

	template<typename EOT>
	class InversionMutation : public Base<EOT>
	{
	public:
	    /// The class name.
	    virtual std::string className() const { return "InversionMutation"; }

	    /**
	     * Inversion two components of the given chromosome.
	     * @param chrom The cromosome which is going to be changed.
	     */
	    bool operator()(EOT& sol)
	    {
		unsigned i, j;

		// generate two different indices
		i=eo::rng.random(sol.size());
		do j = eo::rng.random(sol.size()); while (i == j);

		inversion(sol, i, j);

		return true;
	    }
	};

    }
}

#endif /* _VARIATION_INVERSIONMUTATION_H_ */
