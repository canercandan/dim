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

#ifndef _INPUTPROBASENDER_EASY_H_
#define _INPUTPROBASENDER_EASY_H_

#include "Base.h"

namespace dim
{
    namespace inputprobasender
    {
	template <typename EOT>
	class Easy : public Base<EOT>
	{
	public:
	    void operator()(core::Pop<EOT>& /*pop*/, core::IslandData<EOT>& data)
	    {
		std::vector< boost::mpi::request > reqs;

		/***********************************
		 * Send vecProbaRet to all islands *
		 ***********************************/

		for (size_t i = 0; i < this->size(); ++i)
		    {
			if (i == this->rank()) { continue; }
			reqs.push_back( this->world().isend( i, this->tag(), data.proba[i] ) );
		    }

		// for island itself because of the MPI communication optimizing.
		data.probaret[this->rank()] = data.proba[this->rank()];

		/**************************************
		 * Receive probaret from all islands *
		 **************************************/

		for (size_t i = 0; i < this->size(); ++i)
		    {
			if (i == this->rank()) { continue; }
			reqs.push_back( this->world().irecv( i, this->tag(), data.probaret[i] ) );
		    }

		/****************************
		 * Process all MPI requests *
		 ****************************/

		boost::mpi::wait_all( reqs.begin(), reqs.end() );
	    }
	};
    }
}

#endif /* _INPUTPROBASENDER_EASY_H_ */
