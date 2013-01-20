// -*- mode: c++; c-indent-level: 4; c++-member-init-indent: 8; comment-column: 35; -*-

//-----------------------------------------------------------------------------
// eoContinue.h
// (c) Maarten Keijzer, Geneura Team, 1999, 2000
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

#ifndef _CONTINUATOR_BASE_H_
#define _CONTINUATOR_BASE_H_

#include <eoFunctor.h>
#include <eoPersistent.h>

#include <dim/core/Pop.h>

namespace dim
{
    namespace continuator
    {

	/** Termination condition for the genetic algorithm
	 * Takes the population as input, returns true for continue,
	 * false for termination
	 *
	 * @ingroup Continuators
	 * @ingroup Continuator
	 */
	template< class EOT>
	class Base : public eoUF<const core::Pop<EOT>&, bool>, public eoPersistent
	{
	public:
	    virtual std::string className(void) const { return "Continue"; }

	    /** Read from a stream
	     * @param __is istream to read from
	     */
	    void readFrom (std :: istream & __is) {
		(void)__is;
		/* It should be implemented by subclasses ! */
	    }

	    /** Print on a stream
	     * @param __os ostream to print on
	     */
	    void printOn (std :: ostream & __os) const {
		(void)__os;
		/* It should be implemented by subclasses ! */
	    }
	};

	/**
	 * Termination condition with a count condition (totalGenerations). This continuator contains
	 * a count of cycles, which can be retrieved or set.
	 *
	 * @ingroup Continuators
	 * @ingroup Continuator
	 */
	template< class EOT >
	class Count : public Base< EOT >
	{
	public:

	    Count( ) :
		thisGenerationPlaceholder( 0 ),
		thisGeneration( thisGenerationPlaceholder )
	    {
		// empty
	    }

	    Count( unsigned long& currentGen ) :
		thisGenerationPlaceholder( 0 ),
		thisGeneration( currentGen )
	    {
		// empty
	    }

	    virtual std::string className( void ) const { return "Count"; }

	    virtual void reset( )
	    {
		thisGeneration = 0;
	    }

	protected:

	    unsigned long thisGenerationPlaceholder;
	    unsigned long& thisGeneration;
	};

    } // !continuator
} // !dim

#endif // !_CONTINUATOR_BASE_H_
