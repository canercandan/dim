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

#include <vector>
#include <algorithm>
#include <numeric>
#include <fstream>

#include "Base.h"

namespace dim
{
    namespace vectorupdater
    {
	namespace sync
	{
	    template <typename EOT>
	    class Easy : public Base<EOT>
	    {
	    public:
		Easy( double alpha = 0.8, double beta = 0.99 ) : _alpha(alpha), _beta(beta) {}

		void operator()(core::Pop<EOT>& /*pop*/, core::IslandData<EOT>& data)
		{
		    /****************************
		     * Update transition vector *
		     ****************************/
		    int best = -1;
		    typename EOT::Fitness max = -1;

		    for (size_t i = 0; i < this->size(); ++i)
			{
			    if (data.feedbacks[i] > max)
				{
				    best = i;
				    max = data.feedbacks[i];
				}
			}

		    // computation of epsilon vector (norm is 1)
		    double sum = 0;

		    std::vector< double > epsilon( this->size() );

		    for ( auto &k : epsilon )
			{
			    k = rng.rand() % 1000;
			    sum += k;
			}

		    for ( auto &k : epsilon )
			{
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

		    if (best < 0) //aucune ile n'améliore donc on rééquilibre
			{
			    for ( size_t i = 0; i < this->size(); ++i )
				{
				    data.proba[i] = _beta * data.proba[i] + (1 - _beta) * 1000 * epsilon[i];
				}
			}
		    else
			{
			    for (size_t i = 0; i < this->size(); ++i)
				{
				    if ( static_cast<int>(i) == best )
					{
					    data.proba[i] = _beta * ( _alpha * data.proba[i] + (1 - _alpha) * 1000 ) + (1 - _beta) * 1000 * epsilon[i];
					}
				    else
					{
					    data.proba[i] = _beta * ( _alpha * data.proba[i] ) + (1 - _beta) * 1000 * epsilon[i];
					}
				}
			}
		}

	    private:
		double _alpha;
		double _beta;
	    };
	} // !sync

	namespace async
	{
	    template <typename EOT>
	    class Easy : public Base<EOT>
	    {
	    public:
		Easy( double alpha = 0.8, double beta = 0.99 ) : _alpha(alpha), _beta(beta)
		{
		    std::ostringstream ss;
		    ss << "trace.updater." << this->rank() << ".txt";
		    _of.open(ss.str());
		}

		template <typename T>
		static T positive_add(T x, T y)
		{
		    return std::max(x, 0.) + std::max(y, 0.);
		}

		void compute(core::Pop<EOT>& pop, core::IslandData<EOT>& data)
		{
		    if (pop.empty()) { return; }

		    /****************************
		     * Update transition vector *
		     ****************************/

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
		    auto sum_fits = std::accumulate(data.feedbacks.begin(), data.feedbacks.end(), 0., positive_add< typename EOT::Fitness >);
		    auto proportionalFeedbacks = data.feedbacks;
		    // _of << "[";
		    for (auto& fit : proportionalFeedbacks)
		    	{
			    fit = fit > 0 ? fit / sum_fits * 1000 : 0;
		    	    // _of << fit << ",";
		    	}
 		    // _of << "] "; _of.flush();
		    // _of << std::accumulate(proportionalFeedbacks.begin(), proportionalFeedbacks.end(), 0., positive_add< typename EOT::Fitness >) << " "; _of.flush();


		    // computation of epsilon vector (norm is 1)
		    double sum = 0;

		    std::vector< double > epsilon( this->size() );

		    for ( auto &k : epsilon )
			{
			    k = rng.rand() % 1000;
			    sum += k;
			}

		    for ( auto &k : epsilon )
			{
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

		    // Stategie par critère MAX
		    // if (best < 0) //aucune ile n'améliore donc on rééquilibre
		    // 	{
		    // 	    for ( size_t i = 0; i < this->size(); ++i )
		    // 		{
		    // 		    data.proba[i] = _beta * data.proba[i] + (1 - _beta) * 1000 * epsilon[i];
		    // 		}
		    // 	}
		    // else
		    // 	{
		    // 	    for (size_t i = 0; i < this->size(); ++i)
		    // 		{
		    // 		    if ( static_cast<int>(i) == best )
		    // 			{
		    // 			    data.proba[i] = _beta * ( _alpha * data.proba[i] + (1 - _alpha) * 1000 ) + (1 - _beta) * 1000 * epsilon[i];
		    // 			}
		    // 		    else
		    // 			{
		    // 			    data.proba[i] = _beta * ( _alpha * data.proba[i] ) + (1 - _beta) * 1000 * epsilon[i];
		    // 			}
		    // 		}
		    // 	}

		    // _of << std::accumulate(data.proba.begin(), data.proba.end(), 0., positive_add< typename EOT::Fitness >) << " "; _of.flush();

		    typename EOT::Fitness sum_multi = 0;
		    for (size_t i = 0; i < this->size(); ++i)
			{
			    sum_multi += data.proba[i] * proportionalFeedbacks[i] / 1000;
			    // _of << data.proba[i] << " * " << proportionalFeedbacks[i] << " / 1000 = " << data.proba[i] * proportionalFeedbacks[i] / 1000 << std::endl; _of.flush();
			}

		    // _of << std::accumulate(data.proba.begin(), data.proba.end(), 0., positive_add< typename EOT::Fitness >) << " "; _of.flush();

		    // Stratégie par récompense proportionnelle
		    for ( size_t i = 0; i < this->size(); ++i )
			{
			    // _of << "[" << data.proba[i] / 1000. << "*" << proportionalFeedbacks[i] << "=" << data.proba[i] * proportionalFeedbacks[i] / 1000. << "] "; _of.flush();
			    // _of << epsilon[i] << " "; _of.flush();
			    // data.proba[i] = _beta * ( _alpha * data.proba[i] + (1 - _alpha) * data.proba[i] * proportionalFeedbacks[i] / 1000 ) + (1 - _beta) * 1000 * epsilon[i];
			    // 0.99×(0.8×200+(0.2×(0.2×0.5)×1000))+(0.01×1000×0.1) = 179.2

			    // data.proba[i] = _beta * ( _alpha * data.proba[i] + ( this->rank() == i ? (1 - _alpha) * 1000 : 0 ) /* * proportionalFeedbacks[i] / 1000*/ ) + (1 - _beta) * 1000 * epsilon[i];
			    typename EOT::Fitness res = ( data.proba[i] * proportionalFeedbacks[i] ) / sum_multi;
			    _of << res << " "; _of.flush();
			    data.proba[i] = _beta * ( _alpha * data.proba[i] + (1 - _alpha) * res ) + (1 - _beta) * 1000 * epsilon[i];

			}

		    // _of << std::accumulate(data.proba.begin(), data.proba.end(), 0., positive_add< typename EOT::Fitness >) << " "; _of.flush();
		    // _of << std::accumulate(proportionalFeedbacks.begin(), proportionalFeedbacks.end(), 0., positive_add< typename EOT::Fitness >) << " "; _of.flush();

		}

		void communicate(core::Pop<EOT>& /*pop*/, core::IslandData<EOT>& /*data*/)
		{
		    // empty
		}

	    private:
		double _alpha;
		double _beta;
		std::ofstream _of;
	    };
	} // !async
    }
}

#endif /* _VECTORUPDATER_EASY_H_ */
