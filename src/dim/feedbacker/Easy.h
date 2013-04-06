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

#include <cmath>
#include <vector>
#include <queue>

#include "Base.h"

#include <boost/utility/identity_type.hpp>

#undef AUTO
#if __cplusplus > 199711L
#define AUTO(TYPE) auto
#else // __cplusplus <= 199711L
#define AUTO(TYPE) TYPE
#endif

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
			{
#else
		    for (size_t i = 0; i < pop.size(); ++i)
			{
			    EOT& ind = pop[i];
#endif

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
		    for (size_t i = 0; i < this->_senders.size(); ++i)
			{
			    delete _senders[i];
			    delete _receivers[i];
			}
		}

		virtual void firstCall(core::Pop<EOT>& /*pop*/, core::IslandData<EOT>& /*data*/)
		{
#ifdef TRACE
		    std::ostringstream ss;
		    ss << "trace.feedbacker." << this->rank();
		    _of.open(ss.str());
#endif // !TRACE
		}

		void operator()(core::Pop<EOT>& pop, core::IslandData<EOT>& data)
		{
		    /************************************************
		     * Send feedbacks back to all islands (ANALYSE) *
		     ************************************************/

		    // _of << pop.size() << " "; _of.flush();

#if __cplusplus > 199711L
		    for (auto& ind : pop)
			{
#else
		    for (size_t i = 0; i < pop.size(); ++i)
		    	{
			    EOT& ind = pop[i];
#endif

			    // AUTO(unsigned) elapsed = std_or_boost::chrono::duration_cast<std_or_boost::chrono::microseconds>( std_or_boost::chrono::system_clock::now() - data.vectorLastUpdatedTime ).count() / 1000.;
		    	    AUTO(double) effectiveness = ind.fitness() - ind.getLastFitness();

		    	    if ( ind.getLastIsland() == static_cast<int>( this->rank() ) )
		    		{
		    		    data.feedbackerReceivingQueue.push( effectiveness, this->rank() );
		    		    continue;
		    		}

		    	    data.feedbackerSendingQueue.push( effectiveness, ind.getLastIsland() );
		    	}

		    /********************
		     * Update feedbacks *
		     ********************/

		    while ( !data.feedbackerReceivingQueue.empty() )
		    	{
		    	    AUTO(typename BOOST_IDENTITY_TYPE((std_or_boost::tuple<typename EOT::Fitness, double, size_t>))) fbr = data.feedbackerReceivingQueue.pop();
		    	    AUTO(typename EOT::Fitness) Fi = std_or_boost::get<0>(fbr);
		    	    // double t = std::get<1>(fbr);
		    	    AUTO(size_t) from = std_or_boost::get<2>(fbr);
		    	    AUTO(typename EOT::Fitness)& Si = data.feedbacks[from];
			    AUTO(std_or_boost::chrono::time_point< std_or_boost::chrono::system_clock >)& Ti = data.feedbackLastUpdatedTimes[from];
			    const AUTO(double) alpha_f = 0.01;

			    AUTO(std_or_boost::chrono::time_point< std_or_boost::chrono::system_clock >) end = std_or_boost::chrono::system_clock::now(); // t
			    AUTO(unsigned) elapsed = std_or_boost::chrono::duration_cast<std_or_boost::chrono::milliseconds>( end - Ti ).count(); // delta{t} <- t - t_i

			    AUTO(double) alphaT = exp(log(alpha_f)/elapsed);
			    Si = (1-alphaT)*Si + alphaT*Fi;

#ifdef TRACE
			    _of << Si << " "; _of.flush();
#endif

			    Ti = end; // t_i <- t
		    	}
		}

		class Sender : public core::Thread<EOT>, public core::ParallelContext
		{
		public:
		    Sender(size_t to, size_t tag = 0) : ParallelContext(tag), _to(to) {}

		    void operator()(core::Pop<EOT>& /*pop*/, core::IslandData<EOT>& data)
		    {
			while (data.toContinue)
			    {
				AUTO(typename BOOST_IDENTITY_TYPE((std_or_boost::tuple<typename EOT::Fitness, double, size_t>))) fbs = data.feedbackerSendingQueue.pop( _to, true );
				AUTO(typename EOT::Fitness) fit = std_or_boost::get<0>( fbs );

				this->world().send(_to, this->size() * ( this->rank() + _to ) + this->tag(), fit);
			    }
		    }

		private:
		    size_t _to;
		};

		class Receiver : public core::Thread<EOT>, public core::ParallelContext
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

		virtual void addTo( core::ThreadsRunner<EOT>& tr )
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
		std::ofstream _of;
#endif // !TRACE

	    };
	} // !async
    }
}

#endif /* _FEEDBACKER_EASY_H_ */
