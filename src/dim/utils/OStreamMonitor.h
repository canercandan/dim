/*

  (c) Marc Schoenauer, Maarten Keijzer and GeNeura Team, 2000
  (c) Thales group, 2010

  This library is free software; you can redistribute it and/or modify it under
  the terms of the GNU Lesser General Public License as published by the Free
  Software Foundation; version 2 of the license.

  This library is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
  PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License along
  with this library; if not, write to the Free Software Foundation, Inc., 59
  Temple Place, Suite 330, Boston, MA 02111-1307 USA

  Contact: http://eodev.sourceforge.net

  Authors:
  todos@geneura.ugr.es
  Marc.Schoenauer@polytechnique.fr
  mkeijzer@dhi.dk
  Johann Dréo <johann.dreo@thalesgroup.com>
*/

#ifndef _UTILS_OSTREAMMONITOR_H_
#define _UTILS_OSTREAMMONITOR_H_

#include <string>
#include <iostream>

#if __cplusplus > 199711L
#include <chrono>
#else
#include <boost/chrono/chrono_io.hpp>
#endif

#include <utils/eoLogger.h>
#include <eoObject.h>

#include "Monitor.h"

#include <boost/utility/identity_type.hpp>

#undef AUTO
#if __cplusplus > 199711L
#define AUTO(TYPE) auto
#else // __cplusplus <= 199711L
#define AUTO(TYPE) TYPE
#endif

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
	   Prints statistics to a given ostream.

	   You can pass any instance of an ostream to the constructor, like, for example, std::clog.

	   @ingroup Monitors
	*/
	class OStreamMonitor : public Monitor
	{
	public:
	    OStreamMonitor(
			   std::ostream & _out,
			   std::string _delim = "\t",
			   unsigned int _width=20,
			   char _fill=' ',
			   unsigned _stepTimer=1000
			   )
		: out(_out),
		  delim(_delim),
		  width(_width),
		  fill(_fill),
		  stepTimer(_stepTimer),
		  start( std_or_boost::chrono::system_clock::now() ),
		  lastElapsedTime(0),
		  firsttime(true)
	    {}

	    Monitor& operator()(void);

	    virtual std::string className(void) const { return "OStreamMonitor"; }

	private:
	    std::ostream & out;
	    std::string delim;
	    unsigned int width;
	    char fill;

	    //! step timer
	    unsigned stepTimer;

	    //! start time
	    std_or_boost::chrono::time_point< std_or_boost::chrono::system_clock > start;

	    //! last elapsed time
	    unsigned lastElapsedTime;

	    bool firsttime;
	};

    } // !utils
} // !dim

#endif // !_UTILS_OSTREAMMONITOR_H_
