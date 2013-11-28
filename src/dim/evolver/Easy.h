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

#ifndef _EVOLVER_EASY_H_
#define _EVOLVER_EASY_H_

#include "Base.h"
#include <dim/utils/Measure.h>

#include <boost/utility/identity_type.hpp>

#undef MOVE
#if __cplusplus > 199711L
# define MOVE(var) std::move(var)
#else
# define MOVE(var) var
#endif

namespace dim
{
    namespace evolver
    {
#if __cplusplus > 199711L
	namespace std_or_boost = std;
#else
	namespace std_or_boost = boost;
#endif

	template <typename EOT>
	class Easy : public Base<EOT>
	{
	public:
	    Easy(eoEvalFunc<EOT>& eval, eoMonOp<EOT>& op, bool invalidate = true, size_t nbmove = 1) : _eval(eval), _op(op), _invalidate(invalidate), _nbmove(nbmove) {}

	    virtual void firstCall(core::Pop<EOT>& /*pop*/, core::IslandData<EOT>& data)
	    {
		std::ostringstream ss;

#ifdef MEASURE
		ss.str(""); ss << data.monitorPrefix << ".evolve_total.time." << this->rank();
		_measureFiles["evolve_total"] = new std::ofstream(ss.str().c_str());

		ss.str(""); ss << data.monitorPrefix << ".evolve_op.time." << this->rank();
		_measureFiles["evolve_op"] = new std::ofstream(ss.str().c_str());

		ss.str(""); ss << data.monitorPrefix << ".evolve_eval.time." << this->rank();
		_measureFiles["evolve_eval"] = new std::ofstream(ss.str().c_str());
#endif // !MEASURE
	    }

	    void operator()(core::Pop<EOT>& pop, core::IslandData<EOT>& /*data*/)
	    {
		DO_MEASURE(

			   for (size_t i = 0; i < pop.size(); ++i)
			       {
				   EOT& ind = pop[i];

				   for (size_t k = 0; k < _nbmove; ++k)
				       {
					   EOT candidate = ind;

					   DO_MEASURE(
						      _op( candidate );
						      , _measureFiles, "evolve_op" );

					   if (_invalidate)
					       {
						   candidate.invalidate();
					       }

					   DO_MEASURE(
						      _eval( candidate );
						      , _measureFiles, "evolve_eval" );

					   if ( candidate.fitness() > ind.fitness() )
					       {
						   ind = MOVE(candidate);
					       }
				       }
			       }

			   , _measureFiles, "evolve_total" );
	    }

	private:
	    eoEvalFunc<EOT>& _eval;
	    eoMonOp<EOT>& _op;
	    bool _invalidate;
	    size_t _nbmove;

#ifdef MEASURE
	    std::map<std::string, std::ofstream*> _measureFiles;
#endif // !MEASURE
	};
    } // !evolver
} // !dim

#endif /* _EVOLVER_EASY_H_ */
