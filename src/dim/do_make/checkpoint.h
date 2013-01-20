// -*- mode: c++; c-indent-level: 4; c++-member-init-indent: 8; comment-column: 35; -*-

//-----------------------------------------------------------------------------
// checkpoint_dim.h
// (c) Maarten Keijzer, Marc Schoenauer and GeNeura Team, 2000
/*
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

  Contact: todos@geneura.ugr.es, http://geneura.ugr.es
  Marc.Schoenauer@polytechnique.fr
  mkeijzer@dhi.dk
*/
//-----------------------------------------------------------------------------

#ifndef _DO_MAKE_CHECKPOINT_H_
#define _DO_MAKE_CHECKPOINT_H_

#include <boost/mpi.hpp>

#include <eo>
#include <string>
#include <iostream>
#include <vector>

#include <dim/utils/utils>

namespace dim
{
    namespace do_make
	{

	    template <class EOT>
	    utils::CheckPoint<EOT>& checkpoint(eoParser& _parser, eoState& _state, continuator::Base<EOT>& _continue, core::IslandData<EOT>& data )
	    {

		// Need to know a few MPI data
		boost::mpi::communicator world;
		const size_t ALL = world.size();
		const size_t RANK = world.rank();

		bool printBest = _parser.createParam(false, "printBestStat", "Print Best/avg/stdev every gen.", '\0', "Output").value();
		std::string monitorPrefix = _parser.createParam(std::string("result"), "monitorPrefix", "Monitor prefix filenames", '\0', "Output").value();

		utils::CheckPoint<EOT>& checkpoint = _state.storeFunctor( new utils::CheckPoint<EOT>( _continue ) );

		std::ostringstream ss_prefix;
		ss_prefix << monitorPrefix << "_monitor_" << RANK;
		utils::FileMonitor& fileMonitor = _state.storeFunctor( new utils::FileMonitor( ss_prefix.str(), " ", false, true ) );
		checkpoint.add(fileMonitor);

		utils::StdoutMonitor* stdMonitor = NULL;
		if (printBest)
		    {
			stdMonitor = new utils::StdoutMonitor("\t", 8);
			_state.storeFunctor( stdMonitor );
			checkpoint.add(*stdMonitor);
		    }

		utils::FixedValue<unsigned int>& islNum = _state.storeFunctor( new utils::FixedValue<unsigned int>( RANK, "Island" ) );
		fileMonitor.add(islNum);
		if (printBest) { stdMonitor->add(islNum); }

		utils::TimeCounter& tCounter = _state.storeFunctor( new utils::TimeCounter );
		checkpoint.add(tCounter);
		fileMonitor.add(tCounter);
		if (printBest) { stdMonitor->add(tCounter); }

		utils::GenCounter& genCounter = _state.storeFunctor( new utils::GenCounter( 0, "migration" ) );
		checkpoint.add(genCounter);
		fileMonitor.add(genCounter);
		if (printBest) { stdMonitor->add(genCounter); }

		std::ostringstream ss_size;
		ss_size << "nb_individual_isl" << RANK;
		utils::FuncPtrStat<EOT, size_t>& popSizeStat = utils::makeFuncPtrStat( utils::getPopSize<EOT>, _state, ss_size.str() );
		checkpoint.add(popSizeStat);
		fileMonitor.add(popSizeStat);
		if (printBest) { stdMonitor->add(popSizeStat); }

		std::ostringstream ss_avg;
		ss_avg << "avg_ones_isl" << RANK;
		utils::AverageStat<EOT>& avg = _state.storeFunctor( new utils::AverageStat<EOT>( ss_avg.str() ) );
		checkpoint.add(avg);
		fileMonitor.add(avg);
		if (printBest) { stdMonitor->add(avg); }

		std::ostringstream ss_delta;
		ss_delta << "delta_avg_ones_isl" << RANK;
		utils::AverageDeltaFitnessStat<EOT>& avg_delta = _state.storeFunctor( new utils::AverageDeltaFitnessStat<EOT>( ss_delta.str() ) );
		checkpoint.add(avg_delta);
		fileMonitor.add(avg_delta);
		if (printBest) { stdMonitor->add(avg_delta); }

		std::ostringstream ss_best;
		ss_best << "best_value_isl" << RANK;
		utils::BestFitnessStat<EOT>& best = _state.storeFunctor( new utils::BestFitnessStat<EOT>( ss_best.str() ) );
		checkpoint.add(best);
		fileMonitor.add(best);
		if (printBest) { stdMonitor->add(best); }

		std::ostringstream ss_input_size;
		ss_input_size << "nb_input_ind_isl" << RANK;
		utils::FuncPtrStat<EOT, size_t>& inputSizeStat = utils::makeFuncPtrStat( utils::getPopInputSize<EOT>, _state, ss_input_size.str() );
		checkpoint.add(inputSizeStat);
		fileMonitor.add(inputSizeStat);
		if (printBest) { stdMonitor->add(inputSizeStat); }

		std::ostringstream ss_output_size;
		ss_output_size << "nb_output_ind_isl" << RANK;
		utils::FuncPtrStat<EOT, size_t>& outputSizeStat = utils::makeFuncPtrStat( utils::getPopOutputSize<EOT>, _state, ss_output_size.str() );
		checkpoint.add(outputSizeStat);
		fileMonitor.add(outputSizeStat);
		if (printBest) { stdMonitor->add(outputSizeStat); }

		for (size_t i = 0; i < data.proba.size(); ++i)
		    {
			std::ostringstream ss;
			ss << "P" << RANK << "to" << i;
			utils::GetMigrationProbability& migProba = _state.storeFunctor( new utils::GetMigrationProbability( data.proba, i, ss.str() ) );
			checkpoint.add(migProba);
			fileMonitor.add(migProba);
			if (printBest) { stdMonitor->add(migProba); }
		    }

		std::ostringstream ss_proba;
		ss_proba << "P" << RANK << "to*";
		utils::GetMigrationProbabilityRet& migProbaRet = _state.storeFunctor( new utils::GetMigrationProbabilityRet( data.probaret, ss_proba.str() ) );
		checkpoint.add(migProbaRet);
		fileMonitor.add(migProbaRet);
		if (printBest) { stdMonitor->add(migProbaRet); }

		for (size_t i = 0; i < ALL; ++i)
		    {
			std::ostringstream ss;
			ss << "nb_migrants_isl" << RANK << "to" << i;
			utils::OutputSizePerIsland<EOT>& out = _state.storeFunctor( new utils::OutputSizePerIsland<EOT>( i, ss.str() ) );
			checkpoint.add(out);
			fileMonitor.add(out);
			if (printBest) { stdMonitor->add(out); }
		    }

		return checkpoint;
	    }

    } // !do
} // !dim

#endif // !_DO_MAKE_CHECKPOINT_H_
