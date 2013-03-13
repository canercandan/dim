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
#include <atomic>
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

	    std::tuple<T, double, size_t> pop(bool wait = false)
	    {
		// waiting while queue is empty
		if (wait)
		    {
			while ( empty() ) {}
		    }
		else
		    {
			if ( empty() )
			    {
				throw std::runtime_error("The queue is empty.");
			    }
		    }

		std::lock_guard<std::mutex> lock(mutex);
		auto end = std::chrono::system_clock::now();
		auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>( end - timesQueue.front() ).count() / 1000.;
		if (!elapsed) { elapsed = 10e-10; } // temporary solution in order to have a positive number in elapsed value
		std::tuple<T, double, size_t> ret(dataQueue.front(), elapsed, idQueue.front());
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
	struct DataQueueVector : public std::vector< DataQueue< T > >
	{
	    DataQueueVector(size_t size) : std::vector< DataQueue< T > >(size) {}

	    void push(T newData, size_t id)
	    {
		auto& dataQueue = (*this)[id];
		dataQueue.push(newData, id);
	    }

	    std::tuple<T, double, size_t> pop(size_t id, bool wait = false)
	    {
		auto& dataQueue = (*this)[id];
		return dataQueue.pop(wait);
	    }

	    bool empty(size_t id)
	    {
		auto& dataQueue = (*this)[id];
		return dataQueue.empty();
	    }

	    size_t size(size_t id)
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

	    IslandData() : feedbacks(size()), feedbackLastUpdatedTimes(size()), vectorLastUpdatedTime(std::chrono::system_clock::now()), proba(size()), feedbackerSendingQueue(size()), migratorSendingQueue(size()), toContinue(true) {}

	    std::vector< Fitness > feedbacks;
	    std::vector< std::chrono::time_point< std::chrono::system_clock > > feedbackLastUpdatedTimes;
	    std::chrono::time_point< std::chrono::system_clock > vectorLastUpdatedTime;
	    std::vector< Fitness > proba;

	    DataQueueVector< Fitness > feedbackerSendingQueue;
	    DataQueue< Fitness > feedbackerReceivingQueue;

	    DataQueueVector< EOT > migratorSendingQueue;
	    DataQueue< EOT > migratorReceivingQueue;

	    std::atomic<bool> toContinue;
	};

    } // !core
} // !dim

#endif /* _CORE_ISLANDDATA_H_ */
