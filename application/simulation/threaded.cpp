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
#include <dim/dim>
#include <dim/algo/Easy.h>
#include <dim/evolver/Easy.h>
#include <dim/feedbacker/Easy.h>
#include <dim/migrator/Easy.h>
#include <dim/vectorupdater/Easy.h>
#include <dim/core/State.h>

#if __cplusplus > 199711L
namespace std_or_boost = std;
#else
#include <boost/thread/thread.hpp>
#include <boost/chrono/chrono_io.hpp>
namespace std_or_boost = boost;
#endif

typedef dim::core::Bit<double> EOT;

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

    // a
    // double alphaP = parser.createParam(double(0.8), "alphaP", "Alpha Probability", 'a', "Islands Model").value();
    double alphaP = parser.createParam(double(0.2), "alpha", "Alpha Probability", 'a', "Islands Model").value();
    double alphaF = parser.createParam(double(0.01), "alphaF", "Alpha Fitness", 'A', "Islands Model").value();
    // b
    // double betaP = parser.createParam(double(0.99), "beta", "Beta Probability", 'b', "Islands Model").value();
    double betaP = parser.createParam(double(0.01), "beta", "Beta Probability", 'b', "Islands Model").value();
    // p
    /*size_t probaMin = */parser.createParam(size_t(10), "probaMin", "Minimum probability to stay in the same island", 'p', "Islands Model").value();
    // d
    double probaSame = parser.createParam(double(100./ALL), "probaSame", "Probability for an individual to stay in the same island", 'd', "Islands Model").value();
    // r
    /*size_t reward = */parser.createParam(size_t(2), "reward", "reward", 'r', "Islands Model")/*.value()*/;
    /*size_t penalty = */parser.createParam(size_t(1), "penalty", "penalty", 0, "Islands Model")/*.value()*/;
    // I
    bool initG = parser.createParam(bool(true), "initG", "initG", 'I', "Islands Model").value();

    bool update = parser.createParam(bool(true), "update", "update", 'U', "Islands Model").value();
    bool feedback = parser.createParam(bool(true), "feedback", "feedback", 'F', "Islands Model").value();
    bool migrate = parser.createParam(bool(true), "migrate", "migrate", 'M', "Islands Model").value();
    unsigned nmigrations = parser.createParam(unsigned(1), "nmigrations", "Number of migrations to do at each generation (0=all individuals are migrated)", 0, "Islands Model").value();
    bool barrier = parser.createParam(bool(false), "barrier", "barrier", 0, "Islands Model").value();
    unsigned stepTimer = parser.createParam(unsigned(100), "stepTimer", "stepTimer", 0, "Islands Model").value();
    bool deltaUpdate = parser.createParam(bool(true), "deltaUpdate", "deltaUpdate", 0, "Islands Model").value();
    bool deltaFeedback = parser.createParam(bool(true), "deltaFeedback", "deltaFeedback", 0, "Islands Model").value();
    double sensitivity = 1 / parser.createParam(double(1.), "sensitivity", "sensitivity of delta{t} (1/sensitivity)", 0, "Islands Model").value();

    std::vector<double> rewards(ALL, 1.);
    std::vector<double> timeouts(ALL, 1.);

    for (size_t i = 0; i < ALL; ++i)
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

    dim::core::IslandData<EOT> data;

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
    ptMon = new SimulatedOp( timeouts[RANK] );
    state.storeFunctor(ptMon);

    /**********************************
     * Déclaration des composants DIM *
     **********************************/

    dim::core::ThreadsRunner< EOT > tr;

    dim::evolver::Easy<EOT> evolver( /*eval*/*ptEval, *ptMon, false );

    dim::feedbacker::Base<EOT>* ptFeedbacker = NULL;
    if (feedback)
	{
	    ptFeedbacker = new dim::feedbacker::Easy<EOT>(alphaF, sensitivity, deltaFeedback, barrier);
	}
    else
	{
	    ptFeedbacker = new dim::algo::Easy<EOT>::DummyFeedbacker();
	}
    state_dim.storeFunctor(ptFeedbacker);

    dim::vectorupdater::Base<EOT>* ptUpdater = NULL;
    if (update)
	{
	    ptUpdater = new dim::vectorupdater::Easy<EOT>(alphaP, betaP, sensitivity, deltaUpdate);
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
	    ptMigrator = new dim::migrator::Easy<EOT>(nmigrations, barrier);
	}
    else
	{
	    ptMigrator = new dim::algo::Easy<EOT>::DummyMigrator();
	}
    state_dim.storeFunctor(ptMigrator);

    dim::algo::Easy<EOT> island( evolver, *ptFeedbacker, *ptUpdater, memorizer, *ptMigrator, checkpoint, barrier, monitorPrefix );

    tr.addHandler(*ptFeedbacker).addHandler(*ptMigrator).add(island);

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
		      << "barrier: " << barrier << std::endl
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

    tr( pop, data );

    world.abort(0);

    return 0 ;
}
