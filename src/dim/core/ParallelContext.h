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

#ifndef _CORE_PARALLELCONTEXT_H_
#define _CORE_PARALLELCONTEXT_H_

#include <boost/mpi.hpp>

namespace dim
{
    namespace core
    {

	class ParallelContext
	{
	public:
	    ParallelContext(size_t tag = 0) : _size(_world.size()), _rank(_world.rank()), _tag(tag) {}
	    virtual ~ParallelContext() {}

	    inline size_t size() const { return _size; }
	    inline size_t rank() const { return _rank; }

	    inline void size(size_t v) { _size = v; }
	    inline void rank(size_t v) { _rank = v; }

	    inline boost::mpi::communicator& world() { return this->_world; }
	    inline size_t tag() const { return _tag; }

	private:
	    boost::mpi::communicator _world;
	    size_t _size;
	    size_t _rank;
	    const size_t _tag;
	};

    } // !core
} // !dim

#endif /* _CORE_PARALLELCONTEXT_H_ */
