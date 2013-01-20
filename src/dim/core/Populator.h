// -*- mode: c++; c-indent-level: 4; c++-member-init-indent: 8; comment-column: 35; -*-

//-----------------------------------------------------------------------------
// Populator.h
// (c) Maarten Keijzer and Marc Schoenauer, 2001
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

  Contact: mkeijzer@dhi.dk
  Marc.Schoenauer@polytechnique.fr
*/
//-----------------------------------------------------------------------------

#ifndef _CORE_POPULATOR_H_
#define _CORE_POPULATOR_H_

#include <dim/selection/SelectOne.h>

#include "Pop.h"

namespace dim
{
    namespace core
    {

	/** Populator is a helper class for general operators eoGenOp
	    It is an Pop but also behaves like an Pop::iterator
	    as far as operator* and operator++ are concerned

	    @see eoGenOp
	    @see eoOpContainer

	    @ingroup Core
	    @ingroup Utilities
	*/
	template <class EOT>
	class Populator
	{
	public :

	    Populator(const Pop<EOT>& _src, Pop<EOT>& _dest) : dest(_dest), current(dest.end()), src(_src)
	    {
		dest.reserve(src.size()); // we don't know this, but wth.
		current = dest.end();
	    }

	    /** @brief Virtual Constructor */
	    virtual ~Populator() {};

	    struct OutOfIndividuals {};

	    /** a populator behaves like an iterator. Hence the operator*
	     *      it returns the current individual -- eventually getting
	     *      a new one through the operator++ if at the end
	     */
	    EOT& operator*(void)
	    {
		if (current == dest.end())
		    get_next(); // get a new individual

		return *current;
	    }

	    /** only prefix increment defined
		Does not add a new element when at the end, use operator* for that
		If not on the end, increment the pointer to the next individual
	    */
	    Populator& operator++()
	    {
		if (current == dest.end())
		    { // keep the pointer there
			return *this;
		    }
		// else
		++current;
		return *this;
	    }

	    /** mandatory for operators that generate more offspring than parents
	     *  if such a thing exists ?
	     */
	    void insert(const EOT& _eo)
	    { /* not really efficient, but its nice to have */
		current = dest.insert(current, _eo);
	    }

	    /** just to make memory mangement more efficient
	     */
	    void reserve(int how_many)
	    {
		size_t sz = current - dest.begin();
		if (dest.capacity() < dest.size() + how_many)
		    {
			dest.reserve(dest.size() + how_many);
		    }

		current = dest.begin() + sz;
	    }

	    /** can be useful for operators with embedded selectors
	     *  e.g. your brain and my beauty -type
	     */
	    const Pop<EOT>& source(void) { return src; }

	    /** Get the offspring population.
		Can be useful when you want to do some online niching kind of thing
	    */
	    Pop<EOT>& offspring(void)    { return dest; }

	    typedef unsigned position_type;

	    /** this is a direct access container: tell position */
	    position_type tellp()         { return current - dest.begin(); }
	    /** this is a direct access container: go to position */
	    void seekp(position_type pos) { current = dest.begin() + pos; }
	    /** no more individuals  */
	    bool exhausted(void)          { return current == dest.end(); }

	    /** the pure virtual selection method - will be instanciated in
	     *   eoSeqPopulator and eoSelectivePopulator
	     */
	    virtual const EOT& select() = 0;

	protected:
	    Pop<EOT>& dest;
	    typename Pop<EOT>::iterator current;
	    const Pop<EOT>& src;

	private:

	    void get_next() {
		if(current == dest.end())
		    { // get new individual from derived class select()
			dest.push_back(select());
			current = dest.end();
			--current;
			return;
		    }
		// else
		++current;
		return;
	    }

	};


	/** SeqPopulator: an Populator that sequentially goes through the
	    population is supposed to be used after a batch select of a whole
	    bunch or genitors
	*/
	template <class EOT>
	class SeqPopulator : public Populator<EOT>
	{
	public:

	    using Populator< EOT >::src;

	    SeqPopulator(const Pop<EOT>& _pop, Pop<EOT>& _dest) :
		Populator<EOT>(_pop, _dest), current(0) {}

	    /** the select method simply returns next individual in the src pop */
	    const EOT& select(void) {
		if(current >= Populator< EOT >::src.size()) {
		    throw OutOfIndividuals();
		}

		const EOT& res = src[current++];
		return res;
	    }


	private:

	    struct OutOfIndividuals {};

	    unsigned current;
	};


	/** SelectivePopulator an Poplator that uses an selection::SelectOne to select guys.
	    Supposedly, it is passed the initial population.
	*/
	template <class EOT>
	class SelectivePopulator : public Populator<EOT>
	{
	public :

	    using Populator< EOT >::src;

	    SelectivePopulator(const Pop<EOT>& _pop, Pop<EOT>& _dest, selection::SelectOne<EOT>& _sel)
		: Populator<EOT>(_pop, _dest), sel(_sel)
	    { sel.setup(_pop); };

	    /** the select method actually selects one guy from the src pop */
	    const EOT& select() {
		return sel(src);
	    }


	private:

	    selection::SelectOne<EOT>& sel;
	};

    } // !core
} // !dim

#endif // !_CORE_POPULATOR_H_
