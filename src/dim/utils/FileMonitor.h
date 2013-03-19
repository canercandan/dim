// -*- mode: c++; c-indent-level: 4; c++-member-init-indent: 8; comment-column: 35; -*-

//-----------------------------------------------------------------------------
// FileMonitor.h
// (c) Marc Schoenauer, Maarten Keijzer and GeNeura Team, 2000
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
  Marc.Schoenauer@polytechnique.fr
  mkeijzer@dhi.dk
*/
//-----------------------------------------------------------------------------

#ifndef _UTILS_FILEMONITOR_H_
#define _UTILS_FILEMONITOR_H_

#include <string>
#include <fstream>
#include <stdexcept>
#include <chrono>

#include "Monitor.h"
#include "eoObject.h"

namespace dim
{
    namespace utils
    {

	/** Prints statistics to file

	    Modified the default behavior, so that it erases existing files. Can
	    be modified in the ctor.

	    @version MS 25/11/00
	    @ingroup Monitors
	*/
	class FileMonitor : public Monitor
	{
	public :

	    /*! Constructor
	     *
	     * Try to create the file in writing mode, erasing it if asked.
	     *
	     * @param _filename complete filename to write to
	     * @param _frequency write to the file at this frequency
	     * @param _delim delimiter string to use between each item of the registered vector (e.g. of eoStats)
	     * @param _counter beginning value of the counter
	     * @param _keep_existing if true, overwrite any existing file with the same name prior to any output
	     * @param _header print the header (with the descriptions of registered eoStats) at the beginning of the file (WARNING: true will discards header printing)
	     * @param _overwrite if true, overwrite the existing file
	     */
	    FileMonitor(
			std::string _filename,
			unsigned _frequency = 1,
			std::string _delim = " ",
			unsigned _counter = 0,
			bool _keep_existing = false,
			bool _header = false,
			bool _overwrite = false,
			unsigned _stepTimer = 1000
			)
		: filename(_filename),
		  frequency(_frequency),
		  delim(_delim),
		  counter(_counter),
		  keep(_keep_existing),
		  header(_header),
		  firstcall(true),
		  overwrite(_overwrite),
		  stepTimer(_stepTimer)
	    {
		if (!_keep_existing) {
		    std::ofstream os (filename.c_str ());

		    if (!os) {
			std::string str = "Error, FileMonitor could not open: " + filename;
			throw std::runtime_error (str);
		    }
		} // if ! keep
	    }

	    //! Called first, try to open the file in append mode and write the header if asked
	    virtual Monitor& operator()(void);

	    /*! Main call, normally called at each generation.
	      Write the content of the registered vector into the file, each item being separated by delim
	    */
	    virtual Monitor& operator()(std::ostream& os);

	    //! Try to open the file, and then call printHeader(file)
	    void printHeader(void);

	    //! Print long names of the registered items, separated by delim.
	    virtual void printHeader(std::ostream& os);

	    virtual std::string getFileName() { return filename;}

	private :

	    //! complete filename to write to
	    std::string filename;

	    //! write to the file at a frequency
	    unsigned frequency;

	    //! delimiter to use between each write
	    std::string delim;

	    // beginnig value of the counter
	    unsigned int counter;

	    //! should we append or create a new file
	    bool keep;

	    //! printing header at begin of file?
	    bool header;

	    //! flag to avoid calling twice operator()(void)
	    bool firstcall;

	    //! erase the entire file prior to writing in it (mode eos_base::
	    bool overwrite;

	    //! step timer
	    unsigned stepTimer;

	    //! start time
	    std::chrono::time_point< std::chrono::system_clock > start = std::chrono::system_clock::now();

	    //! last elapsed time
	    unsigned lastElapsedTime = 0;
	};

    } // !utils
} // !dim

#endif // !_UTILS_FILEMONITOR_H_
