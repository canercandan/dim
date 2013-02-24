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
		~Easy()
		{
		    for (auto& sender : _senders) { delete sender; }
		    for (auto& receiver : _receivers) { delete receiver; }
		}

		virtual void firstCall(core::Pop<EOT>& pop, core::IslandData<EOT>& data)
		{
		    std::ostringstream ss;
		    ss << "trace.migrator." << this->rank() << ".algo.txt";
		    _of_algo.open(ss.str());

		    for (auto& ind : pop)
			{
			    data.migratorReceivingQueue.push( ind, this->rank() );
			}
		    pop.clear();
		}

		void operator()(core::Pop<EOT>& pop, core::IslandData<EOT>& data)
		{
		    /********************
		     * Send individuals *
		     ********************/

		    std::vector< size_t > outputSizes( this->size(), 0 );

		    for (auto& ind : pop)
		    	{
		    	    /*************
		    	     * Selection *
		    	     *************/

		    	    double s = 0;
		    	    int r = rng.rand() % 1000 + 1;

		    	    size_t j;
		    	    for ( j = 0; j < this->size() && r > s; ++j )
		    		{
		    		    s += data.proba[j];
		    		}
		    	    --j;

		    	    // _of_algo << j << " "; _of_algo.flush();

		    	    ++outputSizes[j];

		    	    if (j == this->rank())
		    		{
		    		    data.migratorReceivingQueue.push( ind, this->rank() );
		    		    continue;
		    		}

		    	    data.migratorSendingQueue.push( ind, j );
		    	}

		    pop.clear();

		    pop.setOutputSizes( outputSizes );
		    pop.setOutputSize( std::accumulate(outputSizes.begin(), outputSizes.end(), 0) );

		    /*********************
		     * Update population *
		     *********************/

		    size_t inputSize = 0;

		    // a loop just in case we want more than 1 individual per generation coming to island
		    for (int k = 0; k < 1; ++k)
		    	{
		    	    // This special pop function is waiting while the queue of individual is empty.
		    	    auto imm = data.migratorReceivingQueue.pop(true);
		    	    auto ind = std::get<0>(imm);
		    	    pop.push_back( ind );
		    	    ++inputSize;
		    	}

		    pop.setInputSize( inputSize );
		}

		class Sender : public core::Thread< core::Pop<EOT>&, core::IslandData<EOT>& >, public core::ParallelContext
		{
		public:
		    Sender(size_t to, size_t tag = 0) : ParallelContext(tag), _to(to) {}

		    void operator()(core::Pop<EOT>& /*pop*/, core::IslandData<EOT>& data)
		    {
			while (data.toContinue)
			    {
				// waiting until there is an individual in the queue
				auto em = data.migratorSendingQueue.pop( _to, true );
				auto ind = std::get<0>(em);
				this->world().send(_to, this->size() * ( this->rank() + _to ) + this->tag(), ind);
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
				EOT ind;
				this->world().recv(_from, this->size() * ( this->rank() + _from ) + this->tag(), ind);
				data.migratorReceivingQueue.push( ind, _from );
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
		std::ofstream _of_comm, _of_algo;
	    };
	} // !async
    } // !migrator
} // !dim

#endif /* _MIGRATOR_EASY_H_ */
