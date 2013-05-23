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

#if __cplusplus > 199711L
#include <tuple>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <chrono>
#else
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/tuple/tuple_io.hpp>
#include <boost/chrono/chrono_io.hpp>
#include <boost/atomic.hpp>
#endif

#include <boost/thread/barrier.hpp>

#include <vector>
#include <queue>

#include "ParallelContext.h"

#include <boost/utility/identity_type.hpp>

#undef AUTO
#if __cplusplus > 199711L
#define AUTO(TYPE) auto
#else // __cplusplus <= 199711L
#define AUTO(TYPE) TYPE
#endif

namespace dim
{
    namespace core
    {
#if __cplusplus > 199711L
	namespace std_or_boost = std;
#else
	namespace std_or_boost = boost;
#endif

	template <typename T>
	struct DataQueue
	{
	    DataQueue() {}
	    DataQueue(const DataQueue& d) { *this = d; }

	    DataQueue& operator=(const DataQueue& d)
	    {
	    	if ( &d != this )
	    	    {
	    		dataQueue = d.dataQueue;
	    		timesQueue = d.timesQueue;
	    		idQueue = d.idQueue;
	    	    }
		return *this;
	    }

	    virtual ~DataQueue() {}

	    std_or_boost::mutex mutex;
	    std::queue<T> dataQueue;
	    std::queue< std_or_boost::chrono::time_point< std_or_boost::chrono::system_clock > > timesQueue;
	    std::queue<size_t> idQueue;

	    void push(T newData, size_t id = 0)
	    {
		std_or_boost::lock_guard<std_or_boost::mutex> lock(mutex);
		dataQueue.push(newData);
		timesQueue.push(std_or_boost::chrono::system_clock::now());
		idQueue.push(id);
	    }

	    std_or_boost::tuple<T, double, size_t> pop(bool wait = false)
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

		std_or_boost::lock_guard<std_or_boost::mutex> lock(mutex);

		AUTO(typename BOOST_IDENTITY_TYPE((std_or_boost::chrono::time_point<std_or_boost::chrono::system_clock>))) end = std_or_boost::chrono::system_clock::now();

		AUTO(double) elapsed = std_or_boost::chrono::duration_cast<std_or_boost::chrono::microseconds>( end - timesQueue.front() ).count() / 1000.;

		if (!elapsed) { elapsed = 10e-10; } // temporary solution in order to have a positive number in elapsed value
		std_or_boost::tuple<T, double, size_t> ret(dataQueue.front(), elapsed, idQueue.front());
		dataQueue.pop();
		timesQueue.pop();
		idQueue.pop();
		return ret;
	    }

	    bool empty()
	    {
		std_or_boost::lock_guard<std_or_boost::mutex> lock(mutex);
		return dataQueue.empty();
	    }

	    size_t size()
	    {
		std_or_boost::lock_guard<std_or_boost::mutex> lock(mutex);
		return dataQueue.size();
	    }
	};

	template <typename T>
	struct DataQueueVector : public std::vector< DataQueue< T > >
	{
	    DataQueueVector(size_t size) : std::vector< DataQueue< T > >(size) {}

	    void push(T newData, size_t id)
	    {
		AUTO(typename BOOST_IDENTITY_TYPE((DataQueue< T >)))& dataQueue = (*this)[id];
		dataQueue.push(newData, id);
	    }

	    std_or_boost::tuple<T, double, size_t> pop(size_t id, bool wait = false)
	    {
		AUTO(typename BOOST_IDENTITY_TYPE((DataQueue< T >)))& dataQueue = (*this)[id];
		return dataQueue.pop(wait);
	    }

	    bool empty(size_t id)
	    {
		AUTO(typename BOOST_IDENTITY_TYPE((DataQueue< T >)))& dataQueue = (*this)[id];
		return dataQueue.empty();
	    }

	    size_t size(size_t id)
	    {
		AUTO(typename BOOST_IDENTITY_TYPE((DataQueue< T >)))& dataQueue = (*this)[id];
		return dataQueue.size();
	    }

	    size_t size()
	    {
		size_t sum = 0;
		for (size_t i = 0; i < std::vector< DataQueue< T > >::size(); ++i)
		    {
			sum += size(i);
		    }
		return sum;
	    }
	};

	template <typename EOT>
	struct IslandData : public ParallelContext
	{
	    using ParallelContext::size;

#if __cplusplus > 199711L
	    using Fitness = typename EOT::Fitness;
#else
	    typedef typename EOT::Fitness Fitness;
#endif

	    IslandData(int __size = -1, int __rank = -1)
		: ParallelContext(0, __size, __rank),
		  feedbacks(size()),
		  feedbackLastUpdatedTimes(size(), std_or_boost::chrono::system_clock::now()),
		  vectorLastUpdatedTime(std_or_boost::chrono::system_clock::now()),
		  proba(size(), 0),
		  feedbackerSendingQueue(size()),
		  migratorSendingQueue(size()),
		  toContinue(true),
		  bar(size())
	    {}

	    IslandData(const IslandData& d)
		: ParallelContext(0, d.size(), d.rank()),
		  feedbackerSendingQueue(size()),
		  migratorSendingQueue(size()),
		  toContinue(true),
		  bar(size())
	    {
		*this = d;
	    }

	    IslandData& operator=(const IslandData& d)
	    {
	    	if ( &d != this )
	    	    {
			size(d.size());
			rank(d.rank());
			feedbacks = d.feedbacks;
			feedbackLastUpdatedTimes = d.feedbackLastUpdatedTimes;
			vectorLastUpdatedTime = d.vectorLastUpdatedTime;
			proba = d.proba;
			feedbackerSendingQueue = d.feedbackerSendingQueue;
			feedbackerReceivingQueue = d.feedbackerReceivingQueue;
			migratorSendingQueue = d.migratorSendingQueue;
			migratorReceivingQueue = d.migratorReceivingQueue;
		    }
		return *this;
	    }

	    virtual ~IslandData() {}

	    std::vector< Fitness > feedbacks;
	    std::vector< std_or_boost::chrono::time_point< std_or_boost::chrono::system_clock > > feedbackLastUpdatedTimes;
	    std_or_boost::chrono::time_point< std_or_boost::chrono::system_clock > vectorLastUpdatedTime;
	    std::vector< unsigned > proba;

	    DataQueueVector< Fitness > feedbackerSendingQueue;
	    DataQueue< Fitness > feedbackerReceivingQueue;

	    DataQueueVector< EOT > migratorSendingQueue;
	    DataQueue< EOT > migratorReceivingQueue;

	    std_or_boost::atomic<bool> toContinue;
	    // std_or_boost::condition_variable cv;
	    // std_or_boost::mutex cv_m;
	    boost::barrier bar;
	};

    } // !core
} // !dim

#endif /* _CORE_ISLANDDATA_H_ */
