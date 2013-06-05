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
namespace std_or_boost = boost;
#endif

typedef dim::core::Bit<double> EOT;

struct DummyOp : public eoMonOp<EOT>
{
    bool operator()(EOT&) {}
};

class SimulatedOp : public eoMonOp<EOT>
{
public:
    SimulatedOp(double timeout) : _timeout(timeout)
    {
#ifdef TRACE
	boost::mpi::communicator world;
	std::ostringstream ss;
	ss << "sleep." << world.rank();
	_of.open(ss.str().c_str());
#endif
    }

    /// The class name.
    virtual std::string className() const { return "SimulatedOp"; }

    bool operator()(EOT&)
    {
#ifdef TRACE
	AUTO(std_or_boost::chrono::time_point< std_or_boost::chrono::system_clock >) start = std_or_boost::chrono::system_clock::now();
#endif

	/*std_or_*/boost::this_thread::sleep_for(/*std_or_*/boost::chrono::microseconds( int(_timeout * 1000) ));

#ifdef TRACE
	long elapsed = std_or_boost::chrono::duration_cast<std_or_boost::chrono::microseconds>(std_or_boost::chrono::system_clock::now()-start).count(); // micro
	_of << elapsed / 1000. << " "; _of.flush();
#endif

	return true;
    }

private:
    double _timeout;

#ifdef TRACE
    std::ofstream _of;
#endif
};

class SimulatedEval : public eoEvalFunc<EOT>
{
public:
#if __cplusplus > 199711L
    SimulatedEval(typename EOT::Fitness fit) : _fit(fit) {}
#else
    SimulatedEval(EOT::Fitness fit) : _fit(fit) {}
#endif

    /// The class name.
    virtual std::string className() const { return "SimulatedEval"; }

    void operator()(EOT& sol)
    {
	sol.fitness( sol.fitness() + _fit );
    }

private:
#if __cplusplus > 199711L
    typename
#endif
    EOT::Fitness _fit;

};

class FitnessInit : public eoUF<EOT&, void>
{
public:
    void operator()(EOT& sol)
    {
	sol.fitness( 0 );
    }
};

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

    bool sync = parser.createParam(bool(true), "sync", "sync", 0, "Islands Model").value();
    bool smp = parser.createParam(bool(true), "smp", "smp", 0, "Islands Model").value();
    unsigned nislands = parser.createParam(unsigned(4), "nislands", "Number of islands (see --smp)", 0, "Islands Model").value();
    // a
    double alphaP = parser.createParam(double(0.2), "alpha", "Alpha Probability", 'a', "Islands Model").value();
    double alphaF = parser.createParam(double(0.01), "alphaF", "Alpha Fitness", 'A', "Islands Model").value();
    // b
    double betaP = parser.createParam(double(0.01), "beta", "Beta Probability", 'b', "Islands Model").value();
    // d
    double probaSame = parser.createParam(double(100./(smp ? nislands : ALL)), "probaSame", "Probability for an individual to stay in the same island", 'd', "Islands Model").value();
    // I
    bool initG = parser.createParam(bool(true), "initG", "initG", 'I', "Islands Model").value();

    bool update = parser.createParam(bool(true), "update", "update", 'U', "Islands Model").value();
    bool feedback = parser.createParam(bool(true), "feedback", "feedback", 'F', "Islands Model").value();
    bool migrate = parser.createParam(bool(true), "migrate", "migrate", 'M', "Islands Model").value();
    unsigned nmigrations = parser.createParam(unsigned(1), "nmigrations", "Number of migrations to do at each generation (0=all individuals are migrated)", 0, "Islands Model").value();
    unsigned stepTimer = parse0r.createParam(unsigned(1000), "stepTimer", "stepTimer", 0, "Islands Model").value();
    bool deltaUpdate = parser.createParam(bool(true), "deltaUpdate", "deltaUpdate", 0, "Islands Model").value();
    bool deltaFeedback = parser.createParam(bool(true), "deltaFeedback", "deltaFeedback", 0, "Islands Model").value();
    double sensitivity = 1 / parser.createParam(double(1.), "sensitivity", "sensitivity of delta{t} (1/sensitivity)", 0, "Islands Model").value();
    std::string rewardStrategy = parser.createParam(std::string("avg"), "rewardStrategy", "Strategy of rewarding: best or avg", 0, "Islands Model").value();

    std::vector<double> rewards(smp ? nislands : ALL, 1.);
    std::vector<double> timeouts(smp ? nislands : ALL, 1.);

    for (size_t i = 0; i < (smp ? nislands : ALL); ++i)
	{
	    std::ostringstream ss;
	    ss << "reward" << i;
	    rewards[i] = parser.createParam(double(1.), ss.str(), ss.str(), 0, "Islands Model").value();
	    ss.str("");
	    ss << "timeout" << i;
	    timeouts[i] = parser.createParam(double(1.), ss.str(), ss.str(), 0, "Islands Model").value();
	}

    /*********************************
     * Déclaration des composants EO *
     *********************************/

    unsigned chromSize = parser.getORcreateParam(unsigned(0), "chromSize", "The length of the bitstrings", 'n',"Problem").value();
    eoInit<EOT>& init = dim::do_make::genotype(parser, state, EOT(), 0);

    eoEvalFunc<EOT>* ptEval = NULL;
    ptEval = new SimulatedEval( rewards[RANK] );
    state.storeFunctor(ptEval);

    eoEvalFuncCounter<EOT> eval(*ptEval);

    unsigned popSize = parser.getORcreateParam(unsigned(100), "popSize", "Population Size", 'P', "Evolution Engine").value();
    dim::core::Pop<EOT>& pop = dim::do_make::detail::pop(parser, state, init);

    double targetFitness = parser.getORcreateParam(double(1000), "targetFitness", "Stop when fitness reaches",'T', "Stopping criterion").value();
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

    if (!smp) // no smp enabled use mpi instead
	{

	    /****************************************
	     * Distribution des opérateurs aux iles *
	     ****************************************/

	    eoMonOp<EOT>* ptMon = NULL;
	    if (sync)
		{
		    ptMon = new DummyOp;
		}
	    else
		{
		    ptMon = new SimulatedOp( timeouts[RANK] );
		}
	    state.storeFunctor(ptMon);

	    /**********************************
	     * Déclaration des composants DIM *
	     **********************************/

	    dim::core::ThreadsRunner< EOT > tr;

	    dim::evolver::Easy<EOT> evolver( /*eval*/*ptEval, *ptMon, false );

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
		    dim::vectorupdater::Reward<EOT>* ptReward = NULL;
		    if (rewardStrategy == "best")
			{
			    ptReward = new dim::vectorupdater::Best<EOT>(alphaP, betaP);
			}
		    else
			{
			    ptReward = new dim::vectorupdater::Proportional<EOT>(alphaP, betaP, sensitivity, sync ? false : deltaUpdate);
			}
		    state_dim.storeFunctor(ptReward);

		    ptUpdater = new dim::vectorupdater::Easy<EOT>(*ptReward);
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

	    FitnessInit fitInit;

	    apply<EOT>(fitInit, pop);

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

    FitnessInit fitInit;

    for (size_t i = 0; i < nislands; ++i)
	{
	    std::cout << "island " << i << std::endl;

	    islandPop[i].append(popSize, init);

	    apply<EOT>(fitInit, islandPop[i]);

	    islandData[i] = dim::core::IslandData<EOT>(nislands, i);

	    std::cout << islandData[i].size() << " " << islandData[i].rank() << std::endl;

	    islandData[i].proba = probabilities(i);
	    apply<EOT>(eval, islandPop[i]);

	    /****************************************
	     * Distribution des opérateurs aux iles *
	     ****************************************/

	    eoMonOp<EOT>* ptMon = NULL;
	    ptMon = new SimulatedOp( timeouts[islandData[i].rank()] );
	    state.storeFunctor(ptMon);

	    eoEvalFunc<EOT>* __ptEval = NULL;
	    __ptEval = new SimulatedEval( rewards[islandData[i].rank()] );
	    state.storeFunctor(__ptEval);

	    dim::evolver::Base<EOT>* ptEvolver = new dim::evolver::Easy<EOT>( /*eval*/*__ptEval, *ptMon, false );
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
		    ptReward = new dim::vectorupdater::Proportional<EOT>(alphaP, betaP, sensitivity, sync ? false : deltaUpdate);
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
