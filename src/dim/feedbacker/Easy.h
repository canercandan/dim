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

#ifndef _FEEDBACKER_EASY_H_
#define _FEEDBACKER_EASY_H_

#include <vector>

#include "Base.h"

namespace dim
{
    namespace feedbacker
    {
	template <typename EOT>
	class Easy : public Base<EOT>
	{
	public:
	    virtual void firstCall(core::Pop<EOT>& /*pop*/, core::IslandData<EOT>& data)
	    {
		_reqs.resize( this->size() );

		for (size_t i = 0; i < this->size(); ++i)
		    {
			if (i == this->rank()) { continue; }
			_reqs[i] = this->world().send_init( i, this->tag(), data.feedbacks[i] );
		    }
	    }

	    void operator()(core::Pop<EOT>& pop, core::IslandData<EOT>& data)
	    {
		/************************************************
		 * Send feedbacks back to all islands (ANALYSE) *
		 ************************************************/

		std::vector<typename EOT::Fitness> sums(this->size(), 0);
		std::vector<int> nbs(this->size(), 0);
		for (auto &ind : pop)
		    {
			sums[ind.getLastIsland()] += ind.fitness() - ind.getLastFitness();
			++nbs[ind.getLastIsland()];
		    }

		for (size_t i = 0; i < this->size(); ++i)
		    {
			if (i == this->rank()) { continue; }
			data.feedbacks[i] = nbs[i] > 0 ? sums[i] / nbs[i] : 0;
			this->world().start( _reqs[i] );
		    }

		// for island itself because of the MPI communication optimizing.
		data.feedbacks[this->rank()] = nbs[this->rank()] > 0 ? sums[this->rank()] / nbs[this->rank()] : 0;

		/**************************************
		 * Receive feedbacks from all islands *
		 **************************************/

		std::vector< boost::mpi::request > recv_reqs;

		for (size_t i = 0; i < this->size(); ++i)
		    {
			if (i == this->rank()) { continue; }
			recv_reqs.push_back( this->world().irecv( i, this->tag(), data.feedbacks[i] ) );
		    }

		/****************************
		 * Process all MPI requests *
		 ****************************/

		boost::mpi::wait_all( _reqs.begin(), _reqs.end() );
		boost::mpi::wait_all( recv_reqs.begin(), recv_reqs.end() );
	    }

	private:
	    std::vector< boost::mpi::request > _reqs;
	};
    }
}

#endif /* _FEEDBACKER_EASY_H_ */
