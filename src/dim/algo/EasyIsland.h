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

#ifndef _ALGO_EASYISLAND_H_
#define _ALGO_EASYISLAND_H_

// For time mesuring
#include <chrono>
#include <sstream>
#include <fstream>

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

	template <typename EOT>
	class EasyIsland : public Base<EOT>
	{
	public:
	    EasyIsland(utils::CheckPoint<EOT>& checkpoint) : _checkpoint(checkpoint), _evolve(_dummyEvolve), _feedback(_dummyFeedback), _probasend(_dummyProbaSend), _update(_dummyUpdate), _memorize(_dummyMemorize), _migrate(_dummyMigrate) {}

	    EasyIsland(evolver::Base<EOT>& evolver, feedbacker::Base<EOT>& feedbacker, inputprobasender::Base<EOT>& probasender, dim::vectorupdater::Easy<EOT>& updater, dim::memorizer::Easy<EOT>& memorizer, migrator::Base<EOT>& migrator, utils::CheckPoint<EOT>& checkpoint) : _checkpoint(checkpoint), _evolve(evolver), _feedback(feedbacker), _probasend(probasender), _update(updater), _memorize(memorizer), _migrate(migrator) {}

	    virtual ~EasyIsland() {}

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
			_probasend(pop, data);
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
	    struct DummyEvolver : public evolver::Base<EOT> { void operator()(core::Pop<EOT>&, core::IslandData<EOT>&) {} } _dummyEvolve;
	    struct DummyFeedbacker : public feedbacker::Base<EOT> { void operator()(core::Pop<EOT>&, core::IslandData<EOT>&) {} } _dummyFeedback;
	    struct DummyInputProbaSender : public inputprobasender::Base<EOT> { void operator()(core::Pop<EOT>&, core::IslandData<EOT>&) {} } _dummyProbaSend;
	    struct DummyVectorUpdater : public vectorupdater::Base<EOT> { void operator()(core::Pop<EOT>&, core::IslandData<EOT>&) {} } _dummyUpdate;
	    struct DummyMemorizer : public memorizer::Base<EOT> { void firstCall(core::Pop<EOT>&, core::IslandData<EOT>&) {}; void operator()(core::Pop<EOT>&, core::IslandData<EOT>&) {} } _dummyMemorize;
	    struct DummyMigrator : public migrator::Base<EOT> { void operator()(core::Pop<EOT>&, core::IslandData<EOT>&) {} } _dummyMigrate;

	    utils::CheckPoint<EOT>& _checkpoint;
	    evolver::Base<EOT>& _evolve;
	    feedbacker::Base<EOT>& _feedback;
	    inputprobasender::Base<EOT>& _probasend;
	    vectorupdater::Base<EOT>& _update;
	    memorizer::Base<EOT>& _memorize;
	    migrator::Base<EOT>& _migrate;

	    bool _initG = true;
	    size_t _probaSame = 100;
	};

    } // !algo
} // !dim

#endif /* _ALGO_EASYISLAND_H_ */
