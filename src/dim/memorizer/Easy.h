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

#ifndef _MEMORIZER_EASY_H_
#define _MEMORIZER_EASY_H_

#include "Base.h"

namespace dim
{
    namespace memorizer
    {
	namespace sync
	{
	    template <typename EOT>
	    class Easy : public Base<EOT>
	    {
	    public:
		void firstCall(core::Pop<EOT>& pop, core::IslandData<EOT>& /*data*/)
		{
		    for (auto &ind : pop)
			{
			    ind.addIsland(this->rank());
			}
		}

		void operator()(core::Pop<EOT>& pop, core::IslandData<EOT>& /*data*/)
		{
		    for (auto &indi : pop)
			{
			    indi.addFitness();
			    indi.addIsland(this->rank());
			}
		}
	    };
	} // !sync

	namespace async
	{
	    template <typename EOT>
	    class Easy : public Base<EOT>
	    {
	    public:
		void firstCompute(core::Pop<EOT>& pop, core::IslandData<EOT>& /*data*/)
		{
		    for (auto &ind : pop)
			{
			    ind.addIsland(this->rank());
			}
		}

		void compute(core::Pop<EOT>& pop, core::IslandData<EOT>& /*data*/)
		{
		    if (pop.empty()) { return; }

		    for (auto &indi : pop)
			{
			    indi.addFitness();
			    indi.addIsland(this->rank());
			}
		}

		void communicate(core::Pop<EOT>& /*pop*/, core::IslandData<EOT>& /*data*/)
		{
		    // empty
		}
	    };
	} // !async
    } // !memorizer
} // !dim

#endif /* _MEMORIZER_EASY_H_ */
