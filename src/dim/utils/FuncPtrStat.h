// -*- mode: c++; c-indent-level: 4; c++-member-init-indent: 8; comment-column: 35; -*-

//-----------------------------------------------------------------------------
// eoPop.h
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
*/

#ifndef _UTILS_FUNCPTRSTAT_H_
#define _UTILS_FUNCPTRSTAT_H_

#include <eoFunctorStore.h>

#include "Stat.h"

namespace dim
{
    namespace utils
    {

	/** Wrapper to turn any stand-alone function and into an Stat.
	 *
	 * The function should take an core::Pop as argument.
	 *
	 * @ingroup Stats
	 */
	template <class EOT, class T>
	class FuncPtrStat : public Stat<EOT, T>
	{
	public :
	    typedef T (*func_t)(const core::Pop<EOT>&);


	    FuncPtrStat(func_t f, std::string _description = "func_ptr")
		: Stat<EOT, T>(T(), _description), func(f)
	    {}

	    using Stat<EOT, T>::value;

	    void operator()(const core::Pop<EOT>& pop) {
		value() = func(pop);
	    }

	private:
	    func_t func;
	};

	/**
	 * @ingroup Stats
	 */
	template <class EOT, class T>
	FuncPtrStat<EOT, T>& makeFuncPtrStat( T (*func)(const core::Pop<EOT>&), eoFunctorStore& store, std::string description = "func") {
	    return store.storeFunctor(
				      new FuncPtrStat<EOT, T>( func, description)
				      );
	}

	/** Wrapper to turn any stand-alone function and into an Stat.
	 *
	 * The function should take an core::Pop as argument.
	 *
	 * @ingroup Stats
	 */
	template <class EOT, class T>
	class eoFunctorStat : public Stat<EOT, T>
	{
	public :
	    eoFunctorStat(eoUF< const core::Pop<EOT>&, T >& f, std::string _description = "functor")
		: Stat<EOT, T>(T(), _description), func(f)
	    {}

	    using Stat<EOT, T>::value;

	    void operator()(const core::Pop<EOT>& pop) {
		value() = func(pop);
	    }

	private:
	    eoUF< const core::Pop<EOT>&, T >& func;
	};

	/**
	 * @ingroup Stats
	 */
	template <class EOT, class T>
	eoFunctorStat<EOT, T>& makeFunctorStat( eoUF< const core::Pop<EOT>&, T >& func, eoFunctorStore& store, std::string description = "func") {
	    return store.storeFunctor(
				      new eoFunctorStat<EOT, T>( func, description)
				      );
	}

    } // !utils
} // !dim

#endif // !_UTILS_FUNCPTRSTAT_H_
