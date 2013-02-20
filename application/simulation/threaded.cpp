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
#include <dim/dim>
#include <dim/algo/Easy.h>
#include <dim/evolver/Easy.h>
#include <dim/feedbacker/Easy.h>
#include <dim/migrator/Easy.h>
#include <dim/vectorupdater/Easy.h>

using namespace std;
using namespace boost::mpi;
using namespace boost;

typedef dim::core::Bit<double> EOT;

class SimulatedOp : public eoMonOp<EOT>
{
public:
    SimulatedOp(size_t timeout) : _timeout(timeout) {}

    /// The class name.
    virtual std::string className() const { return "SimulatedOp"; }

    bool operator()(EOT&)
    {
	std::this_thread::sleep_for(std::chrono::milliseconds( _timeout ));
	return true;
    }

private:
    size_t _timeout;
};

class SimulatedEval : public eoEvalFunc<EOT>
{
public:
    SimulatedEval(typename EOT::Fitness fit) : _fit(fit) {}

    /// The class name.
    virtual string className() const { return "SimulatedEval"; }

    void operator()(EOT& sol)
    {
	sol.fitness( sol.fitness() + _fit );
    }

private:
    typename EOT::Fitness _fit;
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

    environment env(argc, argv);
    communicator world;

    /****************************
     * Il faut au moins 4 nœuds *
     ****************************/

    const size_t ALL = world.size();
    const size_t RANK = world.rank();

    // if ( ALL < 4 )
    // 	{
    // 	    if ( 0 == RANK )
    // 		{
    // 		    cerr << "Needs at least 4 processes to be launched!" << endl;
    // 		}
    // 	    return 0;
    // 	}

    /************************
     * Initialisation de EO *
     ************************/

    eoParser parser(argc, argv);
    eoState state;    // keeps all things allocated

    /*****************************
     * Definition des paramètres *
     *****************************/

    // a
    double alpha = parser.createParam(double(0.8), "alpha", "Alpha", 'a', "Islands Model").value();
    // b
    double beta = parser.createParam(double(0.99), "beta", "Beta", 'b', "Islands Model").value();
    // p
    /*size_t probaMin = */parser.createParam(size_t(10), "probaMin", "Minimum probability to stay in the same island", 'p', "Islands Model").value();
    // d
    size_t probaSame = parser.createParam(size_t(100/ALL), "probaSame", "Probability for an individual to stay in the same island", 'd', "Islands Model").value();
    // r
    /*size_t reward = */parser.createParam(size_t(2), "reward", "reward", 'r', "Islands Model")/*.value()*/;
    /*size_t penalty = */parser.createParam(size_t(1), "penalty", "penalty", 0, "Islands Model")/*.value()*/;
    // I
    bool initG = parser.createParam(bool(true), "initG", "initG", 'I', "Islands Model").value();

    /*********************************
     * Déclaration des composants EO *
     *********************************/

    /*unsigned chromSize = */parser.getORcreateParam(unsigned(0), "chromSize", "The length of the bitstrings", 'n',"Problem")/*.value()*/;
    eoInit<EOT>& init = dim::do_make::genotype(parser, state, EOT(), 0);

    size_t timeout = pow(10, RANK);
    // size_t timeout = 10000;

    eoEvalFunc<EOT>* ptEval = NULL;
    ptEval = new SimulatedEval(1);
    state.storeFunctor(ptEval);

    // eoEvalFunc<EOT>* ptEval = NULL;
    // if ( 0 == RANK )
    // 	{
    // 	    ptEval = new SimulatedEval(1);
    // 	}
    // else if ( 1 == RANK )
    // 	{
    // 	    ptEval = new SimulatedEval(5);
    // 	}
    // else if ( 2 == RANK )
    // 	{
    // 	    ptEval = new SimulatedEval(10);
    // 	}
    // else if ( 3 == RANK )
    // 	{
    // 	    ptEval = new SimulatedEval(15);
    // 	}
    // state.storeFunctor(ptEval);

    eoEvalFuncCounter<EOT> eval(*ptEval);

    /*unsigned popSize = */parser.getORcreateParam(unsigned(100), "popSize", "Population Size", 'P', "Evolution Engine")/*.value()*/;
    dim::core::Pop<EOT>& pop = dim::do_make::detail::pop(parser, state, init);

    /*double targetFitness = */parser.getORcreateParam(double(1000), "targetFitness", "Stop when fitness reaches",'T', "Stopping criterion")/*.value()*/;
    /*unsigned maxGen = */parser.getORcreateParam(unsigned(0), "maxGen", "Maximum number of generations () = none)",'G',"Stopping criterion")/*.value()*/;
    dim::continuator::Base<EOT>& continuator = dim::do_make::continuator<EOT>(parser, state, eval);

    dim::core::IslandData<EOT> data;

    dim::utils::CheckPoint<EOT>& checkpoint = dim::do_make::checkpoint(parser, state, continuator, data);

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
    ptMon = new SimulatedOp(timeout);
    state.storeFunctor(ptMon);

    // if ( 0 == RANK )
    // 	{
    // 	    ptMon = new SimulatedOp(1);
    // 	}
    // else if ( 1 == RANK )
    // 	{
    // 	    ptMon = new SimulatedOp(5);
    // 	}
    // else if ( 2 == RANK )
    // 	{
    // 	    ptMon = new SimulatedOp(10);
    // 	}
    // else if ( 3 == RANK )
    // 	{
    // 	    ptMon = new SimulatedOp(15);
    // 	}
    // state.storeFunctor(ptMon);

    // /**********************************
    //  * Déclaration des composants DIM *
    //  **********************************/

    dim::evolver::async::Easy<EOT> evolver( /*eval*/*ptEval, *ptMon );
    dim::feedbacker::async::Easy<EOT> feedbacker;
    dim::vectorupdater::async::Easy<EOT> updater(alpha, beta);
    dim::memorizer::async::Easy<EOT> memorizer;
    dim::migrator::async::Easy<EOT> migrator;

    dim::algo::async::Easy<EOT> island( evolver, feedbacker, updater, memorizer, migrator, checkpoint );

    /***************
     * Rock & Roll *
     ***************/

    /******************************************************************************
     * Création de la matrice de transition et distribution aux iles des vecteurs *
     ******************************************************************************/

    dim::core::MigrationMatrix<EOT> probabilities( ALL );
    dim::core::InitMatrix<EOT> initmatrix( initG, probaSame );

    if ( 0 == RANK )
    	{
    	    initmatrix( probabilities );
    	    std::cout << probabilities;
    	    data.proba = probabilities(RANK);

    	    for (size_t i = 1; i < ALL; ++i)
    		{
    		    world.send( i, 100, probabilities(i) );
    		}
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

    island( pop, data );

    world.abort(0);

    return 0 ;
}