
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
#if __cplusplus > 199711L
#include <chrono>
#else
#include <boost/chrono/chrono_io.hpp>
#endif

#include <sstream>
#include <fstream>

#include <dim/core/core>
#include <dim/utils/utils>
#include <dim/evolver/evolver>
#include <dim/feedbacker/feedbacker>
#include <dim/vectorupdater/vectorupdater>
#include <dim/memorizer/memorizer>
#include <dim/migrator/migrator>

#include "Base.h"

#ifdef MEASURE
# define DO_MEASURE(op, f)						\
    {									\
	std_or_boost::chrono::time_point< std_or_boost::chrono::system_clock > start = std_or_boost::chrono::system_clock::now(); \
	op;								\
	std_or_boost::chrono::time_point< std_or_boost::chrono::system_clock > end = std_or_boost::chrono::system_clock::now();	\
	unsigned elapsed = std_or_boost::chrono::duration_cast<std_or_boost::chrono::microseconds>(end-start).count(); \
	f << elapsed << " "; f.flush();					\
    }
#else
# define DO_MEASURE(op, f) { op; }
#endif // !MEASURE

namespace dim
{
    namespace algo
    {
#if __cplusplus > 199711L
	namespace std_or_boost = std;
#else
	namespace std_or_boost = boost;
#endif

	template <typename EOT>
	class Easy : public Base<EOT>
	{
	public:
	    Easy(utils::CheckPoint<EOT>& checkpoint) : _checkpoint(checkpoint), _evolve(__dummyEvolve), _feedback(__dummyFeedback), _update(__dummyUpdate), _memorize(__dummyMemorize), _migrate(__dummyMigrate), _tocontinue(true) {}

	    Easy(evolver::Base<EOT>& evolver, feedbacker::Base<EOT>& feedbacker, vectorupdater::Base<EOT>& updater, memorizer::Base<EOT>& memorizer, migrator::Base<EOT>& migrator, utils::CheckPoint<EOT>& checkpoint, bool barrier = false) : _checkpoint(checkpoint), _evolve(evolver), _feedback(feedbacker), _update(updater), _memorize(memorizer), _migrate(migrator), _tocontinue(true), _barrier(barrier) {}

	    virtual ~Easy() {}

	    void operator()(core::Pop<EOT>& pop, core::IslandData<EOT>& data)
	    {
#ifdef MEASURE
		std::ostringstream ss;

		ss.str(""); ss << "total.time." << this->rank();
		std::ofstream total_time(ss.str());

		ss.str(""); ss << "gen.time." << this->rank();
		std::ofstream gen_time(ss.str());

		ss.str(""); ss << "evolve.time." << this->rank();
		std::ofstream evolve_time(ss.str());

		ss.str(""); ss << "feedback.time." << this->rank();
		std::ofstream feedback_time(ss.str());

		ss.str(""); ss << "update.time." << this->rank();
		std::ofstream update_time(ss.str());

		ss.str(""); ss << "memorize.time." << this->rank();
		std::ofstream memorize_time(ss.str());

		ss.str(""); ss << "migrate.time." << this->rank();
		std::ofstream migrate_time(ss.str());

		DO_MEASURE(

			   _evolve.firstCall(pop, data);
			   _feedback.firstCall(pop, data);
			   _update.firstCall(pop, data);
			   _memorize.firstCall(pop, data);
			   _migrate.firstCall(pop, data);


			   while ( ( data.toContinue = _checkpoint(pop) ) )
			       {
				   DO_MEASURE( DO_MEASURE(_evolve(pop, data), evolve_time);
					       DO_MEASURE(_feedback(pop, data), feedback_time);
					       DO_MEASURE(_update(pop, data), update_time);
					       DO_MEASURE(_memorize(pop, data), memorize_time);
					       DO_MEASURE(_migrate(pop, data), migrate_time);
					       , gen_time );
			       }

			   _evolve.lastCall(pop, data);
			   _feedback.lastCall(pop, data);
			   _update.lastCall(pop, data);
			   _memorize.lastCall(pop, data);
			   _migrate.lastCall(pop, data);

			   if (_barrier)
			       {
				   this->world().barrier();
			       }

			   , total_time );
#else // MEASURE
		_evolve.firstCall(pop, data);
		_feedback.firstCall(pop, data);
		_update.firstCall(pop, data);
		_memorize.firstCall(pop, data);
		_migrate.firstCall(pop, data);


		while ( ( data.toContinue = _checkpoint(pop) ) )
		    {
			_evolve(pop, data);
			_feedback(pop, data);
			_update(pop, data);
			_memorize(pop, data);
			_migrate(pop, data);

			if (_barrier)
			    {
				this->world().barrier();
			    }
		    }

		_evolve.lastCall(pop, data);
		_feedback.lastCall(pop, data);
		_update.lastCall(pop, data);
		_memorize.lastCall(pop, data);
		_migrate.lastCall(pop, data);
#endif // !MEASURE
	    }

	public:
	    struct DummyEvolver : public evolver::Base<EOT> { void operator()(core::Pop<EOT>&, core::IslandData<EOT>&) {} } __dummyEvolve;
	    struct DummyFeedbacker : public feedbacker::Base<EOT> { void operator()(core::Pop<EOT>&, core::IslandData<EOT>&) {} } __dummyFeedback;
	    struct DummyVectorUpdater : public vectorupdater::Base<EOT> { void operator()(core::Pop<EOT>&, core::IslandData<EOT>&) {} } __dummyUpdate;
	    struct DummyMemorizer : public memorizer::Base<EOT> { void firstCall(core::Pop<EOT>&, core::IslandData<EOT>&) {}; void operator()(core::Pop<EOT>&, core::IslandData<EOT>&) {} } __dummyMemorize;
	    struct DummyMigrator : public migrator::Base<EOT> { void operator()(core::Pop<EOT>&, core::IslandData<EOT>&) {} } __dummyMigrate;

	private:
	    utils::CheckPoint<EOT>& _checkpoint;
	    evolver::Base<EOT>& _evolve;
	    feedbacker::Base<EOT>& _feedback;
	    vectorupdater::Base<EOT>& _update;
	    memorizer::Base<EOT>& _memorize;
	    migrator::Base<EOT>& _migrate;

	    std_or_boost::atomic<bool> _tocontinue;
	    bool _barrier;
	};
    } // !algo
} // !dim

#endif /* _ALGO_EASY_H_ */
