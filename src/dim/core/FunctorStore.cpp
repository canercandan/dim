// -*- mode: c++; c-indent-level: 4; c++-member-init-indent: 8; comment-column: 35; -*-

//-----------------------------------------------------------------------------
// FunctorStore.h
// (c) Maarten Keijzer 2000, GeNeura Team, 2000
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
  mak@dhi.dk
*/
//-----------------------------------------------------------------------------

#ifdef _MSC_VER
// to avoid long name warnings
#pragma warning(disable:4786)
#endif

#include <vector>

#include "FunctorStore.h"
#include "FunctorBase.h"

namespace dim
{
    namespace core
    {

	/// clears the memory
	FunctorStore::~FunctorStore()
	{
#if __cplusplus > 199711L
	    for(auto &func : vec)
#else
	    for (unsigned i = 0; i < vec.size(); ++i)
#endif

		{
#if __cplusplus > 199711L
		    delete func;
#else
		    delete vec[i];
#endif
		}
	}

    } // !core
} // !dim
