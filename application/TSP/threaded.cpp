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

using namespace std;
using namespace boost::mpi;
using namespace boost;

using EOT = dim::representation::Route<double>;

int main (int argc, char *argv[])
{
    /******************************
     * Initialisation de MPI + EO *
     ******************************/

    environment env(argc, argv);
    communicator world;

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
    size_t probaSame = parser.createParam(size_t(100), "probaSame", "Probability for an individual to stay in the same island", 'd', "Islands Model").value();
    // r
    /*size_t reward = */parser.createParam(size_t(2), "reward", "reward", 'r', "Islands Model")/*.value()*/;
    /*size_t penalty = */parser.createParam(size_t(1), "penalty", "penalty", 0, "Islands Model")/*.value()*/;
    // I
    bool initG = parser.createParam(bool(true), "initG", "initG", 'I', "Islands Model").value();

    /****************************
     * Il faut au moins 4 nœuds *
     ****************************/

    const size_t ALL = world.size();
    const size_t RANK = world.rank();

    if ( ALL < 4 )
    	{
    	    if ( 0 == RANK )
    		{
    		    cerr << "Needs at least 4 processes to be launched!" << endl;
    		}
    	    return 0;
    	}

    /*********************************
     * Déclaration des composants EO *
     *********************************/

    dim::initialization::Graph::load( /*tspInstance.c_str()*/ "benchs/ali535.tsp" ); // Instance
    dim::initialization::Route<double> init ; // Sol. Random Init.

    string tspInstance =  parser.getORcreateParam(string(), "tspInstance", "filename of the instance for TSP problem", 0, "Problem").value();

    dim::evaluation::Route<double> mainEval;
    eoEvalFuncCounter<EOT> eval(mainEval);

    /*unsigned popSize = */parser.getORcreateParam(unsigned(100), "popSize", "Population Size", 'P', "Evolution Engine")/*.value()*/;
    dim::core::Pop<EOT>& pop = dim::do_make::detail::pop(parser, state, init);

    /*double targetFitness = */parser.getORcreateParam(double(10), "targetFitness", "Stop when fitness reaches",'T', "Stopping criterion")/*.value()*/;
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
    if ( 0 == RANK )
	{
	    // ptMon = new dim::variation::CitySwap<double>;
	    ptMon = new eoSwapMutation<EOT>(1);
	}
    else if ( 1 == RANK )
	{
	    ptMon = new eoSwapMutation<EOT>(2);
	}
    else if ( 2 == RANK )
	{
	    ptMon = new eoShiftMutation<EOT>;
	}
    else if ( 3 == RANK )
	{
	    ptMon = new eoTwoOptMutation<EOT>;
	}
    state.storeFunctor(ptMon);

    // /**********************************
    //  * Déclaration des composants DIM *
    //  **********************************/

    dim::evolver::Easy<EOT> evolver( eval, *ptMon );
    dim::feedbacker::async::Easy<EOT> feedbacker;
    dim::vectorupdater::Easy<EOT> updater(alpha, beta);
    dim::memorizer::Easy<EOT> memorizer;
    dim::migrator::async::Easy<EOT> migrator;
    dim::algo::Easy<EOT> island( evolver, feedbacker, updater, memorizer, migrator, checkpoint );

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

    cout << "size: " << dim::initialization::Graph::size() << endl;

    apply<EOT>(eval, pop);

    island( pop, data );

    world.abort(0);

    return 0 ;
}
