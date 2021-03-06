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

// For time mesuring
#if __cplusplus > 199711L
#include <chrono>
#else
#include <boost/chrono/chrono_io.hpp>
#endif

#include <vector>
#include <queue>
#include <algorithm>
#include <fstream>

#include "Base.h"
#include <dim/utils/Measure.h>

#include <boost/utility/identity_type.hpp>

#undef AUTO
#if __cplusplus > 199711L
#define AUTO(TYPE) auto
#else // __cplusplus <= 199711L
#define AUTO(TYPE) TYPE
#endif

#undef MOVE
#if __cplusplus > 199711L
# define MOVE(var) std::move(var)
#else
# define MOVE(var) var
#endif

namespace dim
{
    namespace migrator
    {
#if __cplusplus > 199711L
	namespace std_or_boost = std;
#else
	namespace std_or_boost = boost;
#endif

	namespace smp
	{
	    template <typename EOT>
	    class Easy : public Base<EOT>
	    {
	    public:
		Easy(std::vector< core::Pop<EOT>* >& islandPop, std::vector< core::IslandData<EOT>* >& islandData) : _islandPop(islandPop), _islandData(islandData) {}

		virtual void firstCall(core::Pop<EOT>& pop, core::IslandData<EOT>& data)
		{
		    std::ostringstream ss;

#ifdef TRACE
		    ss.str(""); ss << "trace.migrator." << this->rank();
		    _of.open(ss.str().c_str());
#endif // !TRACE

#ifdef MEASURE
		    ss.str(""); ss << data.monitorPrefix << ".migrate_total.time." << this->rank();
		    _measureFiles["migrate_total"] = new std::ofstream(ss.str().c_str());

		    ss.str(""); ss << data.monitorPrefix << ".migrate_send.time." << this->rank();
		    _measureFiles["migrate_send"] = new std::ofstream(ss.str().c_str());

		    ss.str(""); ss << data.monitorPrefix << ".migrate_push.time." << this->rank();
		    _measureFiles["migrate_push"] = new std::ofstream(ss.str().c_str());

		    ss.str(""); ss << data.monitorPrefix << ".migrate_update.time." << this->rank();
		    _measureFiles["migrate_update"] = new std::ofstream(ss.str().c_str());

		    ss.str(""); ss << data.monitorPrefix << ".migrate_wait.time." << this->rank();
		    _measureFiles["migrate_wait"] = new std::ofstream(ss.str().c_str());
#endif // !MEASURE
		}

		virtual void lastCall(core::Pop<EOT>&, core::IslandData<EOT>&)
		{
#ifdef MEASURE
		    for ( std::map<std::string, std::ofstream*>::iterator it = _measureFiles.begin(); it != _measureFiles.end(); ++it )
			{
			    delete it->second;
			}
#endif // !MEASURE
		}

		void operator()(core::Pop<EOT>& __pop, core::IslandData<EOT>& __data)
		{
		    DO_MEASURE(

			       core::Pop<EOT>& pop = *(_islandPop[this->rank()]);
			       core::IslandData<EOT>& data = *(_islandData[this->rank()]);

			       /********************
				* Send individuals *
				********************/

			       DO_MEASURE(
					  std::vector< size_t > outputSizes( this->size(), 0 );

					  {
					      for (size_t i = 0; i < pop.size(); ++i)
						  {
						      EOT& ind = pop[i];

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

						      DO_MEASURE(
								 ++outputSizes[j];
#ifdef TRACE
								 _of << ind.getLastFitnesses().size() << " ";
#endif // !TRACE

								 _islandData[j]->migratorReceivingQueue.push(MOVE(ind), this->rank());

								 , _measureFiles, "migrate_push");
						  }

					      pop.clear();

					      pop.setOutputSizes( outputSizes );
					      pop.setOutputSize( std::accumulate(outputSizes.begin(), outputSizes.end(), 0) );
					  }
					  , _measureFiles, "migrate_send" );

			       DO_MEASURE(
					  __data.bar.wait();
					  , _measureFiles, "migrate_wait" );

			       /*********************
				* Update population *
				*********************/

			       DO_MEASURE(
					  size_t inputSize = 0;

					  // size_t size = data.migratorReceivingQueue.size();
					  // for (int k = 0; k < size; ++k);
					  while ( !data.migratorReceivingQueue.empty() )
					      {
						  AUTO(typename BOOST_IDENTITY_TYPE((std_or_boost::tuple<EOT, double, size_t>))) imm = data.migratorReceivingQueue.pop();
						  AUTO(EOT) ind = MOVE(std_or_boost::get<0>(imm));
						  AUTO(double) time = std_or_boost::get<1>(imm);

						  ind.receivedTime = time;
						  pop.push_back( MOVE(ind) );
						  ++inputSize;
					      }

					  pop.setInputSize( inputSize );
					  , _measureFiles, "migrate_update" );

			       , _measureFiles, "migrate_total" );
		}

	    private:
		std::vector< core::Pop<EOT>* >& _islandPop;
		std::vector< core::IslandData<EOT>* >& _islandData;

#ifdef TRACE
		std::ofstream _of;
#endif // !TRACE

#ifdef MEASURE
		std::map<std::string, std::ofstream*> _measureFiles;
#endif // !MEASURE
	    };
	} // !smp

	namespace sync
	{
	    template <typename EOT>
	    class Easy : public Base<EOT>
	    {
	    public:
		virtual void firstCall(core::Pop<EOT>& pop, core::IslandData<EOT>& data)
		{
#ifdef TRACE
		    std::ostringstream ss;
		    ss << "trace.migrator." << this->rank();
		    _of.open(ss.str().c_str());
#endif // !TRACE
		}

		void operator()(core::Pop<EOT>& pop, core::IslandData<EOT>& data)
		{
		    std::vector< boost::mpi::request > reqs;

		    {
			/********************
			 * Send individuals *
			 ********************/

			std::vector< size_t > outputSizes( this->size(), 0 );
			std::vector< core::Pop<EOT> > pops( this->size() );

			for (size_t i = 0; i < pop.size(); ++i)
			    {
				EOT& ind = pop[i];

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

				++outputSizes[j];
				pops[j].push_back(ind);
			    }

			pop.clear();

			pop.setOutputSizes( outputSizes );
			pop.setOutputSize( std::accumulate(outputSizes.begin(), outputSizes.end(), 0) );

			for ( size_t i = 0; i < this->size(); ++i )
			    {
				if (i == this->rank()) { continue; }
				reqs.push_back( this->world().isend( i, this->tag(), pops[i] ) );
			    }

			for (size_t i = 0; i < pops[this->rank()].size(); ++i)
			    {
				EOT& ind = pops[this->rank()][i];
				pop.push_back( ind );
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

				core::Pop<EOT>& newpop = pops[i];
				for (size_t j = 0; j < newpop.size(); ++j)
				    {
					EOT& ind = newpop[j];
					pop.push_back( ind );
				    }

				inputSize += newpop.size();
			    }

			pop.setInputSize( inputSize );
		    }
		}

	    private:
#ifdef TRACE
		std::ofstream _of;
#endif // !TRACE

	    };
	} // !sync

	namespace async
	{
	    template <typename EOT>
	    class Easy : public Base<EOT>
	    {
	    public:
		Easy(size_t nmigrations = 1) : _nmigrations(nmigrations) {}

		~Easy()
		{
		    for (size_t i = 0; i < this->_senders.size(); ++i)
			{
			    delete _senders[i];
			    delete _receivers[i];
			}
		}

		virtual void firstCall(core::Pop<EOT>& pop, core::IslandData<EOT>& data)
		{
#ifdef TRACE
		    std::ostringstream ss;
		    ss << "trace.migrator." << this->rank();
		    _of.open(ss.str().c_str());
#endif // !TRACE

		    for (size_t i = 0; i < pop.size(); ++i)
			{
			    EOT& ind = pop[i];

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

		    for (size_t i = 0; i < pop.size(); ++i)
			{
			    EOT& ind = pop[i];

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

		    // waiting until the queue is fulfilled
		    while ( data.migratorReceivingQueue.empty() ) {}

		    size_t size = data.migratorReceivingQueue.size();
		    if ( _nmigrations && _nmigrations < size ) { size = _nmigrations; }

		    for (int k = 0; k < size; ++k)
			{
			    // This special pop function is waiting while the queue of individual is empty.
			    AUTO(typename BOOST_IDENTITY_TYPE((std_or_boost::tuple<EOT, double, size_t>))) imm = data.migratorReceivingQueue.pop(true);
			    AUTO(EOT) ind = std_or_boost::get<0>(imm);
			    AUTO(double) time = std_or_boost::get<1>(imm);

			    ind.receivedTime = time;
			    pop.push_back( ind );
			    ++inputSize;
			}

		    pop.setInputSize( inputSize );
		}

		class Sender : public core::Thread<EOT>, public core::ParallelContext
		{
		public:
		    Sender(size_t to, size_t tag = 0) : ParallelContext(tag), _to(to) {}

		    void operator()(core::Pop<EOT>& /*pop*/, core::IslandData<EOT>& data)
		    {
			while (data.toContinue)
			    {
				// waiting until there is an individual in the queue
				AUTO(typename BOOST_IDENTITY_TYPE((std_or_boost::tuple<EOT, double, size_t>))) em = data.migratorSendingQueue.pop( _to, true );
				AUTO(EOT) ind = std_or_boost::get<0>(em);

				this->world().send(_to, this->size() * ( this->rank() + _to ) + this->tag(), ind);
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
				EOT ind;
				this->world().recv(_from, this->size() * ( this->rank() + _from ) + this->tag(), ind);
				data.migratorReceivingQueue.push( ind, _from );
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
		size_t _nmigrations;
		std::vector<Sender*> _senders;
		std::vector<Receiver*> _receivers;
#ifdef TRACE
		std::ofstream _of;
#endif // !TRACE

	    };
	}
    } // !migrator
} // !dim

#endif /* _MIGRATOR_EASY_H_ */
