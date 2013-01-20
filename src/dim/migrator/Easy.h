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

#ifndef _MIGRATOR_EASY_H_
#define _MIGRATOR_EASY_H_

#include <vector>
#include "Base.h"

namespace dim
{
    namespace migrator
    {
	template <typename EOT>
	class Easy : public Base<EOT>
	{
	public:
	    virtual void firstCall(core::Pop<EOT>& /*pop*/, core::IslandData<EOT>& data)
	    {
		_outputSizes.resize( this->size(), 0 );
		_reqs.resize( this->size() );
		_pops.resize( this->size() );

		for (size_t i = 0; i < this->size(); ++i)
		    {
			if (i == this->rank()) { continue; }
			_reqs[i] = this->world().send_init( i, this->tag(), _pops[i] );
		    }
	    }

	    void operator()(core::Pop<EOT>& pop, core::IslandData<EOT>& data)
	    {
		/***********************************
		 * Send individuals to all islands *
		 ***********************************/
		{
		    for (auto &pop : _pops)
			{
			    pop.clear();
			}

		    /*************
		     * Selection *
		     *************/

		    for (auto &indi : pop)
			{
			    double s = 0;
			    int r = rng.rand() % 1000 + 1;

			    size_t j;
			    for ( j = 0; j < this->size() && r > s; ++j )
				{
				    s += data.proba[j];
				}
			    --j;

			    _pops[j].push_back(indi);
			}

		    size_t outputSize = 0;

		    for (size_t i = 0; i < this->size(); ++i)
			{
			    if (i == this->rank()) { continue; }
			    _outputSizes[i] = _pops[i].size();
			    outputSize += _pops[i].size();
			}

		    pop.setOutputSizes( _outputSizes );
		    pop.setOutputSize( outputSize );

		    pop.clear();

		    for ( size_t i = 0; i < this->size(); ++i )
			{
			    if (i == this->rank()) { continue; }
			    this->world().start( _reqs[i] );
			}

		    dim::core::Pop<EOT>& newpop = _pops[this->rank()];
		    for (auto &indi : newpop)
			{
			    pop.push_back( indi );
			}
		}

		std::vector< dim::core::Pop<EOT> > recv_pops( this->size() );
		std::vector< boost::mpi::request > recv_reqs;

		/****************************************
		 * Receive individuals from all islands *
		 ****************************************/
		{
		    for (size_t i = 0; i < this->size(); ++i)
			{
			    if (i == this->rank()) { continue; }
			    reqs_reqs.push_back( this->world().irecv( i, this->tag(), recv_pops[i] ) );
			}
		}

		/****************************
		 * Process all MPI requests *
		 ****************************/

		boost::mpi::wait_all( _reqs.begin(), _reqs.end() );
		boost::mpi::wait_all( recv_reqs.begin(), recv_reqs.end() );

		/*********************
		 * Update population *
		 *********************/
		{
		    size_t inputSize = 0;

		    for (size_t i = 0; i < this->size(); ++i)
			{
			    if (i == this->rank()) { continue; }

			    dim::core::Pop<EOT>& newpop = recv_pops[i];
			    for (auto &indi : newpop)
				{
				    pop.push_back( indi );
				}

			    inputSize += newpop.size();
			}

		    pop.setInputSize( inputSize );
		}
	    }

	private:
	    std::vector< size_t > _outputSizes;
	    std::vector< dim::core::Pop<EOT> > _pops;
	    std::vector< boost::mpi::request > _reqs;
	};
    }
}

#endif /* _MIGRATOR_EASY_H_ */
