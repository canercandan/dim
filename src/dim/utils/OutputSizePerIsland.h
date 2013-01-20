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

#ifndef _UTILS_OUTPUTSIZEPERISLAND_H_
#define _UTILS_OUTPUTSIZEPERISLAND_H_

#include <string>

#include "Stat.h"

namespace dim
{
    namespace utils
    {

	template < typename EOT >
	class OutputSizePerIsland : public Stat<EOT, size_t>
	{
	public:
	    OutputSizePerIsland( size_t isl, std::string _description = "Output size per island" )
		: Stat<EOT, size_t>(0, _description), _isl(isl)
	    {}

	    void operator()(const core::Pop<EOT>& _pop)
	    {
		if (_pop.getOutputSizes().empty()) { return; }

		this->value() = _pop.getOutputSizes()[this->_isl];
	    }

	private:
	    size_t _isl;
	};

    } // !utils
} // !dim

#endif /* _UTILS_OUTPUTSIZEPERISLAND_H_ */
