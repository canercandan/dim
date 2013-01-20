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

#ifndef _ALGO_EASYLS_H_
#define _ALGO_EASYLS_H_

#include <dim/variation/OpContainer.h>
#include <dim/core/Populator.h>

namespace dim
{
    namespace algo
    {

	template <class EOT>
	class EasyLS: public variation::SequentialOp<EOT>
	{
	public:
	    EasyLS(unsigned nbIter = 100) : _nbIter(nbIter) {}

	    /// The class name.
	    virtual std::string className() const { return "EasyLS"; }

	    void apply(core::Populator<EOT>& pop)
	    {
		for ( unsigned i = 0; i < _nbIter; ++i )
		    {
			variation::SequentialOp<EOT>::apply( pop );
		    }
	    }

	private:
	    unsigned _nbIter;
	};

    } // !algo
} // !dim

#endif // !_ALGO_EASYLS_H_
