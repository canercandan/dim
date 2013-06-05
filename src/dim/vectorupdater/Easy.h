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
	class Reward : public core::IslandOperator<EOT>
	{
	public:
	    virtual void firstCall(core::Pop<EOT>&, core::IslandData<EOT>&) {}
	    virtual void lastCall(core::Pop<EOT>&, core::IslandData<EOT>&) {}
	};

	template <typename EOT>
	class Best : public Reward<EOT>
	{
	public:
	    Best( double alpha = 0.2 /*1-0.8*/, double beta = 0.01 /*1-0.99*/ )
		: _alpha(alpha), _beta(beta)
	    {}

	    virtual ~Best() {}

	    void operator()(core::Pop<EOT>& pop, core::IslandData<EOT>& data)
	    {
		AUTO(typename BOOST_IDENTITY_TYPE((std::vector< typename EOT::Fitness >)))& S = data.feedbacks;

		int best = -1;
		typename EOT::Fitness max = -1;
		for (size_t i = 0; i < this->size(); ++i)
		    {
			if (S[i] > max)
			    {
				best = i;
				max = S[i];
			    }
		    }

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

		unsigned sum = 0;
		for ( size_t i = 0; i < this->size()-1; ++i )
		    {
			if (best == -1) // no improvment then rebalancing
			    {
				data.proba[i] = (1-_beta)*data.proba[i] + _beta*epsilon[i];
			    }
			else
			    {
				if (i == best)
				    {
					data.proba[i] = (1-_beta)*((1-_alpha)*data.proba[i] + _alpha*1000) + _beta*epsilon[i];
				    }
				else
				    {
					data.proba[i] = (1-_beta)*((1-_alpha)*data.proba[i]) + _beta*epsilon[i];
				    }
			    }

			sum += data.proba[i];
		    }
		data.proba.back() = 1000-sum;
	    }

	private:
	    double _alpha;
	    double _beta;
	};

	template <typename EOT>
	class Proportional : public Reward<EOT>
	{
	public:
	    Proportional( double alpha = 0.2 /*1-0.8*/, double beta = 0.01 /*1-0.99*/, double sensitivity = 1., bool delta = true )
		: _alpha(alpha), _beta(beta), _sensitivity(sensitivity), _delta(delta)
	    {}

	    virtual ~Proportional() {}

	    static inline typename EOT::Fitness bounded_sum(typename EOT::Fitness x,
							    typename EOT::Fitness y)
	    {
		return std::max(x, typename EOT::Fitness(0)) + std::max(y, typename EOT::Fitness(0));
	    }

	    void operator()(core::Pop<EOT>& pop, core::IslandData<EOT>& data)
	    {
		AUTO(typename BOOST_IDENTITY_TYPE((std::vector< typename EOT::Fitness >)))& S = data.feedbacks;
		AUTO(typename BOOST_IDENTITY_TYPE((std::vector< std_or_boost::chrono::time_point< std_or_boost::chrono::system_clock > >)))& T = data.feedbackLastUpdatedTimes;
		AUTO(typename BOOST_IDENTITY_TYPE((std_or_boost::chrono::time_point< std_or_boost::chrono::system_clock >)))& tau = data.vectorLastUpdatedTime;

		std::vector< typename EOT::Fitness > R( this->size() );
		for (size_t i = 0; i < this->size(); ++i)
		    {
			R[i] = T[i] >= tau ? S[i] : 0;
		    }

		typename EOT::Fitness sum_fits = std::accumulate(R.begin(), R.end(), 0., bounded_sum );
		if (sum_fits)
		    {
			unsigned sum_sum = 0;
			for (size_t i = 0; i < R.size()-1; ++i)
			    {
				R[i] = R[i] > 0 ? R[i] / sum_fits * 1000 : 0;
				sum_sum += R[i];
			    }
			R.back() = 1000-sum_sum;
		    }
		else
		    {
			tau = std_or_boost::chrono::system_clock::now();
			eo::log << eo::warnings << "S is null" << std::endl; eo::log.flush();
			return;
		    }

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

		AUTO(double) elapsed = std_or_boost::chrono::duration_cast<std_or_boost::chrono::microseconds>( std_or_boost::chrono::system_clock::now() - tau ).count() / 1000.; // \DELTA{t}

		if (!_delta)
		    {
			elapsed = 1.;
		    }

		AUTO(double) alphaT = _alpha ? exp(log(_alpha /*0.2*/)/(elapsed*_sensitivity)) : 0;
		AUTO(double) betaT = _beta ? exp(log(_beta /*0.01*/)/(elapsed*_sensitivity)) : 0;

		unsigned sum = 0;
		for ( size_t i = 0; i < this->size()-1; ++i )
		    {
			data.proba[i] = (1 - betaT) * ( ( 1 - alphaT ) * data.proba[i] + alphaT * R[i] ) + betaT * epsilon[i];

			sum += data.proba[i];
		    }

		data.proba.back() = 1000-sum;

		tau = std_or_boost::chrono::system_clock::now();
	    }

	private:
	    double _alpha;
	    double _beta;
	    bool _delta;
	    double _sensitivity;
	};

	template <typename EOT>
	class Easy : public Base<EOT>
	{
	public:
	    Easy( Reward<EOT>& reward )
		: _reward(reward)
	    {
#ifdef TRACE
		std::ostringstream ss;
		ss << "trace.updater." << this->rank();
		_of.open(ss.str().c_str());
#endif // !TRACE
	    }

	    void operator()(core::Pop<EOT>& pop, core::IslandData<EOT>& data)
	    {
		_reward(pop, data);
	    }

	private:
	    Reward<EOT>& _reward;

#ifdef TRACE
	    std::ofstream _of;
#endif // !TRACE
	};
    } // !vectorupdater
} // !dim

#endif /* _VECTORUPDATER_EASY_H_ */
