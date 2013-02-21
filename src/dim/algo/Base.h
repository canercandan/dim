// -*- mode: c++; c-indent-level: 4; c++-member-init-indent: 8; comment-column: 35; -*-

//-----------------------------------------------------------------------------
// Algo.h
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

#ifndef _ALGO_BASE_H_
#define _ALGO_BASE_H_

#include <dim/core/IslandOperator.h>
#include <dim/core/Thread.h>

namespace dim
{
    namespace algo
    {
	/**
	   This is the base class for population-transforming algorithms. There
	   is only one operator defined, which takes a population and does stuff to
	   it. It needn't be a complete algorithm, can be also a step of an
	   algorithm. This class just gives a common interface to linear
	   population-transforming algorithms.

	   @ingroup Algorithms
	*/

	template< class EOT >
	class Base : public core::IslandOperator<EOT>
	{
	public:
	    virtual void firstCall(core::Pop<EOT>&, core::IslandData<EOT>&) {}
	    virtual void lastCall(core::Pop<EOT>&, core::IslandData<EOT>&) {}
	};
    } // !algo
} // !dim

#endif // !_ALGO_BASE_H_
