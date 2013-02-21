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
	template <typename T>
	struct DataQueue
	{
	    std::mutex mutex;
	    std::queue<T> dataQueue;
	    std::queue< std::chrono::time_point< std::chrono::system_clock > > timesQueue;
	    std::queue<size_t> idQueue;

	    void push(T newData, size_t id = 0)
	    {
		std::lock_guard<std::mutex> lock(mutex);
		dataQueue.push(newData);
		timesQueue.push(std::chrono::system_clock::now());
		idQueue.push(id);
	    }

	    std::tuple<T, long, size_t> pop()
	    {
		std::lock_guard<std::mutex> lock(mutex);
		auto end = std::chrono::system_clock::now();
		auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>( end - timesQueue.front() ).count();
		std::tuple<T, long, size_t> ret(dataQueue.front(), elapsed, idQueue.front());
		dataQueue.pop();
		timesQueue.pop();
		idQueue.pop();
		return ret;
	    }

	    bool empty()
	    {
		std::lock_guard<std::mutex> lock(mutex);
		return dataQueue.empty();
	    }

	    size_t size()
	    {
		std::lock_guard<std::mutex> lock(mutex);
		return dataQueue.size();
	    }
	};

	template <typename T>
	struct DataQueueVector : public std::vector< DataQueue< T > >, public ParallelContext
	{
	    using ParallelContext::size;

	    DataQueueVector() : std::vector< DataQueue< T > >(size()) {}

	    void push(T newData, size_t id)
	    {
		auto& dataQueue = (*this)[id];
		dataQueue.push(newData, id);
	    }

	    std::tuple<T, long, size_t> pop(size_t id)
	    {
		auto& dataQueue = (*this)[id];
		return dataQueue.pop();
	    }

	    bool empty(size_t id) const
	    {
		auto& dataQueue = (*this)[id];
		return dataQueue.empty();
	    }

	    size_t size(size_t id) const
	    {
		auto& dataQueue = (*this)[id];
		return dataQueue.size();
	    }
	};

	template <typename EOT>
	struct IslandData : public ParallelContext
	{
	    using ParallelContext::size;
	    using Fitness = typename EOT::Fitness;

	    IslandData() : feedbacks(size()), proba(size()) {}

	    std::vector< Fitness > feedbacks = std::vector< Fitness >(size(), 0);
	    std::vector< Fitness > proba = std::vector< Fitness >(size(), 0);

	    DataQueueVector< Fitness > feedbackerSendingQueue;
	    DataQueue< Fitness > feedbackerReceivingQueue;

	    DataQueueVector< EOT > migratorSendingQueue;
	    DataQueue< EOT > migratorReceivingQueue;
	};

    } // !core
} // !dim

#endif /* _CORE_ISLANDDATA_H_ */
