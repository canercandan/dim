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

#ifndef _MIGRATOR_EASY_H_
#define _MIGRATOR_EASY_H_

#include <vector>
#include <queue>
#include <algorithm>
#include <fstream>

#include "Base.h"

namespace dim
{
    namespace migrator
    {
	namespace sync
	{
	    template <typename EOT>
	    class Easy : public Base<EOT>
	    {
	    public:
		virtual void firstCall(core::Pop<EOT>& /*pop*/, core::IslandData<EOT>& /*data*/)
		{
		    _outputSizes.resize( this->size(), 0 );
		}

		void operator()(core::Pop<EOT>& pop, core::IslandData<EOT>& data)
		{
		    std::vector< boost::mpi::request > reqs;

		    /***********************************
		     * Send individuals to all islands *
		     ***********************************/
		    {
			std::vector< core::Pop<EOT> > pops( this->size() );

			/*************
			 * Selection *
			 *************/

			for (auto &indi : pop)
			    {
				double s = 0;
				int r = rng.rand() % 1000 + 1;

				size_t j;
				for ( j = 0; j < this->size() && r > s; ++j )
				    {
					s += data.proba[j];
				    }
				--j;

				pops[j].push_back(indi);
			    }

			size_t outputSize = 0;

			for (size_t i = 0; i < this->size(); ++i)
			    {
				if (i == this->rank()) { continue; }
				_outputSizes[i] = pops[i].size();
				outputSize += pops[i].size();
			    }

			pop.setOutputSizes( _outputSizes );
			pop.setOutputSize( outputSize );

			pop.clear();

			for ( size_t i = 0; i < this->size(); ++i )
			    {
				if (i == this->rank()) { continue; }
				reqs.push_back( this->world().isend( i, this->tag(), pops[i] ) );
			    }

			for (auto &indi : pops[this->rank()])
			    {
				pop.push_back( indi );
			    }
		    }

		    std::vector< core::Pop<EOT> > pops( this->size() );

		    /****************************************
		     * Receive individuals from all islands *
		     ****************************************/
		    {
			for (size_t i = 0; i < this->size(); ++i)
			    {
				if (i == this->rank()) { continue; }
				reqs.push_back( this->world().irecv( i, this->tag(), pops[i] ) );
			    }
		    }

		    /****************************
		     * Process all MPI requests *
		     ****************************/

		    boost::mpi::wait_all( reqs.begin(), reqs.end() );

		    /*********************
		     * Update population *
		     *********************/
		    {
			size_t inputSize = 0;

			for (size_t i = 0; i < this->size(); ++i)
			    {
				if (i == this->rank()) { continue; }

				for (auto &indi : pops[i])
				    {
					pop.push_back( indi );
				    }

				inputSize += pops[i].size();
			    }

			pop.setInputSize( inputSize );
		    }
		}

	    private:
		std::vector< size_t > _outputSizes;
	    };
	} // !sync

	namespace async
	{
	    template <typename EOT>
	    class Easy : public Base<EOT>
	    {
	    public:
		virtual void firstCompute(core::Pop<EOT>& pop, core::IslandData<EOT>& data)
		{
		    std::ostringstream ss;
		    ss << "trace.migrator." << this->rank() << ".algo.txt";
		    _of_algo.open(ss.str());

		    for (auto& ind : pop)
			{
			    auto& immData = data.migratorReceivingQueuesVector[this->rank()];
			    auto& m = std::get<0>(immData);
			    auto& imm = std::get<1>(immData);
			    m.lock();
			    imm.push( ind );
			    m.unlock();
			}
		    pop.clear();
		}

		void compute(core::Pop<EOT>& pop, core::IslandData<EOT>& data)
		{
		    if (!pop.empty())
			{
			    /***********************************
			     * Send individuals to all islands *
			     ***********************************/

			    /*************
			     * Selection *
			     *************/

			    std::vector< size_t > outputSizes( this->size(), 0 );

			    for (auto& ind : pop)
				{
				    double s = 0;
				    int r = rng.rand() % 1000 + 1;

				    size_t j;
				    for ( j = 0; j < this->size() && r > s; ++j )
					{
					    s += data.proba[j];
					}
				    --j;

				    _of_algo << j << " "; _of_algo.flush();

				    auto& emData = data.migratorSendingQueuesVector[j];
				    auto& m = std::get<0>(emData);
				    auto& em = std::get<1>(emData);
				    m.lock();
				    em.push( ind );
				    m.unlock();
				    ++outputSizes[j];
				}

			    pop.clear();

			    pop.setOutputSizes( outputSizes );
			    pop.setOutputSize( std::accumulate(outputSizes.begin(), outputSizes.end(), 0) );

			    // in order to avoid communicating individuals who wanna stay in the same island
			    auto& em = std::get<1>(data.migratorSendingQueuesVector[this->rank()]);
			    auto& imm = std::get<1>(data.migratorReceivingQueuesVector[this->rank()]);
			    while ( !em.empty() )
				{
				    imm.push( em.front() );
				    em.pop();
				}
			}

		    /*********************
		     * Update population *
		     *********************/
		    {
			// std::vector<size_t> order(this->size(), 0);

			// for (size_t i = 0; i < this->size(); ++i)
			//     {
			// 	order[i] = i;
			//     }

			// for (size_t i = this->size() - 1; i > 0; --i)
			//     {
			// 	std::swap( order[i], order[ rng.random(this->size()) ] );
			//     }

			size_t inputSize = 0;

			// We're waiting until the queue has an individual to continue
			bool stop = false;
			while (!stop)
			    {
				double s = 0;
				int r = rng.rand() % 1000 + 1;

				size_t j;
				for ( j = 0; j < this->size() && r > s; ++j )
				    {
					s += data.proba[j];
				    }
				--j;

				_of_algo << j << " "; _of_algo.flush();

				auto& imm = std::get<1>(data.migratorReceivingQueuesVector[j]);
				if (!imm.empty())
				    {
					pop.push_back( imm.front() );
					imm.pop();
					++inputSize;
					stop = true;
					break;
				    }

				// for (size_t i = 0; i < this->size(); ++i)
				//     {
				// 	size_t island = order[i];

				// 	_of_algo << island << " "; _of_algo.flush();

				// 	auto& imm = std::get<1>(data.migratorReceivingQueuesVector[island]);
				// 	if (!imm.empty())
				// 	    {
				// 		pop.push_back( imm.front() );
				// 		imm.pop();
				// 		++inputSize;
				// 		stop = true;
				// 		break;
				// 	    }
				//     }
			    }

			pop.setInputSize( inputSize );
		    }
		}

		virtual void firstCommunicate(core::Pop<EOT>&, core::IslandData<EOT>&)
		{
		    std::ostringstream ss;
		    ss << "trace.migrator." << this->rank() << ".comm.txt";
		    _of_comm.open(ss.str());
		}

		void communicate(core::Pop<EOT>& /*pop*/, core::IslandData<EOT>& data)
		{
		    for (size_t i = 0; i < this->size(); ++i)
			{
			    if (i == this->rank()) continue;

			    std::vector< boost::mpi::request > reqs;

			    /***********************************
			     * Send individuals to all islands *
			     ***********************************/

			    auto& emData = data.migratorSendingQueuesVector[i];
			    auto& m = std::get<0>(emData);
			    auto& em = std::get<1>(emData);

			    m.lock();
			    std::vector< EOT > emVec;
			    while (!em.empty())
				{
				    emVec.push_back( em.front() );
				    em.pop();
				}
			    m.unlock();

			    this->world().send(i, this->tag()*10, emVec.size());

			    if (emVec.size())
				{
				    reqs.push_back( this->world().isend(i, this->tag(), emVec) );
				}

			    /****************************************
			     * Receive individuals from all islands *
			     ****************************************/

			    size_t count = 0;
			    this->world().recv(i, this->tag()*10, count);

			    std::vector< EOT > immVec;
			    if (count)
				{
				    reqs.push_back( this->world().irecv(i, this->tag(), immVec) );
				}

			    /****************************
			     * Process all MPI requests *
			     ****************************/

			    boost::mpi::wait_all( reqs.begin(), reqs.end() );

			    if (!immVec.empty())
				{
				    auto& immData = data.migratorReceivingQueuesVector[i];
				    auto& m = std::get<0>(immData);
				    auto& imm = std::get<1>(immData);

				    m.lock();
				    for (size_t k = 0; k < immVec.size(); ++k)
					{
					    imm.push( immVec[k] );
					}
				    m.unlock();
				}
			}
		}

	    private:
		std::ofstream _of_comm, _of_algo;
	    };
	} // !async
    }
}

#endif /* _MIGRATOR_EASY_H_ */
