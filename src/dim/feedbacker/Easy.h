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
		virtual void firstCompute(core::Pop<EOT>& /*pop*/, core::IslandData<EOT>& /*data*/)
		{
		    std::ostringstream ss;
		    ss << "trace.feedbacker." << this->rank() << ".algo.txt";
		    _of_algo.open(ss.str());

		    std::ostringstream ss_data;
		    ss_data << "trace.feedbacker." << this->rank() << ".algo.data.txt";
		    _of_algo_data.open(ss_data.str());
		}

		void compute(core::Pop<EOT>& pop, core::IslandData<EOT>& data)
		{
		    if (!pop.empty())
			{
			    /************************************************
			     * Send feedbacks back to all islands (ANALYSE) *
			     ************************************************/

			    // old population based model for communication
			    // std::vector<typename EOT::Fitness> sums(this->size(), 0);
			    // std::vector<int> nbs(this->size(), 0);
			    // for (auto& ind : pop)
			    // 	{
			    // 	    sums[ind.getLastIsland()] += ind.fitness() - ind.getLastFitness();
			    // 	    ++nbs[ind.getLastIsland()];
			    // 	}

			    // _m_fbs.lock();
			    // for (size_t i = 0; i < this->size(); ++i)
			    // 	{
			    // 	    if (i == this->rank()) { continue; }
			    // 	    data.feedbacks[i] = nbs[i] > 0 ? sums[i] / nbs[i] : 0;
			    // 	    // std::cout << data.feedbacks[i] << " "; std::cout.flush();
			    // 	    _vec_fbs[i].push( data.feedbacks[i] );
			    // 	}
			    // _m_fbs.unlock();

			    // // for island itself because of the MPI communication optimizing.
			    // data.feedbacks[this->rank()] = nbs[this->rank()] > 0 ? sums[this->rank()] / nbs[this->rank()] : 0;

			    // new model
			    for (auto& ind : pop)
			    	{
			    	    auto delta = ind.fitness() - ind.getLastFitness();

				    // if (ind.getLastIsland() == this->rank())
				    // 	{
				    // 	    auto& Ri = data.feedbacks[this->rank()];

				    // 	    // data.feedbacks[this->rank()] = data.feedbacks[this->rank()] + 0.01 * ( delta - data.feedbacks[this->rank()] );
				    // 	    Ri = Ri + 0.01 * ( delta - Ri );
				    // 	    continue;
				    // 	}

				    auto& fbsPair = data.feedbackerSendingQueuesVector[ind.getLastIsland()];
				    fbsPair.first.lock();
				    fbsPair.second.push( delta );
				    fbsPair.first.unlock();
			    	}

			    auto& fbr = data.feedbackerReceivingQueuesVector[this->rank()].second;
			    auto& fbs = data.feedbackerSendingQueuesVector[this->rank()].second;
			    while ( !fbs.empty() )
			    	{
			    	    fbr.push( fbs.front() );
			    	    fbs.pop();
			    	}
			}

		    /*************************************
		     * Update feedbacks from all islands *
		     *************************************/

		    for (size_t i = 0; i < this->size(); ++i)
			{
			    // if (i == this->rank()) { continue; }

			    auto& m = data.feedbackerReceivingQueuesVector[i].first;
			    auto& fbr = data.feedbackerReceivingQueuesVector[i].second;
			    auto& Ri = data.feedbacks[i];

			    m.lock();
			    size_t n = fbr.size();
			    if (n)
				{
				    // typename EOT::Fitness sum = 0;
				    while (!fbr.empty())
					{
					    // sum += _vec_fbr[i].front();

					    auto& Fi = fbr.front();
					    Ri = Ri + 0.01 * ( Fi - Ri );

					    fbr.pop();
					}
				    // double coef = pop.size() / (popSize*this->size());
				    // data.feedbacks[i] = (sum / (typename EOT::Fitness)(n))/* * coef*/;
				    _of_algo_data << Ri << " "; _of_algo_data.flush();
				    // std::cout << data.feedbacks[i] << " "; std::cout.flush();
				}
			    m.unlock();
			}
		}

		virtual void firstCommunicate(core::Pop<EOT>&, core::IslandData<EOT>&)
		{
		    std::ostringstream ss;
		    ss << "trace.feedbacker." << this->rank() << ".comm.txt";
		    _of_comm.open(ss.str());

		    std::ostringstream ss_data;
		    ss_data << "trace.feedbacker." << this->rank() << ".comm.data.txt";
		    _of_comm_data.open(ss_data.str());
		}

		void communicate(core::Pop<EOT>& /*pop*/, core::IslandData<EOT>& data)
		{
		    // size_t next = (this->rank()+1) % this->size();
		    // _of_comm << _vec_fbs[next].size() << " ";
		    // _of_comm << _vec_fbr[next].size() << " ";
		    // _of_comm.flush();

		    for (size_t i = 0; i < this->size(); ++i)
			{
			    if (i == this->rank()) continue;

			    std::vector< boost::mpi::request > reqs;

			    /***********************************
			     * Send feedbacks to all islands *
			     ***********************************/
			    {

				auto& fbs = data.feedbackerSendingQueuesVector[i].second;
				auto& ms = data.feedbackerSendingQueuesVector[i].first;

				ms.lock();
				std::vector< typename EOT::Fitness > fbsVec;
				while (!fbs.empty())
				    {
					fbsVec.push_back( fbs.front() );
					fbs.pop();
				    }
				ms.unlock();

				this->world().send(i, this->tag()*10, fbsVec.size());

				if (fbsVec.size())
				    {
					reqs.push_back( this->world().isend(i, this->tag(), fbsVec) );
				    }
			    }

			    /****************************************
			     * Receive individuals from all islands *
			     ****************************************/
			    {

				size_t count = 0;
				this->world().recv(i, this->tag()*10, count);

				std::vector< typename EOT::Fitness > fbrVec;
				if (count)
				    {
					reqs.push_back( this->world().irecv(i, this->tag(), fbrVec) );
				    }

				/****************************
				 * Process all MPI requests *
				 ****************************/

				boost::mpi::wait_all( reqs.begin(), reqs.end() );

				auto& fbr = data.feedbackerReceivingQueuesVector[i].second;
				auto& mr = data.feedbackerReceivingQueuesVector[i].first;

				if (!fbrVec.empty())
				    {
					mr.lock();
					for (size_t k = 0; k < fbrVec.size(); ++k)
					    {
						fbr.push( fbrVec[k] );
					    }
					mr.unlock();
				    }
			    }
			}
		}

	    private:
		std::ofstream _of_comm, _of_algo;
		std::ofstream _of_comm_data, _of_algo_data;
	    };
	} // !async
    }
}

#endif /* _FEEDBACKER_EASY_H_ */
