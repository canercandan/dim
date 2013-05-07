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

#ifndef _VECTORUPDATER_EASY_H_
#define _VECTORUPDATER_EASY_H_

#if __cplusplus > 199711L
#include <chrono>
#else
#include <boost/chrono/chrono_io.hpp>
#endif

#include <vector>
#include <algorithm>
#include <numeric>
#include <fstream>

#include "Base.h"

#include <boost/utility/identity_type.hpp>

#undef AUTO
#if __cplusplus > 199711L
#define AUTO(TYPE) auto
#else // __cplusplus <= 199711L
#define AUTO(TYPE) TYPE
#endif

namespace dim
{
    namespace vectorupdater
    {
#if __cplusplus > 199711L
	namespace std_or_boost = std;
#else
	namespace std_or_boost = boost;
#endif

	template <typename EOT>
	class Easy : public Base<EOT>
	{
	public:
	    Easy( double alpha = 0.2 /*1-0.8*/, double beta = 0.01 /*1-0.99*/, double sensitivity = 1., bool delta = true )
		: _alpha(alpha), _beta(beta), _sensitivity(sensitivity), _delta(delta)
	    {
#ifdef TRACE
		std::ostringstream ss;
		ss << "trace.updater." << this->rank();
		_of.open(ss.str().c_str());
#endif // !TRACE
	    }

	    static inline typename EOT::Fitness bounded_sum(typename EOT::Fitness x, typename EOT::Fitness y){ return std::max(x, typename EOT::Fitness(0)) + std::max(y, typename EOT::Fitness(0)); }

	    void operator()(core::Pop<EOT>& pop, core::IslandData<EOT>& data)
	    {
		if (pop.empty()) { return; }

		/****************************
		 * Update transition vector *
		 ****************************/

		// Prepare the vector R
		// Ri = { Si, if Ti > \tau; 0, otherwise }
		AUTO(typename BOOST_IDENTITY_TYPE((std::vector< typename EOT::Fitness >)))& S = data.feedbacks;
		AUTO(typename BOOST_IDENTITY_TYPE((std::vector< std_or_boost::chrono::time_point< std_or_boost::chrono::system_clock > >)))& T = data.feedbackLastUpdatedTimes;
		AUTO(typename BOOST_IDENTITY_TYPE((std_or_boost::chrono::time_point< std_or_boost::chrono::system_clock >)))& tau = data.vectorLastUpdatedTime;

		std::vector< typename EOT::Fitness > R( this->size() );
		for (size_t i = 0; i < this->size(); ++i)
		    {
			R[i] = T[i] >= tau ? S[i] : 0;
// #ifdef TRACE
// 			_of << R[i] << " ";
// #endif
		    }
// #ifdef TRACE
// 		_of << std::endl; std::cout.flush();
// #endif

		// Stategie par critère MAX
		// int best = -1;
		// typename EOT::Fitness max = 0;
		// for (size_t i = 0; i < this->size(); ++i)
		// 	{
		// 	    if (data.feedbacks[i] > max)
		// 		{
		// 		    best = i;
		// 		    max = data.feedbacks[i];
		// 		}
		// 	}

		// Stratégie par récompense proportionnelle
		{
		    typename EOT::Fitness sum_fits = std::accumulate(R.begin(), R.end(), 0., bounded_sum );

// #ifdef TRACE
// 		    _of << sum_fits << " "; _of.flush();
// #endif

		    if (sum_fits)
			{

			    unsigned sum_sum = 0;

			    for (size_t i = 0; i < R.size()-1; ++i)
				{
				    R[i] = R[i] > 0 ? R[i] / sum_fits * 1000 : 0;
				    sum_sum += R[i];
// #ifdef TRACE
// 			    _of << R[i] << " ";
// #endif
				}

			    R.back() = 1000-sum_sum;
// #ifdef TRACE
// 		    _of << R.back();
// 		    _of << std::endl; std::cout.flush();
// #endif

			}
		    else
			{
			    tau = std_or_boost::chrono::system_clock::now();

			    eo::log << eo::warnings << "S is null" << std::endl; eo::log.flush();

			    return;

			    // unsigned sum = 0;

			    // for ( size_t i = 0; i < this->size()-1; ++i )
			    // 	{
			    // 	    R[i] = 1000 / this->size();
			    // 	    sum += R[i];
			    // 	}

			    // R.back() = 1000-sum;
			}
		}

		// _of << std::accumulate(R.begin(), R.end(), 0., bounded_sum ) << " ";
		_of << R.back() << " ";


		// computation of epsilon vector (norm is 1)
		std::vector< double > epsilon( this->size() );

		{
		    unsigned sum = 0;

		    for (size_t i = 0; i < epsilon.size(); ++i)
			{
			    epsilon[i] = rng.rand() % 1000;
			    sum += epsilon[i];
			}

		    unsigned sum_sum = 0;

		    for (size_t i = 0; i < epsilon.size()-1; ++i)
			{
			    epsilon[i] = sum ? epsilon[i] / sum * 1000 : 0;
			    sum_sum += epsilon[i];
			}

		    epsilon.back() = 1000-sum_sum;
		}

		/*******************************************************************************
		 * Si p_i^t est le vecteur de migration de ile numéro i au temps t             *
		 * alors on peut faire un update "baysien" comme ça:                           *
		 *                                                                             *
		 * p_i^{t+1} =  b  *  ( a * p_i^t + (1 - a) * select )  +  (1 - b) * epsilon   *
		 *                                                                             *
		 * où:                                                                         *
		 * - a et b sont les coefficients qui reglent respectivement l'exploitation et *
		 * l'exploration                                                               *
		 * - select est le vecteur avec que des 0 sauf pour l'ile qui semble la plus   *
		 * prometteuse où cela vaut 1.                                                 *
		 * - epsilon un vecteur de 'bruit' (de norme 1) avec des coefficient           *
		 * uniforme entre 0.0 et 1.0                                                   *
		 *******************************************************************************/

		AUTO(double) elapsed = std_or_boost::chrono::duration_cast<std_or_boost::chrono::microseconds>( std_or_boost::chrono::system_clock::now() - tau ).count() / 1000.; // \DELTA{t}

// #ifdef TRACE
// 		_of << elapsed << " "; _of.flush();
// #endif // !TRACE

		if (!_delta)
		    {
			elapsed = 1.;
		    }

		AUTO(double) alphaT = _alpha ? exp(log(_alpha /*0.2*/)/(elapsed*_sensitivity)) : 0;
		AUTO(double) betaT = _beta ? exp(log(_beta /*0.01*/)/(elapsed*_sensitivity)) : 0;
		// AUTO(double) betaT = _beta;

		// std::cout << betaT << " "; std::cout.flush();

#ifdef TRACE
		_of << alphaT << " " << betaT << " "; _of.flush();
#endif // !TRACE
		// {
		//     unsigned sum = 0;

		//     for ( size_t i = 0; i < this->size()-1; ++i )
		// 	{
		// 	    R[i] = 1000 / this->size();
		// 	    sum += R[i];
		// 	}

		//     R.back() = 1000-sum;
		// }

		// std::cout << std::accumulate(R.begin(), R.end(), 0.) << " "; std::cout.flush();

		unsigned sum = 0;

		for ( size_t i = 0; i < this->size()-1; ++i )
		    {
			// typename EOT::Fitness Ri = R[i] / sum_multi;

// #ifdef TRACE
// 			_of << "(" << i << ") "
// 			    << (1 - betaT) << " * ( "
// 			    << 1 - alphaT << " * "
// 			    << data.proba[i] << " + "
// 			    << alphaT << " * "
// 			    << R[i] << " )"
// 			    << " + " << betaT << " * " << 1000 << " * " << epsilon[i]
// 			    << " = ";
// #endif // !TRACE

			data.proba[i] = (1 - betaT) * ( ( 1 - alphaT ) * data.proba[i] + alphaT * R[i] ) + betaT * epsilon[i];
			sum += data.proba[i];

// #ifdef TRACE
// 			_of << data.proba[i] << std::endl;
// 			_of.flush();
// #endif // !TRACE

		    }

		data.proba.back() = 1000-sum;

		tau = std_or_boost::chrono::system_clock::now();

#ifdef TRACE
		_of << std::endl; _of.flush();
#endif // !TRACE
	    }

	private:
	    double _alpha;
	    double _beta;
	    bool _delta;
	    double _sensitivity;

#ifdef TRACE
	    std::ofstream _of;
#endif // !TRACE

	};
    } // !vectorupdater
} // !dim

#endif /* _VECTORUPDATER_EASY_H_ */
