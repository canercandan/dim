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

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include <boost/serialization/vector.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/assume_abstract.hpp>

#include <boost/optional.hpp>

// #include <boost/numeric/ublas/matrix.hpp>
// #include <boost/numeric/ublas/io.hpp>

#include <stdexcept>
#include <iostream>
#include <iterator>
#include <algorithm>
#include <assert.h>

#include <chrono>

#include <eo>
// #include <ga.h>
// #include <ga/make_ga.h>
#include <utils/eoFuncPtrStat.h>
#include <do/make_continue.h>
#include <eompi.h>

#include <dim/dim>

using namespace std;
using namespace eo::mpi;
using namespace MPI;
using namespace boost::mpi;
using namespace boost;
// using namespace boost::numeric/*::ublas*/;

typedef dim::core::Bit<double> EOT;

/****************************************
 * Define MPI tags used for the purpose *
 ****************************************/

enum { INDIVIDUALS=0, FEEDBACKS=1, END=2 };

int main(int argc, char *argv[])
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
    /*size_t reward = */parser.createParam(size_t(2), "reward", "reward", 'r', "Islands Model").value();
    /*size_t penalty = */parser.createParam(size_t(1), "penalty", "penalty", 0, "Islands Model").value();
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

    unsigned chromSize = parser.getORcreateParam(unsigned(1500), "chromSize", "The length of the bitstrings", 'n',"Problem").value();
    eoInit<EOT>& init = dim::do_make::genotype(parser, state, EOT(), 0);

    // string nklInstance =  parser.getORcreateParam(string(), "nklInstance", "filename of the instance for NK-L problem", 0, "Problem").value();

    dim::evaluation::OneMax<EOT> mainEval;
    // my::nkLandscapesEval<EOT> mainEval;
    eoEvalFuncCounter<EOT> eval(mainEval);

    /*unsigned popSize = */parser.getORcreateParam(unsigned(100), "popSize", "Population Size", 'P', "Evolution Engine")/*.value()*/;
    dim::core::Pop<EOT>& pop = dim::do_make::pop(parser, state, init);

    /*double targetFitness = */parser.getORcreateParam(double(chromSize), "targetFitness", "Stop when fitness reaches",'T', "Stopping criterion")/*.value()*/;
    dim::continuator::Base<EOT>& continuator = dim::do_make::continuator<EOT>(parser, state, eval);

    vector< double > vecProba( ALL );
    vector< double > vecProbaRet( ALL );

    dim::core::IslandData<EOT> data;

    dim::utils::CheckPoint<EOT>& checkpoint = dim::do_make::checkpoint(parser, state, continuator, data);

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

    dim::evolver::Easy<EOT> evolver( eval, *ptMon );
    dim::feedbacker::Easy<EOT> feedbacker;
    dim::inputprobasender::Easy<EOT> probasender;
    dim::vectorupdater::Easy<EOT> updater(alpha, beta);
    dim::memorizer::Easy<EOT> memorizer;
    dim::migrator::Easy<EOT> migrator;
    dim::algo::EasyIsland<EOT> island( evolver, feedbacker, probasender, updater, memorizer, migrator, checkpoint );

    /**************
     * EO routine *
     **************/

    make_parallel(parser);
    make_verbose(parser);
    make_help(parser);

    // mainEval.load(nklInstance.c_str()); // nklandscapeseval specific

    /******************************************************************************
     * Création de la matrice de transition et distribution aux iles des vecteurs *
     ******************************************************************************/

    dim::core::MigrationMatrix probabilities( ALL );
    dim::core::InitMatrix initmatrix( initG, probaSame );

    // ublas::matrix< double > m(ALL, ALL);

    if ( 0 == RANK )
    	{
	    initmatrix( probabilities );
    	    cout << probabilities;
	    vecProba = probabilities(RANK);
	    for (size_t j = 0; j < ALL; ++j)
		{
		    vecProbaRet[j] = probabilities(j,RANK);
		}

	    for (size_t i = 1; i < ALL; ++i)
		{
		    world.send( i, 100, probabilities(i) );
		    vector< double > vecProbaRetIsl( ALL );
		    for (size_t j = 0; j < ALL; ++j)
			{
			    vecProbaRetIsl[j] = probabilities(j,i);
			}
		    world.send( i, 101, vecProbaRetIsl );
		}
    	}
    else
	{
	    world.recv( 0, 100, vecProba );
	    world.recv( 0, 101, vecProbaRet );
	}

    /******************************************
     * Get the population size of all islands *
     ******************************************/

    world.barrier();
    dim::utils::print_sum(pop);

    /***************
     * Rock & Roll *
     ***************/

    apply<EOT>(eval, pop);

    for (auto &ind : pop)
	{
	    ind.addIsland(RANK);
	}

    vector< typename EOT::Fitness > vecAvg(ALL, 0);
    vector< typename EOT::Fitness > vecFeedbacks(ALL, 0);
    vector< size_t > outputSizes(ALL, 0);

    // ostringstream ss;

    // ss << "gen.time." << RANK;
    // ofstream gen_time(ss.str());
    // ss.str("");

    // ss << "fb.time." << RANK;
    // ofstream fb_time(ss.str());
    // ss.str("");

    // ss << "vp.time." << RANK;
    // ofstream vp_time(ss.str());
    // ss.str("");

    // ss << "mig.time." << RANK;
    // ofstream mig_time(ss.str());
    // ss.str("");

    // ss << "eval.time." << RANK;
    // ofstream eval_time(ss.str());
    // ss.str("");

    // ss << "com.time." << RANK;
    // ofstream com_time(ss.str());
    // ss.str("");

    // ss << "noncom.time." << RANK;
    // ofstream noncom_time(ss.str());
    // ss.str("");

    long elapsed_mean = 0;
    long elapsed_sum = 0;
    long n = 0;

    // island( pop );

    while ( checkpoint(pop) )
	{
	    auto gen_start = chrono::system_clock::now();
	    long com_elapsed = 0;

	    /**************************************************
	     * Initialize a few variables for each generation *
	     **************************************************/

	    eval.value(0);

	    vector< request > reqs;

	    /**********
	     * Evolve *
	     **********/

	    {
		// auto start = chrono::system_clock::now();

		for (auto &ind : pop)
		    {
			EOT candidate = ind;

			(*ptMon)( candidate );

			candidate.invalidate();
			eval( candidate );

			if ( candidate.fitness() > ind.fitness() )
			    {
				ind = candidate;
			    }
		    }

		// auto end = chrono::system_clock::now();

		//long elapsed = chrono::duration_cast<chrono::microseconds>(end-start).count();
		// eval_time << elapsed << " "; eval_time.flush();

		// cout << "eval elapsed microseconds: " << elapsed << endl; cout.flush();
	    }

	    /************************************************
	     * Send feedbacks back to all islands (ANALYSE) *
	     ************************************************/
	    {
		auto start = chrono::system_clock::now();

		vector<typename EOT::Fitness> sums(ALL, 0);
		vector<int> nbs(ALL, 0);
		for (auto &ind : pop)
		    {
			sums[ind.getLastIsland()] += ind.fitness() - ind.getLastFitness();
			++nbs[ind.getLastIsland()];
		    }

		for (size_t i = 0; i < ALL; ++i)
		    {
			if (i == RANK) { continue; }
			reqs.push_back( world.isend( i, FEEDBACKS, nbs[i] > 0 ? sums[i] / nbs[i] : 0 ) );
		    }

		/**************************************
		 * Receive feedbacks from all islands *
		 **************************************/

		for (size_t i = 0; i < ALL; ++i)
		    {
			if (i == RANK) { continue; }
			reqs.push_back( world.irecv( i, FEEDBACKS, vecFeedbacks[i] ) );
		    }

		vecFeedbacks[RANK] = nbs[RANK] > 0 ? sums[RANK] / nbs[RANK] : 0;

		/****************************
		 * Process all MPI requests *
		 ****************************/

		wait_all( reqs.begin(), reqs.end() );
		reqs.clear();

		auto end = chrono::system_clock::now();

		long elapsed = chrono::duration_cast<chrono::microseconds>(end-start).count();
		// fb_time << elapsed << " "; fb_time.flush();
		com_elapsed += elapsed;
	    }

	    /***********************************
	     * Send vecProbaRet to all islands *
	     ***********************************/
	    {
		auto start = chrono::system_clock::now();

		for (size_t i = 0; i < ALL; ++i)
		    {
			if (i == RANK) { continue; }
			reqs.push_back( world.isend( i, 5, vecProba[i] ) );
		    }

		/**************************************
		 * Receive vecProbaRet from all islands *
		 **************************************/

		for (size_t i = 0; i < ALL; ++i)
		    {
			if (i == RANK) { continue; }
			reqs.push_back( world.irecv( i, 5, vecProbaRet[i] ) );
		    }

		vecProbaRet[RANK] = vecProba[RANK];

		/****************************
		 * Process all MPI requests *
		 ****************************/

		wait_all( reqs.begin(), reqs.end() );
		reqs.clear();

		auto end = chrono::system_clock::now();

		long elapsed = chrono::duration_cast<chrono::microseconds>(end-start).count();
		// vp_time << elapsed << " "; vp_time.flush();
		com_elapsed += elapsed;
	    }


	    /******************************************************************
	     * Memorize last fitness and island of population before evolving *
	     ******************************************************************/

	    for (auto &indi : pop)
		{
		    indi.addFitness();
		    indi.addIsland(RANK);
		}

	    /***********************************
	     * Send individuals to all islands *
	     ***********************************/
	    {
		auto start = chrono::system_clock::now();

		{
		    vector< dim::core::Pop<EOT> > pops( ALL );

		    /*************
		     * Selection *
		     *************/

		    for (auto &indi : pop)
			{
			    double s = 0;
			    int r = rng.rand() % 1000 + 1;

			    size_t j;
			    for ( j = 0; j < ALL && r > s; ++j )
				{
				    s += vecProba[j];
				}
			    --j;

			    pops[j].push_back(indi);
			}

		    size_t outputSize = 0;

		    for (size_t i = 0; i < ALL; ++i)
			{
			    if (i == RANK) { continue; }
			    outputSizes[i] = pops[i].size();
			    outputSize += pops[i].size();
			}

		    pop.setOutputSizes( outputSizes );
		    pop.setOutputSize( outputSize );

		    pop.clear();

		    for ( size_t i = 0; i < ALL; ++i )
			{
			    if (i == RANK) { continue; }
			    reqs.push_back( world.isend( i, INDIVIDUALS, pops[i] ) );
			}

		    dim::core::Pop<EOT>& newpop = pops[RANK];
		    for (auto &indi : newpop)
			{
			    pop.push_back( indi );
			}
		}

		vector< dim::core::Pop<EOT> > pops( ALL );

		/****************************************
		 * Receive individuals from all islands *
		 ****************************************/
		{
		    for (size_t i = 0; i < ALL; ++i)
			{
			    if (i == RANK) { continue; }
			    reqs.push_back( world.irecv( i, INDIVIDUALS, pops[i] ) );
			}
		}

		/****************************
		 * Process all MPI requests *
		 ****************************/

		wait_all( reqs.begin(), reqs.end() );
		reqs.clear();

		auto end = chrono::system_clock::now();

		long elapsed = chrono::duration_cast<chrono::microseconds>(end-start).count();
		// mig_time << elapsed << " "; mig_time.flush();
		com_elapsed += elapsed;

		/*********************
		 * Update population *
		 *********************/
		{
		    size_t inputSize = 0;

		    for (size_t i = 0; i < ALL; ++i)
			{
			    if (i == RANK) { continue; }

			    dim::core::Pop<EOT>& newpop = pops[i];
			    for (auto &indi : newpop)
				{
				    pop.push_back( indi );
				}

			    inputSize += newpop.size();
			}

		    pop.setInputSize( inputSize );
		}

	    }

	    auto gen_end = chrono::system_clock::now();
	    long elapsed_microseconds = chrono::duration_cast<chrono::microseconds>(gen_end-gen_start).count();
	    // gen_time << elapsed_microseconds << " "; gen_time.flush();
	    // com_time << com_elapsed << " "; com_time.flush();
	    // noncom_time << elapsed_microseconds - com_elapsed << " "; noncom_time.flush();

	    ++n;
	    elapsed_mean += (elapsed_microseconds - elapsed_mean) / n;
	    elapsed_sum += elapsed_microseconds;

	    // cout << "generation elapsed microseconds: " << chrono::duration_cast<chrono::microseconds>(gen_end-gen_start).count()
	    // 	 << " mean: " << elapsed_mean
	    // 	 << " sum: " << elapsed_sum
	    // 	 << endl; cout.flush();
	}

    world.abort(0);

    // /*************************************************************************
    //  * MAJ de la matrice de transition et récupération des vecteurs des iles *
    //  *************************************************************************/

    // world.barrier();
    // if ( RANK > 0 )
    // 	{
    // 	    world.send( 0, 0, vecProba );
    // 	}
    // else
    // 	{
    // 	    for (int i = 1; i < ALL; ++i)
    // 		{
    // 		    vector<double> proba(ALL);
    // 		    world.recv( i, 0, proba );
    // 		    for (int j = 0; j < proba.size(); ++j)
    // 			{
    // 			    probabilities(i,j) = proba[j];
    // 			}
    // 		}
    // 	    for (int j = 0; j < vecProba.size(); ++j)
    // 		{
    // 		    probabilities(0,j) = vecProba[j];
    // 		}

    // 	    cout << probabilities;
    // 	    cout.flush();
    // 	}

    /******************************************
     * Get the population size of all islands *
     ******************************************/

    // world.barrier();
    // dim::utils::print_sum(pop);

    /*********
     * DEBUG *
     *********/

    // world.barrier();
    // eo::log << eo::progress;
    // copy(vecAvg.begin(), vecAvg.end(), ostream_iterator<typename EOT::Fitness>(eo::log, " "));
    // eo::log << endl; eo::log.flush();

    return 0;
}
