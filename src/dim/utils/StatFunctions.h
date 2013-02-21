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

#ifndef _UTILS_STATFUNCTIONS_H_
#define _UTILS_STATFUNCTIONS_H_

#include <boost/mpi.hpp>

#include <eo>
#include <string>
#include <vector>

#include <utils/eoLogger.h>

#include "Stat.h"

namespace dim
{
    namespace utils
    {

	template <typename EOT>
	void print_sum( core::Pop<EOT>& pop )
	{
	    boost::mpi::communicator world;
	    int size = pop.size();
	    eo::log << eo::progress << "F" << pop.size() << " "; eo::log.flush();
	    int sum = -1;
	    all_reduce( world, size, sum, std::plus<int>() );
	    if ( 0 == world.rank() )
		{
		    eo::log << eo::progress << "sum: " << sum << std::endl; eo::log.flush();
		}
	}

	template < typename EOT > size_t getPopSize(const core::Pop< EOT >& pop) { return pop.size(); }
	template < typename EOT > size_t getPopInputSize(const core::Pop< EOT >& pop) { return pop.getInputSize(); }
	template < typename EOT > size_t getPopOutputSize(const core::Pop< EOT >& pop) { return pop.getOutputSize(); }

	template < typename EOT >
	class GetMigratorSendingQueueSize : public eoUF<const core::Pop<EOT>&, size_t>
	{
	public:
	    GetMigratorSendingQueueSize(core::IslandData<EOT>& data) : _data(data) {}

	    size_t operator() ( const core::Pop<EOT>& )
	    {
		auto& immPair = _data.migratorSendingQueuesVector[_data.rank()];
		immPair.first.lock();
		size_t res = immPair.second.size();
		immPair.first.unlock();
		return res;
	    }

	private:
	    core::IslandData<EOT>& _data;
	};

	template < typename EOT >
	class GetMigratorReceivingQueueSize : public eoUF<const core::Pop<EOT>&, size_t>
	{
	public:
	    GetMigratorReceivingQueueSize(core::IslandData<EOT>& data) : _data(data) {}

	    size_t operator() ( const core::Pop<EOT>& )
	    {
		return _data.migratorReceivingQueue.size();
	    }

	private:
	    core::IslandData<EOT>& _data;
	};

    } // !utils
} // !dim

#endif /* _UTILS_STATFUNCTIONS_H_ */
