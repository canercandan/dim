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

#ifndef _VARIATION_COMPARISONOP_H_
#define _VARIATION_COMPARISONOP_H_

namespace dim
{
    namespace variation
    {

	template <typename EOT>
	class ComparisonOp
	{
	public:
	    virtual bool operator()(typename EOT::Fitness delta, typename EOT::Fitness fit) = 0;
	};

	template <typename EOT>
	class NeutralComparisonOp : public ComparisonOp<EOT>
	{
	public:
	    virtual bool operator()(typename EOT::Fitness delta, typename EOT::Fitness fit) { return delta <= fit; }
	};

	template <typename EOT>
	class StrictComparisonOp : public ComparisonOp<EOT>
	{
	public:
	    virtual bool operator()(typename EOT::Fitness delta, typename EOT::Fitness fit) { return delta < fit; }
	};
    }
}

#endif /* _VARIATION_COMPARISONOP_H_ */
