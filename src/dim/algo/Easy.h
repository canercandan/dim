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
#include <dim/utils/Measure.h>

namespace dim
{
    namespace algo
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
		Easy(utils::CheckPoint<EOT>& checkpoint) : _checkpoint(checkpoint), _evolve(__dummyEvolve), _feedback(__dummyFeedback), _update(__dummyUpdate), _memorize(__dummyMemorize), _migrate(__dummyMigrate) {}

		Easy(evolver::Base<EOT>& evolver, feedbacker::Base<EOT>& feedbacker, vectorupdater::Base<EOT>& updater, memorizer::Base<EOT>& memorizer, migrator::Base<EOT>& migrator, utils::CheckPoint<EOT>& checkpoint, std::vector< core::Pop<EOT>* >& islandPop, std::vector< core::IslandData<EOT>* >& islandData) : _checkpoint(checkpoint), _evolve(evolver), _feedback(feedbacker), _update(updater), _memorize(memorizer), _migrate(migrator), _islandPop(islandPop), _islandData(islandData) {}

		virtual ~Easy() {}

		void operator()(core::Pop<EOT>& __pop, core::IslandData<EOT>& __data)
		{
		    core::Pop<EOT>& pop = *(_islandPop[this->rank()]);
		    core::IslandData<EOT>& data = *(_islandData[this->rank()]);

		    std::map<std::string, std::ofstream*> measureFiles;

#ifdef MEASURE
		    std::ostringstream ss;

		    ss.str(""); ss << data.monitorPrefix << ".total.time." << this->rank();
		    measureFiles["total"] = new std::ofstream(ss.str().c_str());

		    ss.str(""); ss << data.monitorPrefix << ".gen.time." << this->rank();
		    measureFiles["gen"] = new std::ofstream(ss.str().c_str());

		    ss.str(""); ss << data.monitorPrefix << ".gen_sync.time." << this->rank();
		    measureFiles["gen_sync"] = new std::ofstream(ss.str().c_str());

		    ss.str(""); ss << data.monitorPrefix << ".evolve.time." << this->rank();
		    measureFiles["evolve"] = new std::ofstream(ss.str().c_str());

		    ss.str(""); ss << data.monitorPrefix << ".feedback.time." << this->rank();
		    measureFiles["feedback"] = new std::ofstream(ss.str().c_str());

		    ss.str(""); ss << data.monitorPrefix << ".update.time." << this->rank();
		    measureFiles["update"] = new std::ofstream(ss.str().c_str());

		    ss.str(""); ss << data.monitorPrefix << ".memorize.time." << this->rank();
		    measureFiles["memorize"] = new std::ofstream(ss.str().c_str());

		    ss.str(""); ss << data.monitorPrefix << ".migrate.time." << this->rank();
		    measureFiles["migrate"] = new std::ofstream(ss.str().c_str());
#endif // !MEASURE

		    DO_MEASURE(

			       _evolve.firstCall(pop, data);
			       _feedback.firstCall(pop, data);
			       _update.firstCall(pop, data);
			       _memorize.firstCall(pop, data);
			       _migrate.firstCall(pop, data);

			       while ( ( __data.toContinue = __data.toContinue & _checkpoint(pop) ) )
				   {
				       DO_MEASURE(

						  DO_MEASURE( DO_MEASURE(_evolve(pop, data), measureFiles, "evolve");
							      DO_MEASURE(_feedback(pop, __data), measureFiles, "feedback");
							      DO_MEASURE(_update(pop, data), measureFiles, "update");
							      DO_MEASURE(_memorize(pop, data), measureFiles, "memorize");
							      DO_MEASURE(_migrate(pop, __data), measureFiles, "migrate");
							      , measureFiles, "gen" );

						  // std::cout << data.migratorReceivingQueue.size() << std::endl; std::cout.flush();

						  , measureFiles, "gen_sync" );
				   }

			       _evolve.lastCall(pop, data);
			       _feedback.lastCall(pop, data);
			       _update.lastCall(pop, data);
			       _memorize.lastCall(pop, data);
			       _migrate.lastCall(pop, data);

			       , measureFiles, "total" );

#ifdef MEASURE
		    for ( std::map<std::string, std::ofstream*>::iterator it = measureFiles.begin(); it != measureFiles.end(); ++it )
			{
			    delete it->second;
			}
#endif // !MEASURE

		    std::cout << "end" << std::endl; std::cout.flush();
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

		std::vector< core::Pop<EOT>* >& _islandPop;
		std::vector< core::IslandData<EOT>* >& _islandData;
	    };
	} // !smp

	template <typename EOT>
	class Easy : public Base<EOT>
	{
	public:
	    Easy(utils::CheckPoint<EOT>& checkpoint) : _checkpoint(checkpoint), _evolve(__dummyEvolve), _feedback(__dummyFeedback), _update(__dummyUpdate), _memorize(__dummyMemorize), _migrate(__dummyMigrate) {}

	    Easy(evolver::Base<EOT>& evolver, feedbacker::Base<EOT>& feedbacker, vectorupdater::Base<EOT>& updater, memorizer::Base<EOT>& memorizer, migrator::Base<EOT>& migrator, utils::CheckPoint<EOT>& checkpoint) : _checkpoint(checkpoint), _evolve(evolver), _feedback(feedbacker), _update(updater), _memorize(memorizer), _migrate(migrator) {}

	    virtual ~Easy() {}

	    void operator()(core::Pop<EOT>& pop, core::IslandData<EOT>& data)
	    {
		std::map<std::string, std::ofstream*> measureFiles;

#ifdef MEASURE
		std::ostringstream ss;

		ss.str(""); ss << data.monitorPrefix << ".total.time." << this->rank();
		measureFiles["total"] = new std::ofstream(ss.str().c_str());

		ss.str(""); ss << data.monitorPrefix << ".gen.time." << this->rank();
		measureFiles["gen"] = new std::ofstream(ss.str().c_str());

		ss.str(""); ss << data.monitorPrefix << ".evolve.time." << this->rank();
		measureFiles["evolve"] = new std::ofstream(ss.str().c_str());

		ss.str(""); ss << data.monitorPrefix << ".feedback.time." << this->rank();
		measureFiles["feedback"] = new std::ofstream(ss.str().c_str());

		ss.str(""); ss << data.monitorPrefix << ".update.time." << this->rank();
		measureFiles["update"] = new std::ofstream(ss.str().c_str());

		ss.str(""); ss << data.monitorPrefix << ".memorize.time." << this->rank();
		measureFiles["memorize"] = new std::ofstream(ss.str().c_str());

		ss.str(""); ss << data.monitorPrefix << ".migrate.time." << this->rank();
		measureFiles["migrate"] = new std::ofstream(ss.str().c_str());
#endif // !MEASURE

		DO_MEASURE(

			   _evolve.firstCall(pop, data);
			   _feedback.firstCall(pop, data);
			   _update.firstCall(pop, data);
			   _memorize.firstCall(pop, data);
			   _migrate.firstCall(pop, data);


			   while ( ( data.toContinue = _checkpoint(pop) ) )
			       {
				   DO_MEASURE( DO_MEASURE(_evolve(pop, data), measureFiles, "evolve");
					       DO_MEASURE(_feedback(pop, data), measureFiles, "feedback");
					       DO_MEASURE(_update(pop, data), measureFiles, "update");
					       DO_MEASURE(_memorize(pop, data), measureFiles, "memorize");
					       DO_MEASURE(_migrate(pop, data), measureFiles, "migrate");
					       , measureFiles, "gen" );
			       }

			   _evolve.lastCall(pop, data);
			   _feedback.lastCall(pop, data);
			   _update.lastCall(pop, data);
			   _memorize.lastCall(pop, data);
			   _migrate.lastCall(pop, data);

			   , measureFiles, "total" );

#ifdef MEASURE
		for ( std::map<std::string, std::ofstream*>::iterator it = measureFiles.begin(); it != measureFiles.end(); ++it )
		    {
			delete it->second;
		    }
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
	};
    } // !algo
} // !dim

#endif /* _ALGO_EASY_H_ */
