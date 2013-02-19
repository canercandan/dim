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

#ifndef _ALGO_EASY_H_
#define _ALGO_EASY_H_

// For time mesuring
#include <chrono>
#include <sstream>
#include <fstream>
#include <atomic>

#include <dim/core/core>
#include <dim/utils/utils>
#include <dim/evolver/evolver>
#include <dim/feedbacker/feedbacker>
#include <dim/inputprobasender/inputprobasender>
#include <dim/vectorupdater/vectorupdater>
#include <dim/memorizer/memorizer>
#include <dim/migrator/migrator>

#include "Base.h"

namespace dim
{
    namespace algo
    {
	namespace sync
	{
	    template <typename EOT>
	    class Easy : public Base<EOT>
	    {
	    public:
		Easy(utils::CheckPoint<EOT>& checkpoint) : _checkpoint(checkpoint), _evolve(_dummyEvolve), _feedback(_dummyFeedback), _probasend(_dummyProbaSend), _update(_dummyUpdate), _memorize(_dummyMemorize), _migrate(_dummyMigrate) {}

		Easy(evolver::sync::Base<EOT>& evolver, feedbacker::sync::Base<EOT>& feedbacker, inputprobasender::sync::Base<EOT>& probasender, dim::vectorupdater::sync::Base<EOT>& updater, dim::memorizer::sync::Base<EOT>& memorizer, migrator::sync::Base<EOT>& migrator, utils::CheckPoint<EOT>& checkpoint) : _checkpoint(checkpoint), _evolve(evolver), _feedback(feedbacker), _probasend(probasender), _update(updater), _memorize(memorizer), _migrate(migrator) {}

		virtual ~Easy() {}

		void operator()(core::Pop<EOT>& pop, core::IslandData<EOT>& data)
		{
		    std::ostringstream ss;
		    ss << "gen.time." << this->rank();
		    std::ofstream gen_time(ss.str());

		    _evolve.firstCall(pop, data);
		    _feedback.firstCall(pop, data);
		    _probasend.firstCall(pop, data);
		    _update.firstCall(pop, data);
		    _memorize.firstCall(pop, data);
		    _migrate.firstCall(pop, data);

		    while ( _checkpoint(pop) )
			{
			    auto gen_start = std::chrono::system_clock::now();

			    _evolve(pop, data);
			    _feedback(pop, data);
			    _probasend(pop, data); // juste pour stat
			    _update(pop, data);
			    _memorize(pop, data);
			    _migrate(pop, data);

			    auto gen_end = std::chrono::system_clock::now();
			    long elapsed_microseconds = std::chrono::duration_cast<std::chrono::microseconds>(gen_end-gen_start).count();
			    gen_time << elapsed_microseconds << " "; gen_time.flush();
			}

		    _evolve.lastCall(pop, data);
		    _feedback.lastCall(pop, data);
		    _probasend.lastCall(pop, data);
		    _update.lastCall(pop, data);
		    _memorize.lastCall(pop, data);
		    _migrate.lastCall(pop, data);
		}

	    private:
		struct DummyEvolver : public evolver::sync::Base<EOT> { void operator()(core::Pop<EOT>&, core::IslandData<EOT>&) {} } _dummyEvolve;
		struct DummyFeedbacker : public feedbacker::sync::Base<EOT> { void operator()(core::Pop<EOT>&, core::IslandData<EOT>&) {} } _dummyFeedback;
		struct DummyInputProbaSender : public inputprobasender::sync::Base<EOT> { void operator()(core::Pop<EOT>&, core::IslandData<EOT>&) {} } _dummyProbaSend;
		struct DummyVectorUpdater : public vectorupdater::sync::Base<EOT> { void operator()(core::Pop<EOT>&, core::IslandData<EOT>&) {} } _dummyUpdate;
		struct DummyMemorizer : public memorizer::sync::Base<EOT> { void firstCall(core::Pop<EOT>&, core::IslandData<EOT>&) {}; void operator()(core::Pop<EOT>&, core::IslandData<EOT>&) {} } _dummyMemorize;
		struct DummyMigrator : public migrator::sync::Base<EOT> { void operator()(core::Pop<EOT>&, core::IslandData<EOT>&) {} } _dummyMigrate;

		utils::CheckPoint<EOT>& _checkpoint;
		evolver::sync::Base<EOT>& _evolve;
		feedbacker::sync::Base<EOT>& _feedback;
		inputprobasender::sync::Base<EOT>& _probasend;
		vectorupdater::sync::Base<EOT>& _update;
		memorizer::sync::Base<EOT>& _memorize;
		migrator::sync::Base<EOT>& _migrate;

		bool _initG = true;
		size_t _probaSame = 100;
	    };
	} // !sync

	namespace async
	{
	    template <typename EOT>
	    class Easy : public Base<EOT>
	    {
	    public:
		Easy(utils::CheckPoint<EOT>& checkpoint) : _checkpoint(checkpoint), _evolve(_dummyEvolve), _feedback(_dummyFeedback), _probasend(_dummyProbaSend), _update(_dummyUpdate), _memorize(_dummyMemorize), _migrate(_dummyMigrate), _tocontinue(true) {}

		Easy(evolver::async::Base<EOT>& evolver, feedbacker::async::Base<EOT>& feedbacker, inputprobasender::async::Base<EOT>& probasender, dim::vectorupdater::async::Base<EOT>& updater, dim::memorizer::async::Base<EOT>& memorizer, migrator::async::Base<EOT>& migrator, utils::CheckPoint<EOT>& checkpoint) : _checkpoint(checkpoint), _evolve(evolver), _feedback(feedbacker), _probasend(probasender), _update(updater), _memorize(memorizer), _migrate(migrator), _tocontinue(true) {}

		virtual ~Easy() {}

		void compute(core::Pop<EOT>& pop, core::IslandData<EOT>& data)
		{
		    _evolve.firstCompute(pop, data);
		    _feedback.firstCompute(pop, data);
		    _probasend.firstCompute(pop, data);
		    _update.firstCompute(pop, data);
		    _memorize.firstCompute(pop, data);
		    _migrate.firstCompute(pop, data);

		    // while ( _tocontinue )
		    while ( ( _tocontinue = _checkpoint(pop) ) )
			{
			    // std::this_thread::sleep_for(std::chrono::microseconds( 1000000 ));

			    _evolve.compute(pop, data);
			    _feedback.compute(pop, data);
			    _probasend.compute(pop, data); // juste pour stat
			    _update.compute(pop, data);
			    _memorize.compute(pop, data);
			    _migrate.compute(pop, data);
			}

		    _evolve.lastCompute(pop, data);
		    _feedback.lastCompute(pop, data);
		    _probasend.lastCompute(pop, data);
		    _update.lastCompute(pop, data);
		    _memorize.lastCompute(pop, data);
		    _migrate.lastCompute(pop, data);
		}

		void communicate(core::Pop<EOT>& pop, core::IslandData<EOT>& data)
		{
		    _evolve.firstCommunicate(pop, data);
		    _feedback.firstCommunicate(pop, data);
		    _probasend.firstCommunicate(pop, data);
		    _update.firstCommunicate(pop, data);
		    _memorize.firstCommunicate(pop, data);
		    _migrate.firstCommunicate(pop, data);

		    while ( _tocontinue )
		    // while ( ( _tocontinue = _checkpoint(pop) ) )
			{
			    // std::this_thread::sleep_for(std::chrono::microseconds( 10000 ));
			    // std::cout << "* "; std::cout.flush();

			    _evolve.communicate(pop, data);
			    _feedback.communicate(pop, data);
			    _probasend.communicate(pop, data); // juste pour stat
			    _update.communicate(pop, data);
			    _memorize.communicate(pop, data);
			    _migrate.communicate(pop, data);


			    /*************************************************************************
			     * MAJ de la matrice de transition et récupération des vecteurs des iles *
			     *************************************************************************/

			    // if ( this->rank() > 0 )
			    // 	{
			    // 	    this->world().send( 0, 42, data.proba );
			    // 	}
			    // else
			    // 	{
			    // 	    for (size_t i = 1; i < this->size(); ++i)
			    // 		{
			    // 		    std::vector<double> proba(this->size());
			    // 		    this->world().recv( i, 42, proba );
			    // 		    for (size_t j = 0; j < proba.size(); ++j)
			    // 			{
			    // 			    _probabilities(i,j) = proba[j];
			    // 			}
			    // 		}
			    // 	    for (size_t j = 0; j < data.proba.size(); ++j)
			    // 		{
			    // 		    _probabilities(0,j) = data.proba[j];
			    // 		}

			    // 	    std::cout << _probabilities; std::cout.flush();
			    // 	}
			}

		    _evolve.lastCommunicate(pop, data);
		    _feedback.lastCommunicate(pop, data);
		    _probasend.lastCommunicate(pop, data);
		    _update.lastCommunicate(pop, data);
		    _memorize.lastCommunicate(pop, data);
		    _migrate.lastCommunicate(pop, data);
		}

		class Compute : public core::Compute
		{
		public:
		    Compute(Base<EOT>& algo, core::Pop<EOT>& pop, core::IslandData<EOT>& data) : _algo(algo), _pop(pop), _data(data) {}

		    void operator()()
		    {
			// std::unique_lock<std::mutex> lk(*(this->pt_m));

			_algo.compute(_pop, _data);
		    }

		private:
		    Base<EOT>& _algo;
		    core::Pop<EOT>& _pop;
		    core::IslandData<EOT>& _data;
		};

		class Communicate : public core::Communicate
		{
		public:
		    Communicate(Base<EOT>& algo, core::Pop<EOT>& pop, core::IslandData<EOT>& data) : _algo(algo), _pop(pop), _data(data) {}

		    void operator()()
		    {
			// std::unique_lock<std::mutex> lk(*(this->pt_m));
			// this->pt_cv->wait(lk); // plus de stabilité entre les deux threads, facultatif à mettre comme param ???

			_algo.communicate(_pop, _data);

			// pt_cv->notify_one(); // notify communication thread one cycle is done, facultatif
		    }

		private:
		    Base<EOT>& _algo;
		    core::Pop<EOT>& _pop;
		    core::IslandData<EOT>& _data;
		};

		void operator()(core::Pop<EOT>& pop, core::IslandData<EOT>& data)
		{
		    Compute computer(*this, pop, data);
		    Communicate communicator(*this, pop, data);

		    core::ThreadsRunner tr;
		    tr.add(computer).add(communicator);

		    tr();
		}

		struct DummyEvolver : public evolver::async::Base<EOT> { void compute(core::Pop<EOT>&, core::IslandData<EOT>&) {} void communicate(core::Pop<EOT>&, core::IslandData<EOT>&) {} } _dummyEvolve;
		struct DummyFeedbacker : public feedbacker::async::Base<EOT> { void compute(core::Pop<EOT>&, core::IslandData<EOT>&) {} void communicate(core::Pop<EOT>&, core::IslandData<EOT>&) {} } _dummyFeedback;
		struct DummyInputProbaSender : public inputprobasender::async::Base<EOT> { void compute(core::Pop<EOT>&, core::IslandData<EOT>&) {} void communicate(core::Pop<EOT>&, core::IslandData<EOT>&) {} } _dummyProbaSend;
		struct DummyVectorUpdater : public vectorupdater::async::Base<EOT> { void compute(core::Pop<EOT>&, core::IslandData<EOT>&) {} void communicate(core::Pop<EOT>&, core::IslandData<EOT>&) {} } _dummyUpdate;
		struct DummyMemorizer : public memorizer::async::Base<EOT> { void firstCompute(core::Pop<EOT>&, core::IslandData<EOT>&) {}; void compute(core::Pop<EOT>&, core::IslandData<EOT>&) {} void communicate(core::Pop<EOT>&, core::IslandData<EOT>&) {} } _dummyMemorize;
		struct DummyMigrator : public migrator::async::Base<EOT> { void compute(core::Pop<EOT>&, core::IslandData<EOT>&) {} void communicate(core::Pop<EOT>&, core::IslandData<EOT>&) {} } _dummyMigrate;

	    private:
		utils::CheckPoint<EOT>& _checkpoint;
		evolver::async::Base<EOT>& _evolve;
		feedbacker::async::Base<EOT>& _feedback;
		inputprobasender::async::Base<EOT>& _probasend;
		vectorupdater::async::Base<EOT>& _update;
		memorizer::async::Base<EOT>& _memorize;
		migrator::async::Base<EOT>& _migrate;

		bool _initG = true;
		size_t _probaSame = 100;

		std::atomic<bool> _tocontinue;
	    };
	} // !async
    } // !algo
} // !dim

#endif /* _ALGO_EASY_H_ */
