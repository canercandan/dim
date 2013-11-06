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

  (c) Thales group 2011

  Author: johann.dreo@thalesgroup.com
*/

#ifndef _UTILS_EVALCOUNTER_H_
#define _UTILS_EVALCOUNTER_H_

#include <string>

#include "Stat.h"

namespace dim
{
    namespace utils
    {

	/**
	   An eoStat that simply gives the current generation index

	   @ingroup Stats
	*/
	template< typename EOT >
	class EvalCounter : public Updater, public eoValueParam<unsigned int>
	{
	public:
	    EvalCounter( eoEvalFuncCounter<EOT>& _eval, std::string label = "Eval" ) : eoValueParam<unsigned int>(0, label), eval(_eval) {}

	    virtual void operator()()
	    {
		value() = eval.value();
	    }

	private:
	    eoEvalFuncCounter<EOT>& eval;
	};

    } // !utils
} // !dim

#endif // !_UTILS_EVALCOUNTER_H_
