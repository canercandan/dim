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

#ifndef _CORE_ISLANDDATA_H_
#define _CORE_ISLANDDATA_H_

#include <vector>
#include "ParallelContext.h"

namespace dim
{
    namespace core
    {

	template <typename EOT>
	struct IslandData : public ParallelContext
	{
	    // IslandData() : feedbacks(ParallelContext::size(), 0),
	    // 		   proba(ParallelContext::size(), 0),
	    // 		   probaret(ParallelContext::size(), 0)
	    // {}

	    std::vector< typename EOT::Fitness > feedbacks = std::vector< typename EOT::Fitness >(ParallelContext::size(), 0);
	    std::vector< typename EOT::Fitness > proba = std::vector< typename EOT::Fitness >(ParallelContext::size(), 0);
	    std::vector< typename EOT::Fitness > probaret = std::vector< typename EOT::Fitness >(ParallelContext::size(), 0);
	};

    } // !core
} // !dim

#endif /* _CORE_ISLANDDATA_H_ */
