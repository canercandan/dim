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

#include <eo>
#include <string>
#include <iostream>
#include <vector>

#include <dim/utils/utils>
#include <dim/variation/IncrementalEval.h>

namespace dim
{
    namespace do_make
	{

	    template <typename EOT>
	    utils::CheckPoint<EOT>& checkpoint(eoParser& _parser, eoState& _state, continuator::Base<EOT>& _continue, variation::IncrementalEvalCounter<EOT>& _eval, core::IslandData<EOT>& data, unsigned _frequency = 1, unsigned stepTimer = 1000 )
	    {
		const size_t ALL = data.size();
		const size_t RANK = data.rank();

		bool printBest = _parser.getORcreateParam(false, "printBestStat", "Print Best/avg/stdev every gen.", '\0', "Output").value();
		std::string monitorPrefix = _parser.getORcreateParam(std::string("result"), "monitorPrefix", "Monitor prefix filenames", '\0', "Output").value();

		utils::CheckPoint<EOT>& checkpoint = _state.storeFunctor( new utils::CheckPoint<EOT>( _continue ) );

		std::ostringstream ss_prefix;
		ss_prefix << monitorPrefix << "_monitor_" << RANK;
		utils::FileMonitor& fileMonitor = _state.storeFunctor( new utils::FileMonitor( ss_prefix.str(), _frequency, ",", 0, false, true, false, stepTimer ) );
		checkpoint.add(fileMonitor);

		utils::StdoutMonitor* stdMonitor = NULL;
		if (printBest)
		    {
			stdMonitor = new utils::StdoutMonitor("\t", 8, ' ', stepTimer);
			_state.storeFunctor( stdMonitor );
			checkpoint.add(*stdMonitor);
		    }

		std::ostringstream ss;

		utils::FixedValue<unsigned int>& islNum = _state.storeFunctor( new utils::FixedValue<unsigned int>( RANK, "Island" ) );
		fileMonitor.add(islNum);
		if (printBest) { stdMonitor->add(islNum); }

		utils::TimeCounter& tCounter = _state.storeFunctor( new utils::TimeCounter );
		checkpoint.add(tCounter);
		fileMonitor.add(tCounter);
		if (printBest) { stdMonitor->add(tCounter); }

		utils::DateTimeCounter& dtCounter = _state.storeFunctor( new utils::DateTimeCounter );
		checkpoint.add(dtCounter);
		fileMonitor.add(dtCounter);
		if (printBest) { stdMonitor->add(dtCounter); }

		utils::GenCounter& genCounter = _state.storeFunctor( new utils::GenCounter( 0, "migration" ) );
		checkpoint.add(genCounter);
		fileMonitor.add(genCounter);
		if (printBest) { stdMonitor->add(genCounter); }

		ss.str(""); ss << "inc_evaluation_isl" << RANK;
		utils::IncrementalEvalCounter<EOT>& incrementalEvalCounter = _state.storeFunctor( new utils::IncrementalEvalCounter<EOT>( _eval, ss.str() ) );
		checkpoint.add(incrementalEvalCounter);
		fileMonitor.add(incrementalEvalCounter);
		if (printBest) { stdMonitor->add(incrementalEvalCounter); }

		ss.str(""); ss << "nb_individual_isl" << RANK;
		utils::FuncPtrStat<EOT, size_t>& popSizeStat = utils::makeFuncPtrStat( utils::getPopSize<EOT>, _state, ss.str() );
		checkpoint.add(popSizeStat);
		fileMonitor.add(popSizeStat);
		if (printBest) { stdMonitor->add(popSizeStat); }

		ss.str(""); ss << "sending_queue_size_isl" << RANK;
		utils::GetMigratorSendingQueueSize<EOT>& sendingQueueSizeFunc = _state.storeFunctor( new utils::GetMigratorSendingQueueSize<EOT>( data ) );
		utils::FunctorStat<EOT, size_t>& sendingQueueSizeStat = utils::makeFunctorStat( sendingQueueSizeFunc, _state, ss.str() );
		checkpoint.add(sendingQueueSizeStat);
		fileMonitor.add(sendingQueueSizeStat);
		if (printBest) { stdMonitor->add(sendingQueueSizeStat); }

		ss.str(""); ss << "receiving_queue_size_isl" << RANK;
		utils::GetMigratorReceivingQueueSize<EOT>& receivingQueueSizeFunc = _state.storeFunctor( new utils::GetMigratorReceivingQueueSize<EOT>( data ) );
		utils::FunctorStat<EOT, size_t>& receivingQueueSizeStat = utils::makeFunctorStat( receivingQueueSizeFunc, _state, ss.str() );
		checkpoint.add(receivingQueueSizeStat);
		fileMonitor.add(receivingQueueSizeStat);
		if (printBest) { stdMonitor->add(receivingQueueSizeStat); }

		ss.str(""); ss << "avg_ones_isl" << RANK;
		utils::AverageStat<EOT>& avg = _state.storeFunctor( new utils::AverageStat<EOT>( ss.str() ) );
		checkpoint.add(avg);
		fileMonitor.add(avg);
		if (printBest) { stdMonitor->add(avg); }

		ss.str(""); ss << "delta_avg_ones_isl" << RANK;
		utils::AverageDeltaFitnessStat<EOT>& avg_delta = _state.storeFunctor( new utils::AverageDeltaFitnessStat<EOT>( ss.str() ) );
		checkpoint.add(avg_delta);
		fileMonitor.add(avg_delta);
		if (printBest) { stdMonitor->add(avg_delta); }

		ss.str(""); ss << "best_value_isl" << RANK;
		utils::BestFitnessStat<EOT>& best = _state.storeFunctor( new utils::BestFitnessStat<EOT>( ss.str() ) );
		checkpoint.add(best);
		fileMonitor.add(best);
		if (printBest) { stdMonitor->add(best); }

		ss.str(""); ss << "std_value_isl" << RANK;
		utils::StdevStat<EOT>& std = _state.storeFunctor( new utils::StdevStat<EOT>( ss.str() ) );
		checkpoint.add(std);
		fileMonitor.add(std);
		if (printBest) { stdMonitor->add(std); }

		ss.str(""); ss << "sumofsquares_value_isl" << RANK;
		utils::SumOfSquares<EOT>& sumOfSquares = _state.storeFunctor( new utils::SumOfSquares<EOT>( ss.str() ) );
		checkpoint.add(sumOfSquares);
		fileMonitor.add(sumOfSquares);
		if (printBest) { stdMonitor->add(sumOfSquares); }

		ss.str(""); ss << "distance_value_isl" << RANK;
		utils::DistanceStat<EOT>& distance = _state.storeFunctor( new utils::DistanceStat<EOT>( ss.str() ) );
		checkpoint.add(distance);
		fileMonitor.add(distance);
		if (printBest) { stdMonitor->add(distance); }

		ss.str(""); ss << "iqr_value_isl" << RANK;
		utils::InterquartileRangeStat<EOT>& iqr = _state.storeFunctor( new utils::InterquartileRangeStat<EOT>( 0.0, ss.str() ) );
		checkpoint.add(iqr);
		fileMonitor.add(iqr);
		if (printBest) { stdMonitor->add(iqr); }

		ss.str(""); ss << "nb_input_ind_isl" << RANK;
		utils::FuncPtrStat<EOT, size_t>& inputSizeStat = utils::makeFuncPtrStat( utils::getPopInputSize<EOT>, _state, ss.str() );
		checkpoint.add(inputSizeStat);
		fileMonitor.add(inputSizeStat);
		if (printBest) { stdMonitor->add(inputSizeStat); }

		ss.str(""); ss << "nb_output_ind_isl" << RANK;
		utils::FuncPtrStat<EOT, size_t>& outputSizeStat = utils::makeFuncPtrStat( utils::getPopOutputSize<EOT>, _state, ss.str() );
		checkpoint.add(outputSizeStat);
		fileMonitor.add(outputSizeStat);
		if (printBest) { stdMonitor->add(outputSizeStat); }

		for (size_t i = 0; i < data.proba.size(); ++i)
		    {
			ss.str(""); ss << "P" << RANK << "to" << i;
			utils::GetMigrationProbability<EOT>& migProba = _state.storeFunctor( new utils::GetMigrationProbability<EOT>( data.proba, i, ss.str() ) );
			checkpoint.add(migProba);
			fileMonitor.add(migProba);
			if (printBest) { stdMonitor->add(migProba); }
		    }

		// TODO: just added temporarely to make statistic scripts working, but HAVE TO be removed
		ss.str(""); ss << "P" << RANK << "to*";
		utils::GetSumVectorProbability<EOT>& migProba = _state.storeFunctor( new utils::GetSumVectorProbability<EOT>( data.proba, ss.str() ) );
		checkpoint.add(migProba);
		fileMonitor.add(migProba);
		if (printBest) { stdMonitor->add(migProba); }
		// END TODO

		// TODO: just added temporarely to make statistic scripts working, but HAVE TO be removed
		ss.str(""); ss << "P" << "*to" << RANK;
		utils::FixedValue<unsigned int>& migProbaRet = _state.storeFunctor( new utils::FixedValue<unsigned int>( 0, ss.str() ) );
		fileMonitor.add(migProbaRet);
		if (printBest) { stdMonitor->add(migProbaRet); }
		// END TODO

		for (size_t i = 0; i < data.feedbacks.size(); ++i)
		    {
			ss.str(""); ss << "F" << RANK << "to" << i;
			utils::GetFeedbacks<EOT>& feedbacks = _state.storeFunctor( new utils::GetFeedbacks<EOT>( data.feedbacks, i, ss.str() ) );
			checkpoint.add(feedbacks);
			fileMonitor.add(feedbacks);
			if (printBest) { stdMonitor->add(feedbacks); }
		    }

		for (size_t i = 0; i < ALL; ++i)
		    {
			ss.str(""); ss << "nb_migrants_isl" << RANK << "to" << i;
			utils::OutputSizePerIsland<EOT>& out = _state.storeFunctor( new utils::OutputSizePerIsland<EOT>( i, ss.str() ) );
			checkpoint.add(out);
			fileMonitor.add(out);
			if (printBest) { stdMonitor->add(out); }
		    }

		return checkpoint;
	    }

    } // !do_make
} // !dim

#endif // !_DO_MAKE_CHECKPOINT_H_
