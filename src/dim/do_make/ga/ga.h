// -*- mode: c++; c-indent-level: 4; c++-member-init-indent: 8; comment-column: 35; -*-

//-----------------------------------------------------------------------------
// ga.h
// (c) Maarten Keijzer, Marc Schoenauer and GeNeura Team, 2001
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

/** This file contains all ***INSTANCIATED*** declarations of all components
 * of the library for ***BISTRING*** evolution inside EO.
 * It should be included in the file that calls any of the corresponding fns
 *
 * The corresponding ***INSTANCIATED*** definitions are contained in ga.cpp
 * while the TEMPLATIZED code is define in the different makeXXX.h files
 *
 * Unlike most EO .h files, it does not (and should not) contain any code,
 * just declarations
 */

#ifndef _DO_MAKE_GA_H_
#define _DO_MAKE_GA_H_

#include <eoScalarFitness.h>
#include <utils/eoParser.h>
#include <eoEvalFuncCounter.h>
#include <utils/eoDistance.h>

#include <dim/algo/Base.h>
#include <dim/core/Pop.h>
#include <dim/core/Bit.h>
#include <dim/utils/CheckPoint.h>
#include <dim/variation/GenOp.h>

#include "checkpoint.h"

namespace dim
{
    namespace do_make
    {

	//Representation dependent - rewrite everything anew for each representation
	//////////////////////////

	/** @addtogroup Builders
	 * @{
	 */

	// the genotypes
	eoInit<core::Bit<double> > & genotype(eoParser& _parser, eoState& _state, core::Bit<double> _eo, float _bias=0.5);
	eoInit<core::Bit<eoMinimizingFitness> > & genotype(eoParser& _parser, eoState& _state, core::Bit<eoMinimizingFitness> _eo, float _bias=0.5);

	// the operators
	variation::GenOp<core::Bit<double> >&  op(eoParser& _parser, eoState& _state, eoInit<core::Bit<double> >& _init);
	variation::GenOp<core::Bit<eoMinimizingFitness> >&  op(eoParser& _parser, eoState& _state, eoInit<core::Bit<eoMinimizingFitness> >& _init);

	//Representation INdependent
	////////////////////////////
	// if you use your own representation, simply change the types of templates

	// init pop
	core::Pop<core::Bit<double> >&  pop(eoParser& _parser, eoState& _state, eoInit<core::Bit<double> >&);
	core::Pop<core::Bit<eoMinimizingFitness> >&  pop(eoParser& _parser, eoState& _state, eoInit<core::Bit<eoMinimizingFitness> >&);

	// the continue's
	continuator::Base<core::Bit<double> >& continuator(eoParser& _parser, eoState& _state, eoEvalFuncCounter<core::Bit<double> > & _eval);
	continuator::Base<core::Bit<eoMinimizingFitness> >& continuator(eoParser& _parser, eoState& _state, eoEvalFuncCounter<core::Bit<eoMinimizingFitness> > & _eval);

	// the checkpoint
	utils::CheckPoint<core::Bit<double> >& checkpoint(eoParser& _parser, eoState& _state, eoEvalFuncCounter<core::Bit<double> >& _eval, continuator::Base<core::Bit<double> >& _continue);
	utils::CheckPoint<core::Bit<eoMinimizingFitness> >& checkpoint(eoParser& _parser, eoState& _state, eoEvalFuncCounter<core::Bit<eoMinimizingFitness> >& _eval, continuator::Base<core::Bit<eoMinimizingFitness> >& _continue);


	// the algo
	algo::Base<core::Bit<double> >&  algo_scalar(eoParser& _parser, eoState& _state, eoEvalFunc<core::Bit<double> >& _eval, continuator::Base<core::Bit<double> >& _ccontinue, variation::GenOp<core::Bit<double> >& _op, eoDistance<core::Bit<double> >* _dist = NULL);

	algo::Base<core::Bit<eoMinimizingFitness> >&  algo_scalar(eoParser& _parser, eoState& _state, eoEvalFunc<core::Bit<eoMinimizingFitness> >& _eval, continuator::Base<core::Bit<eoMinimizingFitness> >& _ccontinue, variation::GenOp<core::Bit<eoMinimizingFitness> >& _op, eoDistance<core::Bit<eoMinimizingFitness> >* _dist = NULL);

	// run
	void run_ea(algo::Base<core::Bit<double> >& _ga, core::Pop<core::Bit<double> >& _pop);
	void run_ea(algo::Base<core::Bit<eoMinimizingFitness> >& _ga, core::Pop<core::Bit<eoMinimizingFitness> >& _pop);

	// end of parameter input (+ .status + help)
	// that one is not templatized
	// Because of that, the source is in src/utils dir
	void help(eoParser & _parser);

	/** @} */

    } // !do_make
} // !dim

#endif // !_DO_MAKE_GA_H_
