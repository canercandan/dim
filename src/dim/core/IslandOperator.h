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

#ifndef _CORE_ISLANDOPERATOR_H_
#define _CORE_ISLANDOPERATOR_H_

#include "ParallelContext.h"
#include "BF.h"
#include "Pop.h"
#include "IslandData.h"

namespace dim
{
    namespace core
    {
	namespace sync
	{
	    template <typename EOT>
	    class IslandOperator : public BF<Pop<EOT>&, IslandData<EOT>&>, public ParallelContext
	    {
	    public:
		IslandOperator(size_t tag = 0) : ParallelContext(tag) {}

		virtual void firstCall(Pop<EOT>&, IslandData<EOT>&) = 0;
		virtual void lastCall(Pop<EOT>&, IslandData<EOT>&) = 0;
	    };
	} // !sync

	namespace async
	{
	    template <typename EOT>
	    class IslandOperator : public ParallelContext
	    {
	    public:
		IslandOperator(size_t tag = 0) : ParallelContext(tag) {}

		virtual void firstCompute(Pop<EOT>&, IslandData<EOT>&) = 0;
		virtual void lastCompute(Pop<EOT>&, IslandData<EOT>&) = 0;

		virtual void firstCommunicate(Pop<EOT>&, IslandData<EOT>&) = 0;
		virtual void lastCommunicate(Pop<EOT>&, IslandData<EOT>&) = 0;

		virtual void compute(Pop<EOT>&, IslandData<EOT>&) = 0;
		virtual void communicate(Pop<EOT>&, IslandData<EOT>&) = 0;
	    };
	} // !async
    } // !core
} // !dim

#endif /* _CORE_ISLANDOPERATOR_H_ */
