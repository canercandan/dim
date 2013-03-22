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

#if __cplusplus > 199711L
#include <chrono>
#else
#include <boost/chrono/chrono_io.hpp>
#endif

#include "Base.h"
#include <utils/eoLogger.h>

namespace dim
{
    namespace continuator
    {
#if __cplusplus > 199711L
	namespace std_or_boost = std;
#else
	namespace std_or_boost = boost;
#endif

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
	    Time(long max) : _start( std_or_boost::chrono::system_clock::now() ), _end( std_or_boost::chrono::system_clock::now() + std_or_boost::chrono::seconds(max) ), _max(max) {}

	    /**
	     * Returns false when the running time is reached.
	     * @param _pop the population
	     */
	    virtual bool operator() (const core::Pop<EOT> & /*_pop*/)
	    {
		if ( std_or_boost::chrono::system_clock::now() >= _end )
		    {
#if __cplusplus > 199711L
			auto elapsed = std_or_boost::chrono::duration_cast<std_or_boost::chrono::seconds>(_end-_start).count();
#else
			unsigned elapsed = std_or_boost::chrono::duration_cast<std_or_boost::chrono::seconds>(_end-_start).count();
#endif

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
	    std_or_boost::chrono::time_point< std_or_boost::chrono::system_clock > _start, _end;
	    long _max;
	};

    } // !continuator
} // !dim

#endif // !_CONTINUATOR_TIME_H_
