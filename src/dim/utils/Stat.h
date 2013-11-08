// -*- mode: c++; c-indent-level: 4; c++-member-init-indent: 8; comment-column: 35; -*-

//-----------------------------------------------------------------------------
// Stat.h
// (c) Marc Schoenauer, Maarten Keijzer and GeNeura Team, 2000
// (c) 2010 Thales group
/*
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

  Contact: http://eodev.sourceforge.net

  Authors:
  todos@geneura.ugr.es, http://geneura.ugr.es
  Marc.Schoenauer@polytechnique.fr
  mkeijzer@dhi.dk
  Johann Dréo <johann.dreo@thalesgroup.com>
*/
//-----------------------------------------------------------------------------

#ifndef _UTILS_STAT_H_
#define _UTILS_STAT_H_

#include <numeric> // accumulate

#include <eoFunctor.h>
#include <utils/eoParam.h>

#include <utils/eoLogger.h>

#include <dim/core/Pop.h>

// #include "CheckPoint.h"
#include "Monitor.h"

namespace dim
{
    namespace utils
    {

	/** @defgroup Stats Statistics computation
	 *
	 * Compute various statistics on a population.
	 *
	 * Objects of those classes are generally called by an CheckPoint
	 * to compute statistics about the population at a given generation.
	 * As they inherit from eoValueParam, they can be printed drectly,
	 * for instance by an Monitor.
	 *
	 * @see CheckPoint
	 * @see Monitor
	 *
	 * @ingroup Utilities
	 * @{
	 */

	/**
	   Base class for all statistics that need to be calculated
	   over the (unsorted) population
	   (I guess it is not really necessary? MS.
	   Depstd::ends, there might be reasons to have a stat that is not an eoValueParam,
	   but maybe I'm just kidding myself, MK)
	*/
	template <class EOT>
	class StatBase : public eoUF<const core::Pop<EOT>&, void>
	{
	public:
	    virtual void lastCall(const core::Pop<EOT>&) {}
	    virtual std::string className(void) const { return "StatBase"; }
	};


	template <class EOT> class CheckPoint;

	/**
	   The actual class that will be used as base for all statistics
	   that need to be calculated over the (unsorted) population
	   It is an StatBase AND an eoValueParam so it can be used in Monitors.
	*/
	template <class EOT, class T>
	class Stat : public eoValueParam<T>, public StatBase<EOT>
	{
	public:

	    Stat(T _value, std::string _description)
		: eoValueParam<T>(_value, _description)
	    {}

	    virtual std::string className(void) const
	    { return "Stat"; }


	    Stat<EOT, T>& addTo(CheckPoint<EOT>& cp)        { cp.add(*this);  return *this; }
	    Stat<EOT, T>& addTo(Monitor& mon)               { mon.add(*this); return *this; }
	};



	/**
	   Base class for statistics calculated over a sorted snapshot of the population
	*/
	template <class EOT>
	class SortedStatBase : public eoUF<const std::vector<const EOT*>&, void>
	{
	public:
	    virtual void lastCall(const std::vector<const EOT*>&) {}
	    virtual std::string className(void) const { return "SortedStatBase"; }

	};

	/**
	   The actual class that will be used as base for all statistics
	   that need to be calculated over the sorted population
	   It's an SortedStatBase AND an eoValueParam so it can be used in Monitors.
	*/
	template <class EOT, class ParamType>
	class SortedStat : public SortedStatBase<EOT>, public eoValueParam<ParamType>
	{
	public :
	    SortedStat(ParamType _value, std::string _desc) : eoValueParam<ParamType>(_value, _desc) {}
	    virtual std::string className(void) const { return "SortedStat"; }

	    SortedStat<EOT, ParamType>& addTo(CheckPoint<EOT>& cp)        { cp.add(*this);  return *this; }
	    SortedStat<EOT, ParamType>& addTo(Monitor& mon)               { mon.add(*this); return *this; }
	};

	/**
	   Average fitness of a population. Fitness can be:
	   - double
	   - eoMinimizingFitness or eoMaximizingFitness
	   The average of each objective is evaluated.

	   ( For eoScalarFitnessAssembled user eoAssembledFitnessStat classes.)
	*/
#if  defined(_MSC_VER) && (_MSC_VER < 1300)
	template <class EOT> class AverageStat : public Stat<EOT, EOT::Fitness>
#else
	template <class EOT> class AverageStat : public Stat<EOT, typename EOT::Fitness>
#endif
	{
	public :

	    using Stat<EOT, typename EOT::Fitness>::value;

	    typedef typename EOT::Fitness Fitness;

	    AverageStat(std::string _description = "Average Fitness")
		: Stat<EOT, Fitness>(Fitness(), _description) {}

	    static Fitness sumFitness(double _sum, const EOT& _eot){
		_sum += _eot.fitness();
		return _sum;
	    }

	    AverageStat(double _value, std::string _desc) : Stat<EOT, double>(_value, _desc) {}

	    virtual void operator()(const core::Pop<EOT>& _pop){
		if (_pop.empty())
		    {
			eo::log << eo::logging << "AverageStat: Population empty\n";
			return;
		    }

		doit(_pop, Fitness()); // specializations for scalar and std::vector
	    }

	    virtual std::string className(void) const { return "AverageStat"; }

	private :

	    // Default behavior
	    template <class T>
	    void doit(const core::Pop<EOT>& _pop, T)
	    {
		Fitness v = std::accumulate(_pop.begin(), _pop.end(), Fitness(0.0), AverageStat::sumFitness);

		value() = v / _pop.size();
	    }

	};

	template <class EOT>
	class SumOfSquares : public Stat<EOT, double>
	{
	public:
	    using Stat<EOT, double>::value;

	    typedef typename EOT::Fitness fitness_type;

	    SumOfSquares(std::string _description = "SumOfSquares")
		: Stat<EOT, double>(fitness_type(0.0), _description)
	    {}

	    static double sumOfSquares(double _sq, const EOT& _eo)
	    {
		double fitness = _eo.fitness();
		_sq += fitness * fitness;
		return _sq;
	    }

	    virtual void operator()(const core::Pop<EOT>& _pop)
	    {
		value() = std::accumulate(_pop.begin(), _pop.end(), fitness_type(0.0), SumOfSquares::sumOfSquares);
	    }

	    virtual std::string className(void) const { return "SumOfSquares"; }
	};


	/**
	   Average fitness + Std. dev. of a population, fitness needs to be scalar.
	*/
	template <class EOT>
	class SecondMomentStats : public Stat<EOT, std::pair<double, double> >
	{
	public :

	    using Stat<EOT, std::pair<double, double> >::value;

	    typedef typename EOT::Fitness fitness_type;

	    typedef std::pair<double, double> SquarePair;

	    SecondMomentStats(std::string _description = "Average & Stdev")
		: Stat<EOT, SquarePair>(std::make_pair(0.0,0.0), _description)
	    {}

	    static SquarePair sumOfSquares(SquarePair _sq, const EOT& _eo)
	    {
		double fitness = _eo.fitness();

		_sq.first += fitness;
		_sq.second += fitness * fitness;
		return _sq;
	    }

	    virtual void operator()(const core::Pop<EOT>& _pop)
	    {
		SquarePair result = std::accumulate(_pop.begin(), _pop.end(), std::make_pair(0.0, 0.0), SecondMomentStats::sumOfSquares);

		double n = _pop.size();
		value().first = result.first / n; // average
		value().second = sqrt( (result.second - n * value().first * value().first) / (n - 1.0)); // stdev
	    }

	    virtual std::string className(void) const { return "SecondMomentStats"; }
	};

	/**
	   The n_th element fitness in the population (see eoBestFitnessStat)
	*/
#if  defined(_MSC_VER) && (_MSC_VER < 1300)
	template <class EOT>
	class NthElementFitnessStat : public SortedStat<EOT, EOT::Fitness >
#else
	template <class EOT>
	class NthElementFitnessStat : public SortedStat<EOT, typename EOT::Fitness >
#endif
	{
	public :
	    using SortedStat<EOT, typename EOT::Fitness >::value;

	    typedef typename EOT::Fitness Fitness;

	    NthElementFitnessStat(unsigned _whichElement, std::string _description = "nth element fitness")
		: SortedStat<EOT, Fitness>(Fitness(), _description), whichElement(_whichElement) {}

	    virtual void operator()(const std::vector<const EOT*>& _pop)
	    {
		if (whichElement > _pop.size())
		    throw std::logic_error("fitness requested of element outside of pop");

		doit(_pop, Fitness());
	    }

	    virtual std::string className(void) const { return "NthElementFitnessStat"; }
	private :

	    struct CmpFitness
	    {
		CmpFitness(unsigned _whichElement, bool _maxim) : whichElement(_whichElement), maxim(_maxim) {}

		bool operator()(const EOT* a, const EOT* b)
		{
		    if (maxim)
			return a->fitness()[whichElement] > b->fitness()[whichElement];

		    return a->fitness()[whichElement] < b->fitness()[whichElement];
		}

		unsigned whichElement;
		bool maxim;
	    };

	    // for everything else
	    template <class T>
	    void doit(const std::vector<const EOT*>& _pop, T)
	    {
		value() = _pop[whichElement]->fitness();
	    }

	    unsigned whichElement;
	};

	/* Actually, you shouldn't need to sort the population to get the best fitness
	   MS - 17/11/00

	   But then again, if another stat needs sorted fitness anyway, getting the best
	   out would be very fast.
	   MK - 09/01/03

	   template <class EOT>
	   class BestFitnessStat : public Stat<EOT, typename EOT::Fitness >
	   {
	   public :
	   typedef typename EOT::Fitness Fitness;

	   BestFitnessStat(std::string _description = "Best Fitness") :
	   Stat<EOT, Fitness>(Fitness(), _description) {}

	   virtual void operator()(const core::Pop<EOT>& _pop)
	   {
	   value() = _pop.nth_element_fitness(0);
	   }

	   };
	*/

	/**
	   Best fitness of a population. Fitness can be:
	   - double
	   - eoMinimizingFitness or eoMaximizingFitness

	   ( For eoScalarFitnessAssembled look at eoAssembledFitnessStat )
	*/

#if  defined(_MSC_VER) && (_MSC_VER < 1300)
	template <class EOT>
	class BestFitnessStat : public Stat<EOT, EOT::Fitness>
#else
	template <class EOT>
	class BestFitnessStat : public Stat<EOT, typename EOT::Fitness>
#endif
	{
	public:

	    using Stat<EOT, typename EOT::Fitness>::value;

	    typedef typename EOT::Fitness Fitness;

	    BestFitnessStat(std::string _description = "Best ")
		: Stat<EOT, Fitness>(Fitness(), _description)
	    {}

	    void operator()(const core::Pop<EOT>& _pop)
	    {
		if (_pop.empty())
		    {
			eo::log << eo::logging << "BestFitnessStat: Population empty\n";
			return;
		    }

		doit(_pop, Fitness() ); // specializations for scalar and std::vector
	    }

	    virtual std::string className(void) const { return "BestFitnessStat"; }


	private :

	    struct CmpFitness
	    {
		CmpFitness(unsigned _which, bool _maxim) : which(_which), maxim(_maxim) {}

		bool operator()(const EOT& a, const EOT& b)
		{
		    if (maxim)
			return a.fitness()[which] < b.fitness()[which];

		    return a.fitness()[which] > b.fitness()[which];
		}

		unsigned which;
		bool maxim;
	    };

	    // default
	    template<class T>
	    void doit(const core::Pop<EOT>& _pop, T)
	    { // find the largest elements
		value() = _pop.best_element().fitness();
	    }

	};
	/** @example t-eoSSGA.cpp
	 */

	/** Keep the best individual found so far
	 */
	template <class EOT>
	class BestIndividualStat : public Stat<EOT, EOT>
	{
	public:
	    using Stat<EOT, EOT>::value;

	    BestIndividualStat(std::string _description = "BestIndiv ")
		: Stat<EOT, EOT>( EOT(), _description )
	    {}

	    void operator()(const core::Pop<EOT>& pop) 
	    {
		EOT best = pop.best_element();
		// on the first call, value() is invalid
		if( value().invalid() ) {
		    // thus we cannot compare it to something else
		    value() = best;
		} else {
		    // keep the best individual found so far
		    if( best.fitness() > value().fitness() ) {
			value() = best;
		    }
		}
	    }

	    virtual std::string className(void) const { return "BestIndividualStat"; }
	};



	template <class EOT>
	class DistanceStat : public Stat<EOT, double>
	{
	public:

	    using Stat<EOT, double>::value;

	    DistanceStat(std::string _name = "distance")
		: Stat<EOT, double>(0.0, _name)
	    {}

	    template <class T>
	    double distance(T a, T b)
	    {
		T res = a-b;
		return res < 0? -res : res;
	    }

	    double distance(bool a, bool b)
	    {
		return (a==b)? 0 : 1;
	    }

	    void operator()(const core::Pop<EOT>& _pop)
	    {
		double& v = value();
		v = 0.0;

		for (unsigned i = 0; i < _pop.size(); ++i)
		    {
			for (unsigned j = 0; j < _pop.size(); ++j)
			    {
				for (unsigned k = 0; k < _pop[i].size(); ++k)
				    {
					v += distance(_pop[i][k], _pop[j][k]);
				    }
			    }
		    }

		double sz = _pop.size();
		v /= sz * sz * _pop[0].size();
	    }
	    virtual std::string className(void) const { return "DistanceStat"; }

	};



	template <class EOT>
	class StdevStat : public Stat< EOT, typename EOT::Fitness >
	{
	public :
	    using Stat<EOT, typename EOT::Fitness>::value;
	    typedef typename eoSecondMomentStats<EOT>::SquarePair SquarePair;

	    StdevStat(std::string _description = "Stdev") : Stat<EOT, double>(0.0, _description) {}

	    virtual void operator()(const core::Pop<EOT>& _pop)
	    {
		SquarePair result = std::accumulate(_pop.begin(), _pop.end(), std::make_pair(0.0, 0.0), eoSecondMomentStats<EOT>::sumOfSquares);

		double n = _pop.size();
		value() = sqrt( (result.second - (result.first / n)) / (n - 1.0)); // stdev
	    }
	};


	//! A robust measure of dispersion (also called midspread or middle fifty) that is the difference between the third and the first quartile.
	template<class EOT>
	class InterquartileRangeStat : public Stat< EOT, typename EOT::Fitness >
	{
	public:
	    using Stat<EOT, typename EOT::Fitness>::value;

	    InterquartileRangeStat( typename EOT::Fitness start, std::string description = "IQR" ) : Stat<EOT,typename EOT::Fitness>( start, description ) {}

	    virtual void operator()( const core::Pop<EOT> & _pop )
	    {
		std::vector<typename EOT::Fitness> fitnesses;

		for ( size_t i = 0; i < _pop.size(); ++i )
		    {
			if ( _pop[i].invalid() ) { continue; }
			fitnesses.push_back( _pop[i].fitness() );
		    }

		if (fitnesses.empty())
		    {
			eo::log << eo::logging << "InterquartileRangeStat: Valid population empty\n";
			return;
		    }

		unsigned int quartile = fitnesses.size()/4;
		std::nth_element( fitnesses.begin(), fitnesses.begin()+quartile*1, fitnesses.end() );
		typename EOT::Fitness Q1 = fitnesses[quartile];

		std::nth_element( fitnesses.begin(), fitnesses.begin()+quartile*3, fitnesses.end() );
		typename EOT::Fitness Q3 = fitnesses[quartile*3];

		value() = Q3 - Q1;
	    }

	    virtual std::string className(void) const { return "InterquartileRangeStat"; }
	};
	/** @example t-eoIQRStat.cpp
	 */

	/** Compute the average size of indivudals over the population
	 *
	 * Obviously, will work only on representations that implement the (standard) "size()" method,
	 * like any STL container.
	 */
	template<class EOT>
	class AverageSizeStat : public Stat< EOT, double>
	{
	public:

	    using Stat<EOT, double>::value;

	    AverageSizeStat( std::string description = "Av.Size" ) :
		Stat<EOT,double>( 0.0, description ) {} // 0 by default

	    virtual void operator()( const core::Pop<EOT> & pop )
	    {
		if (pop.empty())
		    {
			eo::log << eo::logging << "AverageSizeStat: Population empty\n";
			return;
		    }

		size_t pop_size = pop.size();

		std::vector<size_t> sizes;
		sizes.reserve(pop_size);

		for( unsigned int i=0, s = pop_size; i<s; ++i ) {
		    sizes.push_back( pop[i].size() );
		}

		size_t sum = std::accumulate( sizes.begin(), sizes.end(), 0 );

		value() = static_cast<double>(sum) / static_cast<double>(pop_size);
	    }

	    virtual std::string className(void) const { return "AverageSizeStat"; }
	};

	template <class EOT>
	class AverageDeltaFitnessStat : public Stat<EOT, typename EOT::Fitness>
	{
	public :
	    using Stat<EOT, typename EOT::Fitness>::value;

	    typedef typename EOT::Fitness Fitness;

	    AverageDeltaFitnessStat(std::string _description = "Average Delta Fitness")
		: Stat<EOT, Fitness>(Fitness(), _description) {}

	    static Fitness sumFitness(double _sum, const EOT& _eot){
		double last = _eot.getLastFitness() > 0 ? _eot.getLastFitness() : 1;
		_sum += _eot.fitness() / last;
		return _sum;
	    }

	    AverageDeltaFitnessStat(double _value, std::string _desc) : Stat<EOT, double>(_value, _desc) {}

	    virtual void operator()(const core::Pop<EOT>& _pop)
	    {
		if (_pop.empty())
		    {
			eo::log << eo::logging << "AverageDeltaFitnessStat: Population empty\n";
			return;
		    }

		doit(_pop, Fitness()); // specializations for scalar and std::vector
	    }

	    virtual std::string className(void) const { return "AverageDeltaFitnessStat"; }

	private :
	    // Default behavior
	    template <class T>
	    void doit(const core::Pop<EOT>& _pop, T)
	    {
		Fitness v = std::accumulate(_pop.begin(), _pop.end(), Fitness(0.0), AverageDeltaFitnessStat::sumFitness);
		value() = v / _pop.size();
	    }

	};

    } // !utils
} // !dim

/** @} */
#endif // !_UTILS_STAT_H_
