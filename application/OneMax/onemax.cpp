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

#include <fstream>
#include <contrib/boost/mpi/environment.hpp>
#include <eo>
#include <ga.h>
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

typedef dim::core::Bit<double> EOT;

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

    // a
    double alphaP = parser.createParam(double(0.2), "alpha", "Alpha Probability", 'a', "Islands Model").value();
    double alphaF = parser.createParam(double(0.01), "alphaF", "Alpha Fitness", 'A', "Islands Model").value();
    // b
    double betaP = parser.createParam(double(0.01), "beta", "Beta Probability", 'b', "Islands Model").value();
    // d
    double probaSame = parser.createParam(double(100./ALL), "probaSame", "Probability for an individual to stay in the same island", 'd', "Islands Model").value();
    // I
    bool initG = parser.createParam(bool(true), "initG", "initG", 'I', "Islands Model").value();

    bool update = parser.createParam(bool(true), "update", "update", 'U', "Islands Model").value();
    bool feedback = parser.createParam(bool(true), "feedback", "feedback", 'F', "Islands Model").value();
    bool migrate = parser.createParam(bool(true), "migrate", "migrate", 'M', "Islands Model").value();
    unsigned nmigrations = parser.createParam(unsigned(1), "nmigrations", "Number of migrations to do at each generation (0=all individuals are migrated)", 0, "Islands Model").value();
    bool sync = parser.createParam(bool(false), "sync", "sync", 0, "Islands Model").value();
    bool smp = parser.createParam(bool(false), "smp", "smp", 0, "Islands Model").value();
    unsigned nislands = parser.createParam(unsigned(4), "nislands", "Number of islands (see --smp)", 0, "Islands Model").value();
    unsigned stepTimer = parser.createParam(unsigned(100), "stepTimer", "stepTimer", 0, "Islands Model").value();
    bool deltaUpdate = parser.createParam(bool(true), "deltaUpdate", "deltaUpdate", 0, "Islands Model").value();
    bool deltaFeedback = parser.createParam(bool(true), "deltaFeedback", "deltaFeedback", 0, "Islands Model").value();
    double sensitivity = 1 / parser.createParam(double(1.), "sensitivity", "sensitivity of delta{t} (1/sensitivity)", 0, "Islands Model").value();

    /*********************************
     * Déclaration des composants EO *
     *********************************/

    unsigned chromSize = parser.getORcreateParam(unsigned(1000), "chromSize", "The length of the bitstrings", 'n',"Problem").value();
    eoInit<EOT>& init = dim::do_make::genotype(parser, state, EOT(), 0);

    dim::evaluation::OneMax<EOT> mainEval;
    eoEvalFuncCounter<EOT> eval(mainEval);

    unsigned popSize = parser.getORcreateParam(unsigned(100), "popSize", "Population Size", 'P', "Evolution Engine").value();
    dim::core::Pop<EOT>& pop = dim::do_make::detail::pop(parser, state, init);

    double targetFitness = parser.getORcreateParam(double(chromSize), "targetFitness", "Stop when fitness reaches",'T', "Stopping criterion").value();
    unsigned maxGen = parser.getORcreateParam(unsigned(0), "maxGen", "Maximum number of generations () = none)",'G',"Stopping criterion").value();
    dim::continuator::Base<EOT>& continuator = dim::do_make::continuator<EOT>(parser, state, eval);

    dim::core::IslandData<EOT> data(smp ? nislands : -1);

    std::string monitorPrefix = parser.getORcreateParam(std::string("result"), "monitorPrefix", "Monitor prefix filenames", '\0', "Output").value();
    dim::utils::CheckPoint<EOT>& checkpoint = dim::do_make::checkpoint<EOT>(parser, state, continuator, data, 1, stepTimer);

    /**************
     * EO routine *
     **************/

    make_parallel(parser);
    make_verbose(parser);
    make_help(parser);

    /****************************************
     * Distribution des opérateurs aux iles *
     ****************************************/

    eoMonOp<EOT>* ptMon = NULL;
    if ( RANK == 0 )
	{
	    eo::log << eo::logging << RANK << ": bitflip ";
	    ptMon = new eoBitMutation<EOT>( 1, true );
	}
    else
	{
	    eo::log << eo::logging << RANK << ": kflip(" << (RANK-1) * 2 + 1 << ") ";
	    ptMon = new eoDetPermutBitFlip<EOT>( (RANK-1) * 2 + 1 );
	}
    eo::log << eo::logging << std::endl;
    eo::log.flush();
    state.storeFunctor(ptMon);

    if (!smp) // no smp enabled use mpi instead
	{

	    /**********************************
	     * Déclaration des composants DIM *
	     **********************************/

	    dim::core::ThreadsRunner< EOT > tr;

	    dim::evolver::Easy<EOT> evolver( /*eval*/mainEval, *ptMon );

	    dim::feedbacker::Base<EOT>* ptFeedbacker = NULL;
	    if (feedback)
		{
		    if (sync)
			{
			    ptFeedbacker = new dim::feedbacker::sync::Easy<EOT>(alphaF);
			}
		    else
			{
			    ptFeedbacker = new dim::feedbacker::async::Easy<EOT>(alphaF, sensitivity, deltaFeedback);
			}
		}
	    else
		{
		    ptFeedbacker = new dim::algo::Easy<EOT>::DummyFeedbacker();
		}
	    state_dim.storeFunctor(ptFeedbacker);

	    dim::vectorupdater::Base<EOT>* ptUpdater = NULL;
	    if (update)
		{
		    ptUpdater = new dim::vectorupdater::Easy<EOT>(alphaP, betaP, sensitivity, sync ? false : deltaUpdate);
		}
	    else
		{
		    ptUpdater = new dim::algo::Easy<EOT>::DummyVectorUpdater();
		}
	    state_dim.storeFunctor(ptUpdater);

	    dim::memorizer::Easy<EOT> memorizer;

	    dim::migrator::Base<EOT>* ptMigrator = NULL;
	    if (migrate)
		{
		    if (sync)
			{
			    ptMigrator = new dim::migrator::sync::Easy<EOT>();
			}
		    else
			{
			    ptMigrator = new dim::migrator::async::Easy<EOT>(nmigrations);
			}
		}
	    else
		{
		    ptMigrator = new dim::algo::Easy<EOT>::DummyMigrator();
		}
	    state_dim.storeFunctor(ptMigrator);

	    dim::algo::Easy<EOT> island( evolver, *ptFeedbacker, *ptUpdater, memorizer, *ptMigrator, checkpoint, monitorPrefix );

	    if (!sync)
		{
		    tr.addHandler(*ptFeedbacker).addHandler(*ptMigrator).add(island);
		}

	    /***************
	     * Rock & Roll *
	     ***************/

	    /******************************************************************************
	     * Création de la matrice de transition et distribution aux iles des vecteurs *
	     ******************************************************************************/

	    dim::core::MigrationMatrix probabilities( ALL );
	    dim::core::InitMatrix initmatrix( initG, probaSame );

	    if ( 0 == RANK )
		{
		    initmatrix( probabilities );
		    std::cout << probabilities;
		    data.proba = probabilities(RANK);

		    for (size_t i = 1; i < ALL; ++i)
			{
			    world.send( i, 100, probabilities(i) );
			}

		    std::cout << "Island Model Parameters:" << std::endl
			      << "alphaP: " << alphaP << std::endl
			      << "alphaF: " << alphaF << std::endl
			      << "betaP: " << betaP << std::endl
			      << "probaSame: " << probaSame << std::endl
			      << "initG: " << initG << std::endl
			      << "update: " << update << std::endl
			      << "feedback: " << feedback << std::endl
			      << "migrate: " << migrate << std::endl
			      << "sync: " << sync << std::endl
			      << "stepTimer: " << stepTimer << std::endl
			      << "deltaUpdate: " << deltaUpdate << std::endl
			      << "deltaFeedback: " << deltaFeedback << std::endl
			      << "sensitivity: " << sensitivity << std::endl
			      << "chromSize: " << chromSize << std::endl
			      << "popSize: " << popSize << std::endl
			      << "targetFitness: " << targetFitness << std::endl
			      << "maxGen: " << maxGen << std::endl
			;
		}
	    else
		{
		    world.recv( 0, 100, data.proba );
		}

	    /******************************************
	     * Get the population size of all islands *
	     ******************************************/

	    world.barrier();
	    dim::utils::print_sum(pop);

	    apply<EOT>(eval, pop);

	    if (sync)
		{
		    island( pop, data );
		}
	    else
		{
		    tr( pop, data );
		}

	    world.abort(0);

	    return 0 ;

	}

    // smp

    dim::core::ThreadsRunner< EOT > tr;

    std::vector< dim::core::Pop<EOT> > islandPop(nislands);
    std::vector< dim::core::IslandData<EOT> > islandData(nislands/*, data*/);
    // std::vector< dim::core::IslandData<EOT> > islandData;

    dim::core::MigrationMatrix probabilities( nislands );
    dim::core::InitMatrix initmatrix( initG, 100./nislands );

    initmatrix( probabilities );
    std::cout << probabilities;

    for (size_t i = 0; i < nislands; ++i)
	{
	    std::cout << "island " << i << std::endl;

	    islandPop[i].append(popSize, init);

	    islandData[i] = dim::core::IslandData<EOT>(nislands, i);

	    // islandData[i].size(nislands);
	    // islandData[i].rank(i);

	    std::cout << islandData[i].size() << " " << islandData[i].rank() << std::endl;


	    islandData[i].proba = probabilities(i);
	    apply<EOT>(eval, islandPop[i]);

	    dim::evolver::Base<EOT>* ptEvolver = new dim::evolver::Easy<EOT>( /*eval*/mainEval, *ptMon );
	    state_dim.storeFunctor(ptEvolver);

	    dim::feedbacker::Base<EOT>* ptFeedbacker = new dim::feedbacker::smp::Easy<EOT>(islandPop, islandData, alphaF);
	    state_dim.storeFunctor(ptFeedbacker);

	    dim::vectorupdater::Base<EOT>* ptUpdater = new dim::vectorupdater::Easy<EOT>(alphaP, betaP, sensitivity, sync ? false : deltaUpdate);
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
	    ptUpdater->size(nislands);
	    ptMemorizer->size(nislands);
	    ptMigrator->size(nislands);
	    ptIsland->size(nislands);

	    ptEvolver->rank(i);
	    ptFeedbacker->rank(i);
	    ptUpdater->rank(i);
	    ptMemorizer->rank(i);
	    ptMigrator->rank(i);
	    ptIsland->rank(i);

	    tr.add(*ptIsland);
	}

    tr(pop, data);

    return 0 ;
}
