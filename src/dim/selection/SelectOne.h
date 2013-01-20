/** -*- mode: c++; c-indent-level: 4; c++-member-init-indent: 8; comment-column: 35; -*-

    -----------------------------------------------------------------------------
    SelectOne.h
    (c) Maarten Keijzer, GeNeura Team, 2000

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

#ifndef _SELECTION_SELECTONE_H_
#define _SELECTION_SELECTONE_H_

//-----------------------------------------------------------------------------
#include <eoFunctor.h>

#include <dim/core/Pop.h>
//-----------------------------------------------------------------------------

namespace dim
{
    namespace selection
    {

	/** SelectOne selects only one element from a whole population.
	    Most selection techniques are simply repeated applications
	    of SelectOne.

	    @see eoSelectMany, eoSelectRandom, eoDetTournament, eoStochTournament, eoProportional

	    @ingroup Selectors
	*/
#if  defined(_MSC_VER) && (_MSC_VER < 1300)
	template<class EOT, class WorthT = EOT::Fitness>
#else
	template<class EOT, class WorthT = typename EOT::Fitness>
#endif
	class SelectOne : public eoUF<const core::Pop<EOT>&, const EOT&>
	{
	public :
	    /// virtual function to setup some population stats (for instance eoProportional can benefit greatly from this)
	    virtual void setup(const core::Pop<EOT>& _pop)
	    {
		(void)_pop;
	    }
	};
	/** @example t-selectOne.cpp
	 */

    } // !selection
} // !dim

#endif // !_SELECTION_SELECTONE_H_
