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

#include <tuple>
#include <vector>
#include <queue>
#include <mutex>
#include <chrono>

#include "ParallelContext.h"

namespace dim
{
    namespace core
    {

	template <typename EOT>
	struct IslandData : public ParallelContext
	{
	    using ParallelContext::size;

	    using Fitness = typename EOT::Fitness;
	    using Mutex = std::mutex;
	    using Time = std::chrono::time_point<std::chrono::system_clock>;

	    template <typename T> using Vector = std::vector< T >;
	    template <typename T> using Queue = std::queue< T >;
	    template <typename T, typename U> using Data = std::tuple< Mutex, T, U >;
	    template <typename T> using VDQ = Vector< Data< Queue<T>, Queue<Time> > >;

	    Vector<Fitness> feedbacks = Vector<Fitness>(size(), 0);
	    Vector<Fitness> proba = Vector<Fitness>(size(), 0);

	    VDQ<Fitness> feedbackerSendingQueuesVector = VDQ<Fitness>( size() );
	    VDQ<Fitness> feedbackerReceivingQueuesVector = VDQ<Fitness>( size() );

	    VDQ<EOT> migratorSendingQueuesVector = VDQ<EOT>( size() );
	    VDQ<EOT> migratorReceivingQueuesVector = VDQ<EOT>( size() );
	};

    } // !core
} // !dim

#endif /* _CORE_ISLANDDATA_H_ */
