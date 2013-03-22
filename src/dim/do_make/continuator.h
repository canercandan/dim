// -*- mode: c++; c-indent-level: 4; c++-member-init-indent: 8; comment-column: 35; -*-

//-----------------------------------------------------------------------------
// continuator_dim.h
// (c) Maarten Keijzer, Marc Schoenauer and GeNeura Team, 2000
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
  Marc.Schoenauer@polytechnique.fr
  mkeijzer@dhi.dk
*/
//-----------------------------------------------------------------------------

#ifndef _DO_MAKE_CONTINUATOR_H_
#define _DO_MAKE_CONTINUATOR_H_

#include <eo>

#include <dim/utils/utils>
#include <dim/continuator/continuator>

/*
  Contains the templatized version of parser-based choice of stopping criterion
  It can then be instantiated, and compiled on its own for a given EOType
  (see e.g. in dir ga, ga.cpp)
*/

namespace dim
{
    namespace do_make
    {

	/**
	 *
	 * @ingroup Builders
	 */
	template <class EOT>
	continuator::Combined<EOT> * combinedContinuator(continuator::Combined<EOT> *_combined, continuator::Base<EOT> *_cont)
	{
	    if (_combined)                   // already exists
		_combined->add(*_cont);
	    else
		_combined = new continuator::Combined<EOT>(*_cont);
	    return _combined;
	}

	/**
	 *
	 * @ingroup Builders
	 */
	template <class EOT>
	continuator::Base<EOT> & continuator(eoParser& _parser, eoState& _state, eoEvalFuncCounter<EOT> & /*_eval*/)
	{
	    continuator::Combined<EOT>* continuator = NULL;

	    long maxTime = _parser.getORcreateParam(long(0), "maxTime", "Maximum time (in seconds) used by the program () = none)",'X',"Stopping criterion").value();
	    if (maxTime) // positive: -> define and store
		{
		    continuator::Time<EOT> *timeCont = new continuator::Time<EOT>(maxTime);
		    _state.storeFunctor(timeCont);
		    // and "add" to combined
		    continuator = combinedContinuator<EOT>(continuator, timeCont);
		}

	    unsigned maxGen = _parser.getORcreateParam(unsigned(10000), "maxGen", "Maximum number of generations () = none)",'G',"Stopping criterion").value();
	    if (maxGen) // positive: -> define and store
		{
		    continuator::Gen<EOT> *genCont = new continuator::Gen<EOT>(maxGen);
		    _state.storeFunctor(genCont);
		    // and "add" to combined
		    continuator = combinedContinuator<EOT>(continuator, genCont);
		}

#if __cplusplus > 199711L
	    continuator::Fit<EOT> *fitCont = nullptr;
#else
	    continuator::Fit<EOT> *fitCont = NULL;
#endif
	    double targetFitness = _parser.getORcreateParam(double(1000), "targetFitness", "Stop when fitness reaches",'T', "Stopping criterion").value();
	    if (targetFitness)
		{
		    fitCont = new continuator::Fit<EOT>(targetFitness);
		    // store
		    _state.storeFunctor(fitCont);
		    // add to combinedContinue
		    continuator = combinedContinuator<EOT>(continuator, fitCont);
		}

	    if (!continuator)
		{
		    throw std::runtime_error("You MUST provide a stopping criterion");
		}
	    // OK, it's there: store in the eoState
	    _state.storeFunctor(continuator);

	    return *continuator;
	}

    } // !do_make
} // !dim

#endif // !_DO_MAKE_CONTINUATOR_H_
