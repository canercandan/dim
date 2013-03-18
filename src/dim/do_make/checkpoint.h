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

namespace dim
{
    namespace do_make
	{

	    template <class EOT>
	    utils::CheckPoint<EOT>& checkpoint(eoParser& _parser, eoState& _state, continuator::Base<EOT>& _continue, core::IslandData<EOT>& data, unsigned _frequency = 1 )
	    {
		const size_t ALL = data.size();
		const size_t RANK = data.rank();

		bool printBest = _parser.createParam(false, "printBestStat", "Print Best/avg/stdev every gen.", '\0', "Output").value();
		std::string monitorPrefix = _parser.createParam(std::string("result"), "monitorPrefix", "Monitor prefix filenames", '\0', "Output").value();

		utils::CheckPoint<EOT>& checkpoint = _state.storeFunctor( new utils::CheckPoint<EOT>( _continue ) );

		std::ostringstream ss_prefix;
		ss_prefix << monitorPrefix << "_monitor_" << RANK;
		utils::FileMonitor& fileMonitor = _state.storeFunctor( new utils::FileMonitor( ss_prefix.str(), _frequency, " ", 0, false, true ) );
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

		// utils::GenCounter& genCounter = _state.storeFunctor( new utils::GenCounter( 0, "migration" ) );
		// checkpoint.add(genCounter);
		// fileMonitor.add(genCounter);
		// if (printBest) { stdMonitor->add(genCounter); }

		// std::ostringstream ss_size;
		// ss_size << "nb_individual_isl" << RANK;
		// utils::FuncPtrStat<EOT, size_t>& popSizeStat = utils::makeFuncPtrStat( utils::getPopSize<EOT>, _state, ss_size.str() );
		// checkpoint.add(popSizeStat);
		// fileMonitor.add(popSizeStat);
		// if (printBest) { stdMonitor->add(popSizeStat); }

		// std::ostringstream ss_sending_queue_size;
		// ss_sending_queue_size << "sending_queue_size_isl" << RANK;
		// utils::GetMigratorSendingQueueSize<EOT>& sendingQueueSizeFunc = _state.storeFunctor( new utils::GetMigratorSendingQueueSize<EOT>( data ) );
		// utils::FunctorStat<EOT, size_t>& sendingQueueSizeStat = utils::makeFunctorStat( sendingQueueSizeFunc, _state, ss_sending_queue_size.str() );
		// checkpoint.add(sendingQueueSizeStat);
		// fileMonitor.add(sendingQueueSizeStat);
		// if (printBest) { stdMonitor->add(sendingQueueSizeStat); }

		std::ostringstream ss_receiving_queue_size;
		ss_receiving_queue_size << "receiving_queue_size_isl" << RANK;
		utils::GetMigratorReceivingQueueSize<EOT>& receivingQueueSizeFunc = _state.storeFunctor( new utils::GetMigratorReceivingQueueSize<EOT>( data ) );
		utils::FunctorStat<EOT, size_t>& receivingQueueSizeStat = utils::makeFunctorStat( receivingQueueSizeFunc, _state, ss_receiving_queue_size.str() );
		checkpoint.add(receivingQueueSizeStat);
		fileMonitor.add(receivingQueueSizeStat);
		if (printBest) { stdMonitor->add(receivingQueueSizeStat); }

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
			utils::GetMigrationProbability<EOT>& migProba = _state.storeFunctor( new utils::GetMigrationProbability<EOT>( data.proba, i, ss.str() ) );
			checkpoint.add(migProba);
			fileMonitor.add(migProba);
			if (printBest) { stdMonitor->add(migProba); }
		    }

		// TODO: just added temporarely to make statistic scripts working, but HAVE TO be removed
		std::ostringstream ss_proba;
		ss_proba << "P" << "*to" << RANK;
		utils::FixedValue<unsigned int>& migProbaRet = _state.storeFunctor( new utils::FixedValue<unsigned int>( 0, ss_proba.str() ) );
		fileMonitor.add(migProbaRet);
		if (printBest) { stdMonitor->add(migProbaRet); }
		// END TODO

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
