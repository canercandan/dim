// -*- mode: c++; c-indent-level: 4; c++-member-init-indent: 8; comment-column: 35; -*-

//-----------------------------------------------------------------------------
// Fit.h
// (c) Maarten Keijzer, GeNeura Team, 1999, 2000
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

#ifndef _CONTINUATOR_FIT_H_
#define _CONTINUATOR_FIT_H_

#include <utils/eoLogger.h>

#include "Base.h"

namespace dim
{
    namespace continuator
    {

	/**
	   Continues until the optimum fitness level is reached.

	   All types which derive from eoScalarFitness is able to compare well via the operator override ( <, >, <=, ...)

	   @ingroup Continuators
	*/
	template< class EOT>
	class Fit: public Base<EOT> {
	public:

	    /// Define Fitness
	    typedef typename EOT::Fitness FitnessType;

	    /// Ctor
	    Fit( const FitnessType _optimum)
		: Base<EOT> (), optimum( _optimum ) {};

	    /** Returns false when a fitness criterium is reached. Assumes pop is not sorted! */
	    virtual bool operator() ( const core::Pop<EOT>& _pop )
	    {
		if (_pop.empty())
		    {
			eo::log << eo::logging << "Fit: Population empty\n";
			return true;
		    }

		FitnessType bestCurrentFitness = _pop.best_element().fitness();
		if (bestCurrentFitness >= optimum)
		    {
			eo::log << eo::logging << "STOP in Fit: Best fitness has reached " <<
			    bestCurrentFitness << "\n";
			return false;
		    }
		return true;
	    }

	    virtual std::string className(void) const { return "Fit"; }

	private:
	    FitnessType optimum;
	};

    } // !continuator
} // !dim

#endif // !_CONTINUATOR_FIT_H_
