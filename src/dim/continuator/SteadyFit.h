// -*- mode: c++; c-indent-level: 4; c++-member-init-indent: 8; comment-column: 35; -*-

//-----------------------------------------------------------------------------
// SteadyFit.h
// (c) GeNeura Team, 1999, Marc Schoenauer, 2000
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

  Contact: todos@geneura.ugr.es, http://geneura.ugr.es
*/
//-----------------------------------------------------------------------------

#ifndef _CONTINUATOR_STEADYFIT_H_
#define _CONTINUATOR_STEADYFIT_H_

#include <utils/eoLogger.h>

#include "Base.h"

namespace dim
{
    namespace continuator
    {

	/**
	   A continuator:  does a minimum number of generations, then
	   stops whenever a given number of generations takes place without improvement

	   @ingroup Continuators
	*/
	template< class EOT>
	class SteadyFit: public Count<EOT>
	{
	public:
	    typedef typename EOT::Fitness Fitness;

	    using Count<EOT>::thisGenerationPlaceholder;
	    using Count<EOT>::thisGeneration;

	    /// Ctor for setting a
	    SteadyFit( unsigned long _minGens, unsigned long _steadyGens)
		: Count<EOT>( ), repMinGenerations( _minGens ), repSteadyGenerations( _steadyGens),
		  steadyState(false)
	    {};

	    /// Ctor for enabling the save/load the no. of generations counted
	    SteadyFit( unsigned long _minGens, unsigned long _steadyGen,
			       unsigned long& _currentGen)
		: Count<EOT>( _currentGen ), repMinGenerations( _minGens ), repSteadyGenerations( _steadyGen),
		  steadyState(_currentGen>_minGens)
	    {};

	    /** Returns false when a certain number of generations is
	     * reached withtout improvement */
	    virtual bool operator() ( const core::Pop<EOT>& _vEO ) {
		thisGeneration++;

		Fitness bestCurrentFitness = _vEO.nth_element_fitness(0);

		if (steadyState) {     // already after MinGenenerations
		    if (bestCurrentFitness > bestSoFar) {
			bestSoFar = bestCurrentFitness;
			lastImprovement = thisGeneration;
		    } else {
			if (thisGeneration - lastImprovement > repSteadyGenerations) {
			    eo::log << eo::progress << "STOP in SteadyFit: Done " << repSteadyGenerations
				    << " generations without improvement\n";
			    return false;
			}
		    }
		} else {               // not yet in steady state
		    if (thisGeneration > repMinGenerations) { // go to steady state
			steadyState = true;
			bestSoFar = bestCurrentFitness;
			lastImprovement = thisGeneration;
			eo::log << eo::progress << "SteadyFit: Done the minimum number of generations\n";
		    }
		}
		return true;
	    }

	    /** Sets the parameters (minimum nb of gen. + steady nb of gen.)
		and sets the current generation to 0 (the begin)

		@todo replace thi by an init method ?
	    */
	    virtual void totalGenerations( unsigned long _mg, unsigned long _sg ) {
		repMinGenerations = _mg;
		repSteadyGenerations = _sg;
		reset();
	    };

	    /// Resets the state after it's been reached
	    virtual void reset () {
		steadyState=false;
		Count<EOT>::reset();
	    }

	    /** accessors*/
	    virtual unsigned long minGenerations( )
	    {  return repMinGenerations;  };
	    virtual unsigned long steadyGenerations( )
	    {  return repSteadyGenerations;       };

	    virtual std::string className(void) const { return "SteadyFit"; }
	private:
	    unsigned long repMinGenerations;
	    unsigned long  repSteadyGenerations;
	    bool steadyState;
	    unsigned int lastImprovement;
	    Fitness bestSoFar;
	};

    } // !continuator
} // !dim

#endif // !_CONTINUATOR_STEADYFIT_H_
