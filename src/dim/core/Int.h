/*
   Int.h
// (c) Marc Schoenauer, Maarten Keijzer and GeNeura Team, 2000

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

    Contact: thomas.legrand@lifl.fr
             todos@geneura.ugr.es, http://geneura.ugr.es
             mkeijzer@dhi.dk
*/

#ifndef _CORE_INT_H_
#define _CORE_INT_H_

//-----------------------------------------------------------------------------

#include <iostream>    // std::ostream, std::istream
#include <string>      // std::string

#include "Vector.h"

namespace dim
{
    namespace core
    {

	/** eoInt: implementation of simple integer-valued chromosome.
	* based on Vector class
	*
	* @ingroup Representations
	*/
	template <class FitT> class Int: public Vector<FitT, int>
	{
	public:

	    /**
	    * (Default) Constructor.
	    * @param size Size of the std::vector
	    * @param value fill the vector with this value
	    */
	    Int(unsigned size = 0, int value = 0):
		Vector<FitT, int>(size, value) {}

	    /// My class name.
	    virtual std::string className() const
		{
		    return "Int";
		}

	};
	/** @example t-Int.cpp
	*/

	//-----------------------------------------------------------------------------

    } // !core
} // !dim

#endif // _CORE_INT_H_

// Local Variables:
// coding: iso-8859-1
// mode: C++
// c-file-offsets: ((c . 0))
// c-file-style: "Stroustrup"
// fill-column: 80
// End:
