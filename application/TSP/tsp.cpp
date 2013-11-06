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

#include <boost/algorithm/string.hpp>
#include <boost/mpi.hpp>
#include <eo>
#include <eoSwapMutation.h>
#include <eoShiftMutation.h>
#include <eoInversionMutation.h>
#include <eoTwoOptMutation.h>
#include <dim/dim>
#include <dim/algo/Easy.h>
#include <dim/evolver/Easy.h>
#include <dim/feedbacker/Easy.h>
#include <dim/migrator/Easy.h>
#include <dim/vectorupdater/Easy.h>
#include <dim/core/State.h>

#if __cplusplus > 199711L
namespace std_or_boost = std;
#include <mutex>
#include <condition_variable>
#else
#include <boost/thread/thread.hpp>
#include <boost/chrono/chrono_io.hpp>
#include <boost/thread/mutex.hpp>
namespace std_or_boost = boost;
#endif

typedef dim::representation::Route<double> EOT;

int main (int argc, char *argv[])
{
    /*************************
     * Initialisation de MPI *
     *************************/

    // boost::mpi::environment env(argc, argv, MPI_THREAD_MULTIPLE, true);
    // boost::mpi::communicator world;

    /****************************
     * Il faut au moins 4 nœuds *
     ****************************/

    // const size_t ALL = world.size();
    // const size_t RANK = world.rank();

    /************************
     * Initialisation de EO *
     ************************/

    eoParser parser(argc, argv);
    eoState state;    // keeps all things allocated
    dim::core::State state_dim;    // keeps all things allocated

    /*****************************
     * Definition des paramètres *
     *****************************/

    // N
    unsigned nislands = parser.createParam(unsigned(4), "nislands", "Number of islands (see --smp)", 'N', "Islands Model").value();
    // a
    double alphaP = parser.createParam(double(0.2), "alpha", "Alpha Probability", 'a', "Islands Model").value();
    // A
    double alphaF = parser.createParam(double(0.01), "alphaF", "Alpha Fitness", 'A', "Islands Model").value();
    // b
    double betaP = parser.createParam(double(0.01), "beta", "Beta Probability", 'b', "Islands Model").value();
    // d
    double probaSame = parser.createParam(double(100./nislands), "probaSame", "Probability for an individual to stay in the same island", 'd', "Islands Model").value();
    // I
    bool initG = parser.createParam(bool(true), "initG", "initG", 'I', "Islands Model").value();
    unsigned nmigrations = parser.createParam(unsigned(1), "nmigrations", "Number of migrations to do at each generation (0=all individuals are migrated)", 0, "Islands Model").value();
    unsigned stepTimer = parser.createParam(unsigned(0), "stepTimer", "stepTimer", 0, "Islands Model").value();
    bool deltaUpdate = parser.createParam(bool(true), "deltaUpdate", "deltaUpdate", 0, "Islands Model").value();
    bool deltaFeedback = parser.createParam(bool(true), "deltaFeedback", "deltaFeedback", 0, "Islands Model").value();
    double sensitivity = 1 / parser.createParam(double(1.), "sensitivity", "sensitivity of delta{t} (1/sensitivity)", 0, "Islands Model").value();
    std::string rewardStrategy = parser.createParam(std::string("best"), "rewardStrategy", "Strategy of rewarding: best or avg", 0, "Islands Model").value();

    /*********************************
     * Déclaration des composants EO *
     *********************************/

    std::string tspInstance =  parser.getORcreateParam(std::string("benchs/ali535.xml"), "tspInstance", "filename of the instance for TSP problem", 0, "Problem").value();

    dim::evaluation::Route<double> mainEval;

    unsigned popSize = parser.getORcreateParam(unsigned(100), "popSize", "Population Size", 'P', "Evolution Engine").value();

    double targetFitness = parser.getORcreateParam(double(0), "targetFitness", "Stop when fitness reaches",'T', "Stopping criterion").value();
    unsigned maxGen = parser.getORcreateParam(unsigned(10000), "maxGen", "Maximum number of generations () = none)",'G',"Stopping criterion").value();

    std::string monitorPrefix = parser.getORcreateParam(std::string("result"), "monitorPrefix", "Monitor prefix filenames", '\0', "Output").value();

    std::map< std::string, std::pair< dim::variation::Base<EOT>*, dim::variation::IncrementalEvalCounter<EOT>* > > mapOperators;
    std::vector< std::string > operatorsOrder;

    dim::variation::SwapPartialOp<EOT> swapOp;
    dim::variation::ShiftPartialOp<EOT> shiftOp;
    dim::variation::InversionPartialOp<EOT> inversionOp;

    dim::variation::SwapIncrementalEval<EOT> swapEval;
    dim::variation::ShiftIncrementalEval<EOT> shiftEval;
    dim::variation::InversionIncrementalEval<EOT> inversionEval;
    dim::variation::DummyIncrementalEval<EOT> dummyEval;

    mapOperators["swap"] = std::make_pair(new dim::variation::RandMutation<EOT>(swapOp),
					  new dim::variation::IncrementalEvalCounter<EOT>(dummyEval));
    operatorsOrder.push_back("swap");
    mapOperators["shift"] = std::make_pair(new dim::variation::RandMutation<EOT>(shiftOp),
					   new dim::variation::IncrementalEvalCounter<EOT>(dummyEval));
    operatorsOrder.push_back("shift");
    mapOperators["inversion"] = std::make_pair(new dim::variation::RandMutation<EOT>(inversionOp),
					       new dim::variation::IncrementalEvalCounter<EOT>(dummyEval));;
    operatorsOrder.push_back("inversion");

    dim::variation::IncrementalEvalCounter<EOT>* ptFirstImprovementSwapEvalCounter = new dim::variation::IncrementalEvalCounter<EOT>(swapEval);
    dim::variation::IncrementalEvalCounter<EOT>* ptFirstImprovementShiftEvalCounter = new dim::variation::IncrementalEvalCounter<EOT>(shiftEval);
    dim::variation::IncrementalEvalCounter<EOT>* ptFirstImprovementInversionEvalCounter = new dim::variation::IncrementalEvalCounter<EOT>(inversionEval);

    mapOperators["first_improve_swap"] = std::make_pair(new dim::variation::FirstImprovementMutation<EOT>(swapOp, *ptFirstImprovementSwapEvalCounter), ptFirstImprovementSwapEvalCounter);
    operatorsOrder.push_back("first_improve_swap");
    mapOperators["first_improve_shift"] = std::make_pair(new dim::variation::FirstImprovementMutation<EOT>(shiftOp, *ptFirstImprovementShiftEvalCounter), ptFirstImprovementShiftEvalCounter);
    operatorsOrder.push_back("first_improve_shift");
    mapOperators["first_improve_inversion"] = std::make_pair(new dim::variation::FirstImprovementMutation<EOT>(inversionOp, *ptFirstImprovementInversionEvalCounter), ptFirstImprovementInversionEvalCounter);
    operatorsOrder.push_back("first_improve_inversion");

    dim::variation::IncrementalEvalCounter<EOT>* ptRelativeBestImprovementSwapEvalCounter = new dim::variation::IncrementalEvalCounter<EOT>(swapEval);
    dim::variation::IncrementalEvalCounter<EOT>* ptRelativeBestImprovementShiftEvalCounter = new dim::variation::IncrementalEvalCounter<EOT>(shiftEval);
    dim::variation::IncrementalEvalCounter<EOT>* ptRelativeBestImprovementInversionEvalCounter = new dim::variation::IncrementalEvalCounter<EOT>(inversionEval);

    mapOperators["relative_best_improve_swap"] = std::make_pair(new dim::variation::RelativeBestImprovementMutation<EOT>(swapOp, *ptRelativeBestImprovementSwapEvalCounter), ptRelativeBestImprovementSwapEvalCounter);
    operatorsOrder.push_back("relative_best_improve_swap");
    mapOperators["relative_best_improve_shift"] = std::make_pair(new dim::variation::RelativeBestImprovementMutation<EOT>(shiftOp, *ptRelativeBestImprovementShiftEvalCounter), ptRelativeBestImprovementShiftEvalCounter);
    operatorsOrder.push_back("relative_best_improve_shift");
    mapOperators["relative_best_improve_inversion"] = std::make_pair(new dim::variation::RelativeBestImprovementMutation<EOT>(inversionOp, *ptRelativeBestImprovementInversionEvalCounter), ptRelativeBestImprovementInversionEvalCounter);
    operatorsOrder.push_back("relative_best_improve_inversion");

    dim::variation::IncrementalEvalCounter<EOT>* ptBestImprovementSwapEvalCounter = new dim::variation::IncrementalEvalCounter<EOT>(swapEval);
    dim::variation::IncrementalEvalCounter<EOT>* ptBestImprovementShiftEvalCounter = new dim::variation::IncrementalEvalCounter<EOT>(shiftEval);
    dim::variation::IncrementalEvalCounter<EOT>* ptBestImprovementInversionEvalCounter = new dim::variation::IncrementalEvalCounter<EOT>(inversionEval);

    mapOperators["best_improve_swap"] = std::make_pair(new dim::variation::BestImprovementMutation<EOT>(swapOp, *ptBestImprovementSwapEvalCounter), ptBestImprovementSwapEvalCounter);
    operatorsOrder.push_back("best_improve_swap");
    mapOperators["best_improve_shift"] = std::make_pair(new dim::variation::BestImprovementMutation<EOT>(shiftOp, *ptBestImprovementShiftEvalCounter), ptBestImprovementShiftEvalCounter);
    operatorsOrder.push_back("best_improve_shift");
    mapOperators["best_improve_inversion"] = std::make_pair(new dim::variation::BestImprovementMutation<EOT>(inversionOp, *ptBestImprovementInversionEvalCounter), ptBestImprovementInversionEvalCounter);
    operatorsOrder.push_back("best_improve_inversion");

    // mapOperators["2swap"] = new eoSwapMutation<EOT>(2);	operatorsOrder.push_back("2swap");
    // mapOperators["2opt"] = new eoTwoOptMutation<EOT>;	operatorsOrder.push_back("2opt");

    std::ostringstream ssOrder, ssOrder2;
    ssOrder << "Set an operator between " << operatorsOrder[0];
    ssOrder2 << "List of operators separeted by a comma. Select operators between " << operatorsOrder[0];
    for ( size_t k = 1; k < operatorsOrder.size(); ++k )
	{
	    ssOrder << "," << operatorsOrder[k];
	    ssOrder2 << "," << operatorsOrder[k];
	}

    std::vector<std::string> operatorsVec(nislands, "");

    for (size_t i = 0; i < nislands; ++i)
	{
	    std::ostringstream ss;
	    ss << "operator" << i;
	    operatorsVec[i] = parser.createParam(std::string(operatorsOrder[ i % operatorsOrder.size() ]), ss.str(), ssOrder.str(), 0, "Islands Model").value();
	}

    // O
    std::string operators = parser.createParam(std::string(""), "operators", ssOrder2.str(), 'O', "Islands Model").value();

    if (!operators.empty())
	{
	    boost::split(operatorsVec, operators, boost::is_any_of(","));
	}

    /**************
     * EO routine *
     **************/

    make_parallel(parser);
    make_verbose(parser);
    make_help(parser);

    dim::initialization::TSPLibGraph::load( tspInstance ); // Instance
    dim::initialization::Route<double> init ; // Sol. Random Init.
    dim::core::Pop<EOT>& pop = dim::do_make::detail::pop(parser, state, init);

    // smp

    /**********************************
     * Déclaration des composants DIM *
     **********************************/

    dim::core::ThreadsRunner< EOT > tr;

    std::vector< dim::core::Pop<EOT>* > islandPop(nislands);
    std::vector< dim::core::IslandData<EOT>* > islandData(nislands);

    dim::core::MigrationMatrix probabilities( nislands );
    dim::core::InitMatrix initmatrix( initG, probaSame );

    initmatrix( probabilities );
    std::cout << probabilities;

    std::cout << "size: " << dim::initialization::TSPLibGraph::size() << std::endl;

    for (size_t i = 0; i < nislands; ++i)
	{
	    std::cout << "island " << i << std::endl;

	    islandPop[i] = new dim::core::Pop<EOT>(popSize, init);
	    islandData[i] = new dim::core::IslandData<EOT>(nislands, i, monitorPrefix);

	    std::cout << islandData[i]->size() << " " << islandData[i]->rank() << " " << operatorsVec[ islandData[i]->rank() ] << std::endl;

	    eoEvalFuncCounter<EOT>* ptEval = new eoEvalFuncCounter<EOT>(mainEval);
	    state.storeFunctor(ptEval);

	    islandData[i]->proba = probabilities(i);
	    apply<EOT>(*ptEval, *(islandPop[i]));

	    /****************************************
	     * Distribution des opérateurs aux iles *
	     ****************************************/

	    dim::variation::Base<EOT>* ptMon = mapOperators[ operatorsVec[ islandData[i]->rank() ] ].first;
	    ptMon->monitorPrefix = monitorPrefix;
	    ptMon->rank = i;
	    ptMon->firstCall();

	    dim::variation::IncrementalEvalCounter<EOT>* ptIncrementalEvalCounter = mapOperators[ operatorsVec[ islandData[i]->rank() ] ].second;

	    dim::evolver::Base<EOT>* ptEvolver = new dim::evolver::Easy<EOT>( *ptEval, *ptMon, false );
	    state_dim.storeFunctor(ptEvolver);

	    dim::feedbacker::Base<EOT>* ptFeedbacker = new dim::feedbacker::smp::Easy<EOT>(islandPop, islandData, alphaF);
	    state_dim.storeFunctor(ptFeedbacker);

	    dim::vectorupdater::Reward<EOT>* ptReward = NULL;
	    if (rewardStrategy == "best")
		{
		    ptReward = new dim::vectorupdater::Best<EOT>(alphaP, betaP);
		}
	    else
		{
		    ptReward = new dim::vectorupdater::Average<EOT>(alphaP, betaP);
		}
	    state_dim.storeFunctor(ptReward);

	    dim::vectorupdater::Base<EOT>* ptUpdater = new dim::vectorupdater::Easy<EOT>(*ptReward);
	    state_dim.storeFunctor(ptUpdater);

	    dim::memorizer::Base<EOT>* ptMemorizer = new dim::memorizer::Easy<EOT>();
	    state_dim.storeFunctor(ptMemorizer);

	    dim::migrator::Base<EOT>* ptMigrator = new dim::migrator::smp::Easy<EOT>(islandPop, islandData);
	    state_dim.storeFunctor(ptMigrator);

	    dim::continuator::Base<EOT>& continuator = dim::do_make::continuator<EOT>(parser, state, *ptEval);
	    dim::utils::CheckPoint<EOT>& checkpoint = dim::do_make::checkpoint<EOT>(parser, state, continuator, *ptIncrementalEvalCounter, *(islandData[i]), 1, stepTimer);

	    dim::algo::Base<EOT>* ptIsland = new dim::algo::smp::Easy<EOT>( *ptEvolver, *ptFeedbacker, *ptUpdater, *ptMemorizer, *ptMigrator, checkpoint, islandPop, islandData );
	    state_dim.storeFunctor(ptIsland);

	    ptEvolver->size(nislands);
	    ptFeedbacker->size(nislands);
	    ptReward->size(nislands);
	    ptUpdater->size(nislands);
	    ptMemorizer->size(nislands);
	    ptMigrator->size(nislands);
	    ptIsland->size(nislands);

	    ptEvolver->rank(i);
	    ptFeedbacker->rank(i);
	    ptReward->rank(i);
	    ptUpdater->rank(i);
	    ptMemorizer->rank(i);
	    ptMigrator->rank(i);
	    ptIsland->rank(i);

	    tr.add(*ptIsland);
	}

    dim::core::IslandData<EOT> data(nislands, -1, monitorPrefix);
    tr(pop, data);

    for (size_t i = 0; i < nislands; ++i)
	{
	    delete islandPop[i];
	    delete islandData[i];
	}

    for ( std::map< std::string, std::pair< dim::variation::Base<EOT>*, dim::variation::IncrementalEvalCounter<EOT>* > >::iterator it = mapOperators.begin(); it != mapOperators.end(); ++it )
    	{
    	    delete it->second.first;
    	    delete it->second.second;
    	}

    return 0;
}
