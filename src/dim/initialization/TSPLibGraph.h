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

#ifndef _INITIALIZATION_TSPLIBGRAPH_H_
#define _INITIALIZATION_TSPLIBGRAPH_H_

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>
#include <vector>
#include <map>

namespace dim
{
    namespace initialization
    {

	namespace TSPLibGraph
	{
	    typedef std::vector< std::map< unsigned, double > > Distance;

	    unsigned size();
	    double distance(unsigned __from, unsigned __to);
	    void load(std::string filename);
	}

    }
}

#endif // !_INITIALIZATION_TSPLIBGRAPH_H_
