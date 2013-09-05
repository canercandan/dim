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

#include "TSPLibGraph.h"

namespace dim
{
    namespace initialization
    {

	namespace TSPLibGraph
	{
	    static Distance dist; // Distance Mat.

	    unsigned size() { return dist.size(); }
	    double distance(unsigned __from, unsigned __to) { return dist[__from][__to]; }

	    void load(std::string filename)
	    {
		using boost::property_tree::ptree;
		ptree pt;

		read_xml(filename, pt);

		BOOST_FOREACH(ptree::value_type const& vertex, pt.get_child ("travellingSalesmanProblemInstance.graph"))
		    {
			std::map< unsigned, double > vertex_dist;
			BOOST_FOREACH(ptree::value_type const& edge, vertex.second)
			    {
				vertex_dist[ edge.second.get<unsigned>("") ] = edge.second.get<double>("<xmlattr>.cost");
			    }
			dist.push_back( vertex_dist );
		    }
	    }
	}

    }
}
