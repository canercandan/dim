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

#ifndef _VECTORUPDATER_BASE_H_
#define _VECTORUPDATER_BASE_H_

#include <dim/core/IslandOperator.h>

namespace dim
{
    namespace vectorupdater
    {
	template <typename EOT>
	class Base : public core::IslandOperator<EOT>
	{
	public:
	    virtual void firstCall(core::Pop<EOT>&, core::IslandData<EOT>&) {}
	    virtual void lastCall(core::Pop<EOT>&, core::IslandData<EOT>&) {}
	};
    }
}

#endif /* _VECTORUPDATER_BASE_H_ */
