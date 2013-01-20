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

#ifndef _UTILS_FIXEDVALUE_H_
#define _UTILS_FIXEDVALUE_H_

#include <eo>

#include "Updater.h"

namespace dim
{
    namespace utils
    {

	template < typename T >
	class FixedValue : public Updater, public eoValueParam<T>
	{
	public:
	    FixedValue( T value = 0, std::string label = "Value" ) : eoValueParam<T>(value, label) {}

	    virtual void operator()() { /* nothing to do */ }
	};

    } // !utils
} // !dim

#endif /* _UTILS_FIXEDVALUE_H_ */
