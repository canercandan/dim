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
	    virtual void firstCall(core::Pop<EOT>& /*pop*/, core::IslandData<EOT>& data)
	    {
		_reqs.resize( this->size() );

		for (size_t i = 0; i < this->size(); ++i)
		    {
			if (i == this->rank()) { continue; }
			_reqs[i] = this->world().send_init( i, this->tag(), data.proba[i] );
		    }
	    }

	    void operator()(core::Pop<EOT>& /*pop*/, core::IslandData<EOT>& data)
	    {
		/***********************************
		 * Send vecProbaRet to all islands *
		 ***********************************/

		for (size_t i = 0; i < this->size(); ++i)
		    {
			if (i == this->rank()) { continue; }
			this->world().start( _reqs[i] );
		    }

		// for island itself because of the MPI communication optimizing.
		data.probaret[this->rank()] = data.proba[this->rank()];

		/**************************************
		 * Receive probaret from all islands *
		 **************************************/

		std::vector< boost::mpi::request > recv_reqs;

		for (size_t i = 0; i < this->size(); ++i)
		    {
			if (i == this->rank()) { continue; }
			recv_reqs.push_back( this->world().irecv( i, this->tag(), data.probaret[i] ) );
		    }

		/****************************
		 * Process all MPI requests *
		 ****************************/

		boost::mpi::wait_all( _reqs.begin(), _reqs.end() );
		boost::mpi::wait_all( recv_reqs.begin(), recv_reqs.end() );
	    }

	private:
	    std::vector< boost::mpi::request > _reqs;
	};
    }
}

#endif /* _INPUTPROBASENDER_EASY_H_ */
