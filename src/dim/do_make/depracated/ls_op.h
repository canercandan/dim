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

#ifndef _DO_MAKE_LS_OP_H_
#define _DO_MAKE_LS_OP_H_

#include <eo>

//#include <ga/make_ga.h>
#include <ga/eoBitOp.h>

#include <dim/algo/EasyLS.h>

namespace dim
{
    namespace do_make
	{

	    /* Create local search operators and there parameters */
	    template <class EOT>
	    eoGenOp<EOT>& ls_op(eoParser& parser, eoState& state, eoInit<EOT>& init)
	    {
		eoValueParam<unsigned>& kflipParam = parser.createParam(unsigned(0), "kflip", "K-Flip, 0: disabled", 0, "Problem");
		eoValueParam<bool>& bitflipParam = parser.createParam(false, "bitflip", "Bit-Flip", 0, "Problem");
		eoValueParam<bool>& uniformParam = parser.createParam(true, "uniform", "Uniform", 0, "Problem");
		eoValueParam<unsigned>& uniformNbOpsParam = parser.createParam(unsigned(5), "uniform-nb-ops", "Uniform: number of operators (n + bitflip)", 0, "Problem");

		eoValueParam<unsigned>& nbMoveLSParam = parser.createParam(unsigned(100), "nbMoveLS", "Numero of moves by individual for Local Search", 0, "Problem");

		eoSequentialOp<EOT>* ptSeqOp = new algo::EasyLS<EOT>(nbMoveLSParam.value());
		state.storeFunctor(ptSeqOp);

		eoMonOp<EOT>* ptMon = NULL;

		if ( kflipParam.value() )
		    {
			std::cout << "kflip(" << kflipParam.value() << ")" << std::endl;
			ptMon = new eoDetBitFlip<EOT>(kflipParam.value());
			state.storeFunctor(ptMon);
			ptSeqOp->add(*ptMon, 1);
		    }
		else if ( bitflipParam.value() )
		    {
			std::cout << "bitflip" << std::endl;
			ptMon = new eoBitMutation<EOT>(1, true);
			state.storeFunctor(ptMon);
			ptSeqOp->add(*ptMon, 1);
		    }
		else if ( uniformParam.value() )
		    {
			std::cout << "uniform" << std::endl;
			eoOpContainer<EOT>* ptPropOp = new eoProportionalOp<EOT>;
			state.storeFunctor(ptPropOp);

			ptMon = new eoBitMutation<EOT>(1, true);
			state.storeFunctor(ptMon);
			double rate = 1/(double)uniformNbOpsParam.value();
			ptPropOp->add(*ptMon, rate);

			for (unsigned i = 1; i <= uniformNbOpsParam.value(); ++i)
			    {
				ptMon = new eoDetBitFlip<EOT>( i );
				state.storeFunctor(ptMon);
				ptPropOp->add(*ptMon, rate);
			    }

			ptSeqOp->add(*ptPropOp, 1);
		    }
		else
		    {
			// exception to throw
		    }

		return *ptSeqOp;
	    }

    } // !do
} // !dim

#endif // !_DO_MAKE_LS_OP_H_
