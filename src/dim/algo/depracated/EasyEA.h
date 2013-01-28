// -*- mode: c++; c-indent-level: 4; c++-member-init-indent: 8; comment-column: 35; -*-

//-----------------------------------------------------------------------------
// EasyEA.h
// (c) GeNeura Team, 1998
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

#ifndef _ALGO_EASYEA_H_
#define _ALGO_EASYEA_H_

//-----------------------------------------------------------------------------

#include <apply.h>
#include <eoPopEvalFunc.h>
#include <eoSelect.h>
#include <eoTransform.h>
#include <eoBreed.h>
#include <eoMergeReduce.h>
#include <eoReplacement.h>

#include <dim/core/Continue.h>

#include "Base.h"

namespace dim
{
    namespace algo
    {

	template <class EOT> class eoIslandsEasyEA ;

	template <class EOT> class eoDistEvalEasyEA ;

	/** An easy-to-use evolutionary algorithm; you can use any chromosome,
	    and any selection transformation, merging and evaluation
	    algorithms; you can even change in runtime parameters of those
	    sub-algorithms

	    Change (MS, July 3. 2001):
	    Replaced the eoEvalFunc by an core::PopEvalFunc: this immediately
	    allows many useful constructs, such as co-evolution (e.g. game players),
	    parisian approach (the solution to the problem is the whole population)
	    or simple distribution of evaluations on a cluster.
	    In case an eoEvalFunc is passed, it is embedded on an core::PopLoopEval
	    This makes things a little uglier (required an additional "dummy" member

	    Note: it looks ugly only because we wanted to authorize many different
	    constructors. Please only look at the operator() and there shall be light

	    @ingroup Algorithms
	*/
	template<class EOT> class EasyEA: public Base<EOT>
	{
	public:

	    /** Ctor taking a breed and merge */
	    EasyEA(
		   core::Continue<EOT>& _continuator,
		   eoEvalFunc<EOT>& _eval,
		   eoBreed<EOT>& _breed,
		   eoReplacement<EOT>& _replace
		   ) : continuator(_continuator),
		       eval (_eval),
		       loopEval(_eval),
		       popEval(loopEval),
		       selectTransform(dummySelect, dummyTransform),
		       breed(_breed),
		       mergeReduce(dummyMerge, dummyReduce),
		       replace(_replace),
		       isFirstCall(true)
	    {}

	    /** Ctor taking a breed and merge, an overload of ctor to define an offspring size */
	    EasyEA(
		   core::Continue<EOT>& _continuator,
		   eoEvalFunc<EOT>& _eval,
		   eoBreed<EOT>& _breed,
		   eoReplacement<EOT>& _replace,
		   unsigned _offspringSize
		   ) : continuator(_continuator),
		       eval (_eval),
		       loopEval(_eval),
		       popEval(loopEval),
		       selectTransform(dummySelect, dummyTransform),
		       breed(_breed),
		       mergeReduce(dummyMerge, dummyReduce),
		       replace(_replace),
		       isFirstCall(true)
	    {
		offspring.reserve(_offspringSize); // This line avoids an incremental resize of offsprings.
	    }

	    /**
	     * @brief Ctor allowing to specify which pop eval function we're going to use.
	     *
	     * Ctor taking a breed and merge, an overload of ctor to define an offspring size, and
	     * the pop eval function used. This allows to precise if we would like to use the
	     * parallel evaluation, for instance.
	     */
	    EasyEA(
		   core::Continue<EOT>& _continuator,
		   eoEvalFunc<EOT>& _eval,
		   core::PopEvalFunc<EOT>& _pop_eval,
		   eoBreed<EOT>& _breed,
		   eoReplacement<EOT>& _replace,
		   unsigned _offspringSize
		   ) : continuator(_continuator),
		       eval (_eval),
		       loopEval(_eval),
		       popEval(_pop_eval),
		       selectTransform(dummySelect, dummyTransform),
		       breed(_breed),
		       mergeReduce(dummyMerge, dummyReduce),
		       replace(_replace),
		       isFirstCall(true)
	    {
		offspring.reserve(_offspringSize); // This line avoids an incremental resize of offsprings.
	    }

	    /*
	      EasyEA(core::Continue <EOT> & _continuator,
	      core::PopEvalFunc <EOT> & _pop_eval,
	      eoBreed <EOT> & _breed,
	      eoReplacement <EOT> & _replace
	      ) :
	      continuator (_continuator),
	      eval (dummyEval),
	      loopEval(dummyEval),
	      popEval (_pop_eval),
	      selectTransform (dummySelect, dummyTransform),
	      breed (_breed),
	      mergeReduce (dummyMerge, dummyReduce),
	      replace (_replace),
	      isFirstCall(true)
	      {

	      }
	    */

	    /** NEW Ctor taking a breed and merge and an core::PopEval */
	    EasyEA(
		   core::Continue<EOT>& _continuator,
		   core::PopEvalFunc<EOT>& _eval,
		   eoBreed<EOT>& _breed,
		   eoReplacement<EOT>& _replace
		   ) : continuator(_continuator),
		       eval (dummyEval),
		       loopEval(dummyEval),
		       popEval(_eval),
		       selectTransform(dummySelect, dummyTransform),
		       breed(_breed),
		       mergeReduce(dummyMerge, dummyReduce),
		       replace(_replace),
		       isFirstCall(true)
	    {}


	    /// Ctor eoSelect, eoTransform, eoReplacement and an core::PopEval
	    EasyEA(
		   core::Continue<EOT>& _continuator,
		   core::PopEvalFunc<EOT>& _eval,
		   eoSelect<EOT>& _select,
		   eoTransform<EOT>& _transform,
		   eoReplacement<EOT>& _replace
		   ) : continuator(_continuator),
		       eval (dummyEval),
		       loopEval(dummyEval),
		       popEval(_eval),
		       selectTransform(_select, _transform),
		       breed(selectTransform),
		       mergeReduce(dummyMerge, dummyReduce),
		       replace(_replace),
		       isFirstCall(true)
	    {}

	    /// Ctor eoBreed, eoMerge and eoReduce.
	    EasyEA(
		   core::Continue<EOT>& _continuator,
		   eoEvalFunc<EOT>& _eval,
		   eoBreed<EOT>& _breed,
		   eoMerge<EOT>& _merge,
		   eoReduce<EOT>& _reduce
		   ) : continuator(_continuator),
		       eval (_eval),
		       loopEval(_eval),
		       popEval(loopEval),
		       selectTransform(dummySelect, dummyTransform),
		       breed(_breed),
		       mergeReduce(_merge, _reduce),
		       replace(mergeReduce),
		       isFirstCall(true)
	    {}

	    /// Ctor eoSelect, eoTransform, and eoReplacement
	    EasyEA(
		   core::Continue<EOT>& _continuator,
		   eoEvalFunc<EOT>& _eval,
		   eoSelect<EOT>& _select,
		   eoTransform<EOT>& _transform,
		   eoReplacement<EOT>& _replace
		   ) : continuator(_continuator),
		       eval (_eval),
		       loopEval(_eval),
		       popEval(loopEval),
		       selectTransform(_select, _transform),
		       breed(selectTransform),
		       mergeReduce(dummyMerge, dummyReduce),
		       replace(_replace),
		       isFirstCall(true)
	    {}

	    /// Ctor eoSelect, eoTransform, eoMerge and eoReduce.
	    EasyEA(
		   core::Continue<EOT>& _continuator,
		   eoEvalFunc<EOT>& _eval,
		   eoSelect<EOT>& _select,
		   eoTransform<EOT>& _transform,
		   eoMerge<EOT>&     _merge,
		   eoReduce<EOT>&    _reduce
		   ) : continuator(_continuator),
		       eval (_eval),
		       loopEval(_eval),
		       popEval(loopEval),
		       selectTransform(_select, _transform),
		       breed(selectTransform),
		       mergeReduce(_merge, _reduce),
		       replace(mergeReduce),
		       isFirstCall(true)
	    {}




	    /// Apply a few generation of evolution to the population.
	    virtual void operator()(core::Pop<EOT>& _pop)
	    {

		if (isFirstCall)
		    {
			size_t total_capacity = _pop.capacity() + offspring.capacity();
			_pop.reserve(total_capacity);
			offspring.reserve(total_capacity);
			isFirstCall = false;
		    }

		core::Pop<EOT> empty_pop;

		do
		    {
			try
			    {
				unsigned pSize = _pop.size();

				offspring.clear(); // new offspring

				breed(_pop, offspring);

				popEval(_pop, offspring); // eval of parents + offspring if necessary

				replace(_pop, offspring); // after replace, the new pop. is in _pop

				if (pSize > _pop.size())
				    throw std::runtime_error("Population shrinking!");
				else if (pSize < _pop.size())
				    throw std::runtime_error("Population growing!");
			    }
			catch (std::exception& e)
			    {
				std::string s = e.what();
				s.append( " in EasyEA");
				throw std::runtime_error( s );
			    }
		    }
		while ( continuator( _pop ) );
	    }

	protected :

	    // If selectTransform needs not be used, dummySelect and dummyTransform are used
	    // to instantiate it.
	    class eoDummySelect : public eoSelect<EOT>
	    {
	    public :
		void operator()(const core::Pop<EOT>&, core::Pop<EOT>&)
		{}
	    }
		dummySelect;

	    class eoDummyTransform : public eoTransform<EOT>
	    {
	    public :
		void operator()(core::Pop<EOT>&)
		{}
	    }
		dummyTransform;

	    class eoDummyEval : public eoEvalFunc<EOT>
	    {
	    public:
		void operator()(EOT &)
		{}
	    }
		dummyEval;

	    core::Continue<EOT>&          continuator;

	    eoEvalFunc <EOT> &        eval ;
	    core::PopLoopEval<EOT>        loopEval;

	    core::PopEvalFunc<EOT>&       popEval;

	    eoSelectTransform<EOT>    selectTransform;
	    eoBreed<EOT>&             breed;

	    // If mergeReduce needs not be used, dummyMerge and dummyReduce are used
	    // to instantiate it.
	    eoNoElitism<EOT>          dummyMerge;
	    eoTruncate<EOT>           dummyReduce;

	    eoMergeReduce<EOT>        mergeReduce;
	    eoReplacement<EOT>&       replace;

	    core::Pop<EOT>                offspring;

	    bool		      isFirstCall;

	    // Friend classes
	    friend class eoIslandsEasyEA <EOT> ;
	    friend class eoDistEvalEasyEA <EOT> ;
	};
	/**
	   @example t-EasyEA.cpp
	   Example of a test program building an EA algorithm.
	*/

    } // !algo
} // !dim

#endif // !_ALGO_EASYEA_H_
