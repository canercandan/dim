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
	    Easy( double alpha = 0.2 /*1-0.8*/, double beta = 0.01 /*1-0.99*/ ) : _alpha(alpha), _beta(beta)
	    {
#ifdef TRACE
		std::ostringstream ss;
		ss << "trace.updater." << this->rank();
		_of.open(ss.str());
#endif // !TRACE
	    }

#if __cplusplus <= 199711L
	    static inline typename EOT::Fitness bounded_sum(typename EOT::Fitness x, typename EOT::Fitness y){ return std::max(x, typename EOT::Fitness(0)) + std::max(y, typename EOT::Fitness(0)); }
#endif

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
			R[i] = T[i] > tau ? S[i] : 0;
		    }

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
#if __cplusplus > 199711L
		auto sum_fits = std::accumulate(R.begin(), R.end(), 0., [](typename EOT::Fitness x, typename EOT::Fitness y){ return std::max(x, typename EOT::Fitness(0)) + std::max(y, typename EOT::Fitness(0)); } );
#else
		typename EOT::Fitness sum_fits = std::accumulate(R.begin(), R.end(), 0., bounded_sum );
#endif

#if __cplusplus > 199711L
		for (auto& fit : R)
		    {
#else
		for (size_t i = 0; i < R.size(); ++i)
		    {
			typename EOT::Fitness& fit = R[i];
#endif

			fit = fit > 0 ? fit / sum_fits * 1000 : 0;
		    }

		// computation of epsilon vector (norm is 1)
		double sum = 0;

		std::vector< double > epsilon( this->size() );

#if __cplusplus > 199711L
		for ( auto& k : epsilon )
		    {
#else
		for (size_t i = 0; i < epsilon.size(); ++i)
		    {
			double& k = epsilon[i];
#endif

			k = rng.rand() % 1000;
			sum += k;
		    }

#if __cplusplus > 199711L
		for ( auto& k : epsilon )
		    {
#else
		for (size_t i = 0; i < epsilon.size(); ++i)
		    {
			double& k = epsilon[i];
#endif

			k = sum ? k / sum : 0;
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
		AUTO(double) alphaT = exp(log(_alpha /*0.2*/)/elapsed);
		AUTO(double) betaT = exp(log(_beta /*0.01*/)/elapsed);

		// _of << deltaT << " "; _of.flush();

		for ( size_t i = 0; i < this->size(); ++i )
		    {
			// typename EOT::Fitness Ri = R[i] / sum_multi;

			// R[i] = 1000 / this->size();

#ifdef TRACE
			_of << "(" << i << ") "
			    << (1 - betaT) << " * ( "
			    << 1 - alphaT << " * "
			    << data.proba[i] << " + "
			    << alphaT << " * "
			    << R[i] << " )"
			    << " + " << betaT << " * " << 1000 << " * " << epsilon[i]
			    << " = ";
#endif // !TRACE

			data.proba[i] = (1 - betaT) * ( ( 1 - alphaT ) * data.proba[i] + alphaT * R[i] ) + betaT * 1000 * epsilon[i];

#ifdef TRACE
			_of << data.proba[i] << std::endl;
			_of.flush();
#endif // !TRACE

		    }

		tau = std_or_boost::chrono::system_clock::now();
	    }

	private:
	    double _alpha;
	    double _beta;

#ifdef TRACE
	    std::ofstream _of;
#endif // !TRACE

	};
    } // !vectorupdater
} // !dim

#endif /* _VECTORUPDATER_EASY_H_ */
