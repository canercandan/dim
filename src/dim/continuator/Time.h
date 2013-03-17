// -*- mode: c++; c-indent-level: 4; c++-member-init-indent: 8; comment-column: 35; -*-

//-----------------------------------------------------------------------------
// Time.h
// (c) OPAC Team (LIFL), Dolphin Project (INRIA), 2007
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

  Contact: thomas.legrand@lifl.fr
*/
//-----------------------------------------------------------------------------

#ifndef _CONTINUATOR_TIME_H_
#define _CONTINUATOR_TIME_H_

#include <chrono>

#include "Base.h"
#include <utils/eoLogger.h>

namespace dim
{
    namespace continuator
    {

	/**
	 * Termination condition until a running time is reached.
	 *
	 * @ingroup Continuators
	 */
	template < class EOT >
	class Time : public Base<EOT>
	{
	public:
	    /**
	     * Ctor.
	     * @param _max maximum running time
	     */
	    Time(long max) : _start( std::chrono::system_clock::now() ), _end( std::chrono::system_clock::now() + std::chrono::seconds(max) ), _max(max) {}

	    /**
	     * Returns false when the running time is reached.
	     * @param _pop the population
	     */
	    virtual bool operator() (const core::Pop<EOT> & /*_pop*/)
	    {
		if ( std::chrono::system_clock::now() >= _end )
		    {
			auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(_end-_start).count();
			eo::log << eo::progress << "STOP in Continuator::Time: Reached maximum time [" << elapsed << "/" << _max << "]" << std::endl;
			return false;
		    }
		return true;
	    }

	    /**
	     * Class name
	     */
	    virtual std::string className(void) const { return "Time"; }

	private:
	    std::chrono::time_point< std::chrono::system_clock > _start, _end;
	    long _max;
	};

    } // !continuator
} // !dim

#endif // !_CONTINUATOR_TIME_H_
