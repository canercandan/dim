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

#include <boost/mpi.hpp>
#include <eo>
#include <eoSwapMutation.h>
#include <eoShiftMutation.h>
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

using EOT = dim::representation::Route<double>;

int main (int argc, char *argv[])
{
    /*************************
     * Initialisation de MPI *
     *************************/

    boost::mpi::environment env(argc, argv, MPI_THREAD_MULTIPLE, true);
    boost::mpi::communicator world;

    /****************************
     * Il faut au moins 4 nœuds *
     ****************************/

    const size_t ALL = world.size();
    const size_t RANK = world.rank();

    /************************
     * Initialisation de EO *
     ************************/

    eoParser parser(argc, argv);
    eoState state;    // keeps all things allocated
    dim::core::State state_dim;    // keeps all things allocated

    /*****************************
     * Definition des paramètres *
     *****************************/

    unsigned nislands = parser.createParam(unsigned(4), "nislands", "Number of islands (see --smp)", 0, "Islands Model").value();
    // a
    double alphaP = parser.createParam(double(0.2), "alpha", "Alpha Probability", 'a', "Islands Model").value();
    double alphaF = parser.createParam(double(0.01), "alphaF", "Alpha Fitness", 'A', "Islands Model").value();
    // b
    double betaP = parser.createParam(double(0.01), "beta", "Beta Probability", 'b', "Islands Model").value();
    // d
    double probaSame = parser.createParam(double(100./nislands), "probaSame", "Probability for an individual to stay in the same island", 'd', "Islands Model").value();
    // I
    bool initG = parser.createParam(bool(true), "initG", "initG", 'I', "Islands Model").value();
    unsigned nmigrations = parser.createParam(unsigned(1), "nmigrations", "Number of migrations to do at each generation (0=all individuals are migrated)", 0, "Islands Model").value();
    unsigned stepTimer = parser.createParam(unsigned(1000), "stepTimer", "stepTimer", 0, "Islands Model").value();
    bool deltaUpdate = parser.createParam(bool(true), "deltaUpdate", "deltaUpdate", 0, "Islands Model").value();
    bool deltaFeedback = parser.createParam(bool(true), "deltaFeedback", "deltaFeedback", 0, "Islands Model").value();
    double sensitivity = 1 / parser.createParam(double(1.), "sensitivity", "sensitivity of delta{t} (1/sensitivity)", 0, "Islands Model").value();
    std::string rewardStrategy = parser.createParam(std::string("avg"), "rewardStrategy", "Strategy of rewarding: best or avg", 0, "Islands Model").value();

    std::vector<std::string> operators(nislands, "");
    for (size_t i = 0; i < nislands; ++i)
	{
	    std::ostringstream ss;
	    ss << "operator" << i;
	    operators[i] = parser.createParam(std::string("1swap"), ss.str(), "Set an operator between 1swap, 2swap, shift and 2opt", 0, "Islands Model").value();
	}

    /*********************************
     * Déclaration des composants EO *
     *********************************/

    std::string tspInstance =  parser.getORcreateParam(std::string("benchs/ali535.tsp"), "tspInstance", "filename of the instance for TSP problem", 0, "Problem").value();

    dim::initialization::Graph::load( tspInstance.c_str() ); // Instance
    dim::initialization::Route<double> init ; // Sol. Random Init.

    dim::evaluation::Route<double> mainEval;
    eoEvalFuncCounter<EOT> eval(mainEval);

    unsigned popSize = parser.getORcreateParam(unsigned(100), "popSize", "Population Size", 'P', "Evolution Engine").value();
    dim::core::Pop<EOT>& pop = dim::do_make::detail::pop(parser, state, init);

    double targetFitness = parser.getORcreateParam(double(10), "targetFitness", "Stop when fitness reaches",'T', "Stopping criterion").value();
    unsigned maxGen = parser.getORcreateParam(unsigned(0), "maxGen", "Maximum number of generations () = none)",'G',"Stopping criterion").value();
    dim::continuator::Base<EOT>& continuator = dim::do_make::continuator<EOT>(parser, state, eval);

    dim::core::IslandData<EOT> data(nislands);

    std::string monitorPrefix = parser.getORcreateParam(std::string("result"), "monitorPrefix", "Monitor prefix filenames", '\0', "Output").value();
    dim::utils::CheckPoint<EOT>& checkpoint = dim::do_make::checkpoint(parser, state, continuator, data, 1, stepTimer);

    /**************
     * EO routine *
     **************/

    make_parallel(parser);
    make_verbose(parser);
    make_help(parser);

    // smp

    /**********************************
     * Déclaration des composants DIM *
     **********************************/

    dim::core::ThreadsRunner< EOT > tr;

    std::vector< dim::core::Pop<EOT> > islandPop(nislands);
    std::vector< dim::core::IslandData<EOT> > islandData(nislands);

    dim::core::MigrationMatrix probabilities( nislands );
    dim::core::InitMatrix initmatrix( initG, 100./nislands );

    initmatrix( probabilities );
    std::cout << probabilities;

    std::cout << "size: " << dim::initialization::Graph::size() << std::endl;

    for (size_t i = 0; i < nislands; ++i)
	{
	    std::cout << "island " << i << std::endl;

	    islandPop[i].append(popSize, init);

	    islandData[i] = dim::core::IslandData<EOT>(nislands, i);

	    std::cout << islandData[i].size() << " " << islandData[i].rank() << " " << operators[ islandData[i].rank() ] << std::endl;

	    islandData[i].proba = probabilities(i);
	    apply<EOT>(eval, islandPop[i]);

	    /****************************************
	     * Distribution des opérateurs aux iles *
	     ****************************************/

	    eoMonOp<EOT>* ptMon = NULL;
	    if ( operators[ islandData[i].rank() ] == "1swap" )
	    	{
	    	    // ptMon = new dim::variation::CitySwap<double>;
	    	    ptMon = new eoSwapMutation<EOT>(1);
	    	}
	    else if ( operators[ islandData[i].rank() ] == "2swap" )
	    	{
	    	    ptMon = new eoSwapMutation<EOT>(2);
	    	}
	    else if ( operators[ islandData[i].rank() ] == "shift" )
	    	{
	    	    ptMon = new eoShiftMutation<EOT>;
	    	}
	    else if ( operators[ islandData[i].rank() ] == "2opt" )
	    	{
	    	    ptMon = new eoTwoOptMutation<EOT>;
	    	}
	    state.storeFunctor(ptMon);

	    dim::evolver::Base<EOT>* ptEvolver = new dim::evolver::Easy<EOT>( /*eval*/mainEval, *ptMon );
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
		    ptReward = new dim::vectorupdater::Average<EOT>(alphaP, betaP, false);
		}
	    state_dim.storeFunctor(ptReward);

	    dim::vectorupdater::Base<EOT>* ptUpdater = new dim::vectorupdater::Easy<EOT>(*ptReward);
	    state_dim.storeFunctor(ptUpdater);

	    dim::memorizer::Base<EOT>* ptMemorizer = new dim::memorizer::Easy<EOT>();
	    state_dim.storeFunctor(ptMemorizer);

	    dim::migrator::Base<EOT>* ptMigrator = new dim::migrator::smp::Easy<EOT>(islandPop, islandData, monitorPrefix);
	    state_dim.storeFunctor(ptMigrator);

	    dim::utils::CheckPoint<EOT>& checkpoint = dim::do_make::checkpoint<EOT>(parser, state, continuator, islandData[i], 1, stepTimer);

	    dim::algo::Base<EOT>* ptIsland = new dim::algo::smp::Easy<EOT>( *ptEvolver, *ptFeedbacker, *ptUpdater, *ptMemorizer, *ptMigrator, checkpoint, islandPop, islandData, monitorPrefix );
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

    tr(pop, data);

    return 0 ;
}
