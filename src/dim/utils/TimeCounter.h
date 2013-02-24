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

#include <chrono>

#include "Stat.h"

namespace dim
{
    namespace utils
    {

	/**
	   An eoStat that simply gives the user time since first generation
	   It has to be tempatized by EOT because it must be an eoStat

	   @ingroup Stats
	*/
	class TimeCounter : public Updater, public eoValueParam<double>
	{
	public:
	    TimeCounter() : eoValueParam<double>(0.0, "Time"), _start( std::chrono::system_clock::now() ) {}

	    /** simply stores the time spent in process in its value() */
	    virtual void operator()()
	    {
		// ask for wall clock
		auto end = std::chrono::system_clock::now();
		auto milliseconds_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>( end - _start ).count();
		value() = milliseconds_elapsed / 1000.;
	    }

	private:
	    std::chrono::time_point< std::chrono::system_clock > _start;
	};

    } // !utils
} // !dim

#endif // !_UTILS_TIMECOUNTER_H_
