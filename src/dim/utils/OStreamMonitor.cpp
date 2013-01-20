#ifdef _MSC_VER
// to avoid long name warnings
#pragma warning(disable:4786)
#endif

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <iomanip>
#include <ios>

#include <utils/compatibility.h>
#include <utils/eoParam.h>
#include <utils/eoLogger.h>

#include "OStreamMonitor.h"

//using namespace std;

namespace dim
{
    namespace utils
    {

	Monitor& OStreamMonitor::operator()(void)
	{
	    if (!out) {
		std::string str = "OStreamMonitor: Could not write to the ooutput stream";
		throw std::runtime_error(str);
	    }

	    if (firsttime) {

		eo::log << eo::debug << "First Generation" << std::endl;

		for (iterator it = vec.begin (); it != vec.end (); ++it) {
		    out << (*it)->longName ();
		    out << delim << std::left << std::setfill(fill) << std::setw(width);
		}
		out << std::endl;

		firsttime = false;
	    } // if firstime

	    for (iterator it = vec.begin (); it != vec.end (); ++it) {
		// value only
		out << (*it)->getValue ();
		out << delim << std::left << std::setfill(fill) << std::setw(width);
	    } // for it in vec

	    out << std::endl;
	    eo::log << eo::debug << "End of Generation" << std::endl;

	    return *this;
	}

    } // !utils
} // !dim
