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
#include <ga.h>
#include <dim/dim>
#include <dim/feedbacker/Easy.h>
#include <dim/migrator/Easy.h>
#include <dim/vectorupdater/Easy.h>
#include <dim/algo/Easy.h>

using namespace std;
using namespace boost::mpi;
using namespace boost;

typedef dim::core::Bit<double> EOT;

int main(int argc, char *argv[])
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

    unsigned chromSize = parser.getORcreateParam(unsigned(1000), "chromSize", "The length of the bitstrings", 'n',"Problem").value();
    eoInit<EOT>& init = dim::do_make::genotype(parser, state, EOT(), 0);

    dim::evaluation::OneMax<EOT> mainEval;
    eoEvalFuncCounter<EOT> eval(mainEval);

    unsigned popSize = parser.getORcreateParam(unsigned(100), "popSize", "Population Size", 'P', "Evolution Engine").value();
    dim::core::Pop<EOT>& pop = dim::do_make::pop(parser, state, init);

    /*unsigned maxGen = */parser.getORcreateParam(unsigned(0), "maxGen", "Maximum number of generations () = none)",'G',"Stopping criterion")/*.value()*/;
    /*double targetFitness = */parser.getORcreateParam(double(chromSize), "targetFitness", "Stop when fitness reaches",'T', "Stopping criterion")/*.value()*/;
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
    eo::log << eo::logging << endl;
    eo::log.flush();
    state.storeFunctor(ptMon);

    /**********************************
     * Déclaration des composants DIM *
     **********************************/

    // dim::evolver::sync::Easy<EOT> evolver( eval, *ptMon );
    // dim::feedbacker::sync::Easy<EOT> feedbacker;
    // dim::inputprobasender::sync::Easy<EOT> probasender;
    // dim::vectorupdater::sync::Easy<EOT> updater(alpha, beta);
    // dim::memorizer::sync::Easy<EOT> memorizer;
    // dim::migrator::sync::Easy<EOT> migrator;
    // dim::algo::sync::Easy<EOT> island( evolver, feedbacker, probasender, updater, memorizer, migrator, checkpoint );

    dim::evolver::async::Easy<EOT> evolver( eval, *ptMon );
    dim::feedbacker::async::Easy<EOT> feedbacker;
    feedbacker.popSize = popSize;
    // dim::inputprobasender::async::Easy<EOT> probasender;
    dim::vectorupdater::async::Easy<EOT> updater(alpha, beta);
    dim::memorizer::async::Easy<EOT> memorizer;
    dim::migrator::async::Easy<EOT> migrator;

    // dim::algo::async::Easy<EOT>::DummyEvolver evolver;
    // dim::algo::async::Easy<EOT>::DummyFeedbacker feedbacker;
    dim::algo::async::Easy<EOT>::DummyInputProbaSender probasender;
    // dim::algo::async::Easy<EOT>::DummyVectorUpdater updater;
    // dim::algo::async::Easy<EOT>::DummyMemorizer memorizer;

    dim::algo::async::Easy<EOT> island( evolver, feedbacker, probasender, updater, memorizer, migrator, checkpoint );

    /***************
     * Rock & Roll *
     ***************/

    /******************************************************************************
     * Création de la matrice de transition et distribution aux iles des vecteurs *
     ******************************************************************************/

    dim::core::MigrationMatrix<EOT> probabilities( ALL );
    dim::core::InitMatrix<EOT> initmatrix( initG, probaSame );

    // ublas::matrix< double > m(ALL, ALL);

    if ( 0 == RANK )
	{
	    initmatrix( probabilities );
	    std::cout << probabilities;
	    data.proba = probabilities(RANK);
	    for (size_t j = 0; j < ALL; ++j)
		{
		    data.probaret[j] = probabilities(j, RANK);
		}

	    for (size_t i = 1; i < ALL; ++i)
		{
		    world.send( i, 100, probabilities(i) );
		    std::vector< typename EOT::Fitness > probaRetIsl( ALL );
		    for (size_t j = 0; j < ALL; ++j)
			{
			    probaRetIsl[j] = probabilities(j,i);
			}
		    world.send( i, 101, probaRetIsl );
		}
	}
    else
	{
	    world.recv( 0, 100, data.proba );
	    world.recv( 0, 101, data.probaret );
	}

    /******************************************
     * Get the population size of all islands *
     ******************************************/

    world.barrier();
    dim::utils::print_sum(pop);

    apply<EOT>(eval, pop);

    island( pop, data );

    world.abort(0);

    return 0;
}
