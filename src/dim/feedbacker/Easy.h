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
#include <queue>

#include "Base.h"

namespace dim
{
    namespace feedbacker
    {
	namespace sync
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
	} // !sync

	namespace async
	{
	    template <typename EOT>
	    class Easy : public Base<EOT>
	    {
	    public:
		~Easy()
		{
		    for (auto& sender : _senders) { delete sender; }
		    for (auto& receiver : _receivers) { delete receiver; }
		}

		virtual void firstCall(core::Pop<EOT>& /*pop*/, core::IslandData<EOT>& /*data*/)
		{
		    std::ostringstream ss;
		    ss << "trace.feedbacker." << this->rank() << ".algo.txt";
		    _of_algo.open(ss.str());

		    std::ostringstream ss_data;
		    ss_data << "trace.feedbacker." << this->rank() << ".algo.data.txt";
		    _of_algo_data.open(ss_data.str());
		}

		void operator()(core::Pop<EOT>& pop, core::IslandData<EOT>& data)
		{
		    /************************************************
		     * Send feedbacks back to all islands (ANALYSE) *
		     ************************************************/

		    // _of_algo_data << pop.size() << " "; _of_algo_data.flush();

		    for (auto& ind : pop)
		    	{
		    	    auto delta = ind.fitness() - ind.getLastFitness();

		    	    if ( ind.getLastIsland() == static_cast<int>( this->rank() ) )
		    		{
		    		    data.feedbackerReceivingQueue.push( delta, this->rank() );
		    		    continue;
		    		}

		    	    data.feedbackerSendingQueue.push( delta, ind.getLastIsland() );
		    	}

		    /********************
		     * Update feedbacks *
		     ********************/

		    while ( !data.feedbackerReceivingQueue.empty() )
		    	{
		    	    // waiting for a new fitness
		    	    auto fbr = data.feedbackerReceivingQueue.pop();
		    	    auto Fi = std::get<0>(fbr);
		    	    auto Ti = std::get<1>(fbr);
		    	    auto from = std::get<2>(fbr);
		    	    auto& Ri = data.feedbacks[from];

			    // _of_algo_data << Ti << " "; _of_algo_data.flush();

		    	    if (Ti)
		    		{
		    		    Ri = Ri + 0.01 * ( Fi/Ti - Ri );
		    		    _of_algo_data << Fi << "/" << Ti << "=" << Fi/Ti << " "; _of_algo_data.flush();
		    		}
		    	    else
		    		{
		    		    Ri = Ri + 0.01 * ( Fi - Ri );
		    		}
		    	}
		}

		class Sender : public core::Thread< core::Pop<EOT>&, core::IslandData<EOT>& >, public core::ParallelContext
		{
		public:
		    Sender(size_t to, size_t tag = 0) : ParallelContext(tag), _to(to) {}

		    void operator()(core::Pop<EOT>& /*pop*/, core::IslandData<EOT>& data)
		    {
			while (data.toContinue)
			    {
				auto fbs = data.feedbackerSendingQueue.pop( _to, true );
				auto fit = std::get<0>( fbs );
				this->world().send(_to, this->size() * ( this->rank() + _to ) + this->tag(), fit);
			    }
		    }

		private:
		    size_t _to;
		};

		class Receiver : public core::Thread< core::Pop<EOT>&, core::IslandData<EOT>& >, public core::ParallelContext
		{
		public:
		    Receiver(size_t from, size_t tag = 0) : ParallelContext(tag), _from(from) {}

		    void operator()(core::Pop<EOT>& /*pop*/, core::IslandData<EOT>& data)
		    {
			while (data.toContinue)
			    {
				typename EOT::Fitness fit;
				this->world().recv(_from, this->size() * ( this->rank() + _from ) + this->tag(), fit);
				data.feedbackerReceivingQueue.push( fit, _from );
			    }
		    }
		private:
		    size_t _from;
		};

		virtual void addTo( core::ThreadsRunner< core::Pop<EOT>&, core::IslandData<EOT>& >& tr )
		{
		    for (size_t i = 0; i < this->size(); ++i)
			{
			    if (i == this->rank()) { continue; }

			    _senders.push_back( new Sender(i, this->tag()) );
			    _receivers.push_back( new Receiver(i, this->tag()) );

			    tr.add( _senders.back() );
			    tr.add( _receivers.back() );
			}
		}

	    private:
		std::vector<Sender*> _senders;
		std::vector<Receiver*> _receivers;
		std::ofstream _of_algo, _of_algo_data;
	    };
	} // !async
    }
}

#endif /* _FEEDBACKER_EASY_H_ */
