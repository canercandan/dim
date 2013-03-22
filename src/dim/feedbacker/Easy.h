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
#if __cplusplus > 199711L
	namespace std_or_boost = std;
#else
	namespace std_or_boost = boost;
#endif

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

#if __cplusplus > 199711L
		    for (auto &ind : pop)
#else
		    for (size_t i = 0; i < pop.size(); ++i)
#endif
			{
#if __cplusplus > 199711L
			    sums[ind.getLastIsland()] += ind.fitness() - ind.getLastFitness();
			    ++nbs[ind.getLastIsland()];
#else
			    sums[pop[i].getLastIsland()] += pop[i].fitness() - pop[i].getLastFitness();
			    ++nbs[pop[i].getLastIsland()];
#endif
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
#if __cplusplus > 199711L
		    for (auto& sender : _senders) { delete sender; }
		    for (auto& receiver : _receivers) { delete receiver; }
#else
		    for (size_t i = 0; i < _senders.size(); ++i) { delete _senders[i]; }
		    for (size_t i = 0; i < _receivers.size(); ++i) { delete _receivers[i]; }
#endif
		}

		virtual void firstCall(core::Pop<EOT>& /*pop*/, core::IslandData<EOT>& /*data*/)
		{
#ifdef TRACE
		    std::ostringstream ss;
		    ss << "trace.feedbacker." << this->rank() << ".algo.txt";
		    _of_algo.open(ss.str());

		    std::ostringstream ss_data;
		    ss_data << "trace.feedbacker." << this->rank() << ".algo.data.txt";
		    _of_algo_data.open(ss_data.str());
#endif // !TRACE
		}

		void operator()(core::Pop<EOT>& pop, core::IslandData<EOT>& data)
		{
		    /************************************************
		     * Send feedbacks back to all islands (ANALYSE) *
		     ************************************************/

		    // _of_algo_data << pop.size() << " "; _of_algo_data.flush();

#if __cplusplus > 199711L
		    for (auto& ind : pop)
#else
		    for (size_t i = 0; i < pop.size(); ++i)
#endif
		    	{
#if __cplusplus <= 199711L
			    EOT& ind = pop[i];
#endif

#if __cplusplus > 199711L
			    auto end = std::chrono::system_clock::now();
#else
			    std_or_boost::chrono::time_point< std_or_boost::chrono::system_clock > end = std_or_boost::chrono::system_clock::now();
#endif

#if __cplusplus > 199711L
			    auto elapsed = std_or_boost::chrono::duration_cast<std_or_boost::chrono::microseconds>( end - data.vectorLastUpdatedTime ).count() / 1000.;
#else
			    unsigned elapsed = std_or_boost::chrono::duration_cast<std_or_boost::chrono::microseconds>( end - data.vectorLastUpdatedTime ).count() / 1000.;
#endif

#if __cplusplus > 199711L
		    	    auto delta = ( ind.fitness() - ind.getLastFitness() ) / ( ind.receivedTime + elapsed );
#else
		    	    double delta = ( ind.fitness() - ind.getLastFitness() ) / ( ind.receivedTime + elapsed );
#endif

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
#if __cplusplus > 199711L
		    	    auto fbr = data.feedbackerReceivingQueue.pop();
		    	    auto Fi = std_or_boost::get<0>(fbr);
		    	    // auto t = std::get<1>(fbr);
		    	    auto from = std_or_boost::get<2>(fbr);
		    	    auto& Si = data.feedbacks[from];
			    auto& Ti = data.feedbackLastUpdatedTimes[from];
			    // const auto alpha_s = 0.99;
			    const auto alpha_s = 0.01;
#else
			    typedef typename EOT::Fitness Fitness;
		    	    std_or_boost::tuple<Fitness, double, size_t> fbr = data.feedbackerReceivingQueue.pop();
		    	    Fitness Fi = std_or_boost::get<0>(fbr);
		    	    // double t = std::get<1>(fbr);
		    	    size_t from = std_or_boost::get<2>(fbr);
		    	    Fitness& Si = data.feedbacks[from];
			    std_or_boost::chrono::time_point< std_or_boost::chrono::system_clock >& Ti = data.feedbackLastUpdatedTimes[from];
			    // const double alpha_s = 0.99;
			    const double alpha_s = 0.01;
#endif

			    // _of_algo_data << Ti << " "; _of_algo_data.flush();

		    	    // if (Ti)
		    	    // 	{
		    	    // 	    Ri = Ri + 0.01 * ( Fi/Ti - Ri );
		    	    // 	    _of_algo_data << Fi << "/" << Ti << "=" << Fi/Ti << " "; _of_algo_data.flush();
		    	    // 	}
		    	    // else
		    	    // 	{
		    	    // 	    Ri = Ri + 0.01 * ( Fi - Ri );
		    	    // 	}

			    Si = (1-alpha_s) * Si + alpha_s * Fi;
			    Ti = std_or_boost::chrono::system_clock::now();
		    	}
		}

		class Sender :
#if __cplusplus > 199711L
		    public core::Thread< core::Pop<EOT>&, core::IslandData<EOT>& >
#else
		    public core::Thread<EOT>
#endif
		    , public core::ParallelContext
		{
		public:
		    Sender(size_t to, size_t tag = 0) : ParallelContext(tag), _to(to) {}

		    void operator()(core::Pop<EOT>& /*pop*/, core::IslandData<EOT>& data)
		    {
			while (data.toContinue)
			    {
#if __cplusplus > 199711L
				auto fbs = data.feedbackerSendingQueue.pop( _to, true );
				auto fit = std_or_boost::get<0>( fbs );
#else
				typedef typename EOT::Fitness Fitness;
				std_or_boost::tuple<Fitness, double, size_t> fbs = data.feedbackerSendingQueue.pop( _to, true );
				Fitness fit = std_or_boost::get<0>( fbs );
#endif

				this->world().send(_to, this->size() * ( this->rank() + _to ) + this->tag(), fit);
			    }
		    }

		private:
		    size_t _to;
		};

		class Receiver :
#if __cplusplus > 199711L
		    public core::Thread< core::Pop<EOT>&, core::IslandData<EOT>& >
#else
		    public core::Thread<EOT>
#endif
		    , public core::ParallelContext
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

#if __cplusplus > 199711L
		virtual void addTo( core::ThreadsRunner< core::Pop<EOT>&, core::IslandData<EOT>& >& tr )
#else
		virtual void addTo( core::ThreadsRunner<EOT>& tr )
#endif
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
#ifdef TRACE
		std::ofstream _of_algo, _of_algo_data;
#endif // !TRACE
	    };
	} // !async
    }
}

#endif /* _FEEDBACKER_EASY_H_ */
