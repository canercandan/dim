// -*- mode: c++; c-indent-level: 4; c++-member-init-indent: 8; comment-column: 35; -*-

//-----------------------------------------------------------------------------
// TimeCounter.h
// (c) Marc Schoenauer, Maarten Keijzer, and GeNeura Team, 2002
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
  Marc.Schoenauer@inria.fr
  mkeijzer@dhi.dk
*/
//-----------------------------------------------------------------------------

#ifndef _UTILS_TIMECOUNTER_H_
#define _UTILS_TIMECOUNTER_H_

#if __cplusplus > 199711L
#include <chrono>
#else
#include <boost/chrono/chrono_io.hpp>
#endif

#include "Stat.h"

namespace dim
{
    namespace utils
    {
#if __cplusplus > 199711L
	namespace std_or_boost = std;
#else
	namespace std_or_boost = boost;
#endif

	/**
	   An eoStat that simply gives the user time since first generation
	   It has to be tempatized by EOT because it must be an eoStat

	   @ingroup Stats
	*/
	class TimeCounter : public Updater, public eoValueParam<double>
	{
	public:
	    TimeCounter() : eoValueParam<double>(0.0, "Time"), _start( std_or_boost::chrono::system_clock::now() ) {}

	    /** simply stores the time spent in process in its value() */
	    virtual void operator()()
	    {
		// ask for wall clock
#if __cplusplus > 199711L
		auto end = std_or_boost::chrono::system_clock::now();
#else
	        std_or_boost::chrono::time_point< std_or_boost::chrono::system_clock > end = std_or_boost::chrono::system_clock::now();
#endif

#if __cplusplus > 199711L
		auto milliseconds_elapsed = std_or_boost::chrono::duration_cast<std_or_boost::chrono::milliseconds>( end - _start ).count();
#else
		unsigned milliseconds_elapsed = std_or_boost::chrono::duration_cast<std_or_boost::chrono::milliseconds>( end - _start ).count();
#endif

		value() = milliseconds_elapsed / 1000.;
	    }

	private:
	    std_or_boost::chrono::time_point< std_or_boost::chrono::system_clock > _start;
	};

    } // !utils
} // !dim

#endif // !_UTILS_TIMECOUNTER_H_
