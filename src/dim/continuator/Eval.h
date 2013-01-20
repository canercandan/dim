// -*- mode: c++; c-indent-level: 4; c++-member-init-indent: 8; comment-column: 35; -*-

//-----------------------------------------------------------------------------
// Eval.h
// (c) GeNeura Team, 1999, Marc Schoenauer 2001
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
*/
//-----------------------------------------------------------------------------

#ifndef _CONTINUATOR_EVAL_H_
#define _CONTINUATOR_EVAL_H_

#include <eoEvalFuncCounter.h>

#include "Base.h"

namespace dim
{
    namespace continuator
    {

	/**
	 * Continues until a number of evaluations has been made
	 *
	 * @ingroup Continuators
	 */
	template< class EOT>
	class Eval: public Base<EOT>
	{
	public:
	    /// Ctor
	    Eval( eoEvalFuncCounter<EOT> & _eval, unsigned long _totalEval)
		: eval(_eval), repTotalEvaluations( _totalEval ) {};

	    /** Returns false when a certain number of evaluations has been done
	     */
	    virtual bool operator() ( const core::Pop<EOT>& _vEO ) {
		(void)_vEO;
		if (eval.value() >= repTotalEvaluations)
		    {
			eo::log << eo::progress << "STOP in Eval: Reached maximum number of evaluations [" << repTotalEvaluations << "]" << std::endl;
			return false;
		    }
		return true;
	    }

	    /** Returns the number of generations to reach*/
	    virtual unsigned long totalEvaluations( )
	    {
		return repTotalEvaluations;
	    };

	    virtual std::string className(void) const { return "Eval"; }
	private:
	    eoEvalFuncCounter<EOT> & eval;
	    unsigned long repTotalEvaluations;
	};

    } // !continuator
} // !dim

#endif // !_CONTINUATOR_EVAL_H_
