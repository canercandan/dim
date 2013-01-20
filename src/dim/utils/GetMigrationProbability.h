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

#ifndef _UTILS_GETMIGRATIONPROBABILITY_H_
#define _UTILS_GETMIGRATIONPROBABILITY_H_

#include <eo>

#include "Updater.h"

namespace dim
{
    namespace utils
    {

	class GetMigrationProbability : public Updater, public eoValueParam<double>
	{
	public:
	    GetMigrationProbability( const std::vector< double >& vecProba, const size_t isl, std::string label = "Value" )
		: eoValueParam<double>(0, label), _vecProba(vecProba), _isl(isl) {}

	    virtual void operator()()
	    {
		value() = _vecProba[_isl] / 10;
	    }

	private:
	    const std::vector< double >& _vecProba;
	    const size_t _isl;
	};

	class GetMigrationProbabilityRet : public Updater, public eoValueParam<double>
	{
	public:
	    GetMigrationProbabilityRet( const std::vector< double >& vecProbaRet, std::string label = "Value" )
		: eoValueParam<double>(0, label), _vecProbaRet(vecProbaRet) {}

	    virtual void operator()()
	    {
		value() = 0;
		for (auto i : _vecProbaRet)
		    {
			value() = value() + i;
		    }
		value() = value() / 10;
	    }

	private:
	    const std::vector< double >& _vecProbaRet;
	};

	class GetSumVectorProbability : public Updater, public eoValueParam<double>
	{
	public:
	    GetSumVectorProbability( const std::vector< double >& vecProba, std::string label = "Value" )
		: eoValueParam<double>(0, label), _vecProba(vecProba) {}

	    virtual void operator()()
	    {
		value() = accumulate(_vecProba.begin(), _vecProba.end(), 0) / 10.;
	    }

	private:
	    const std::vector< double >& _vecProba;
	};

    } // !utils
} // !dim

#endif /* _UTILS_GETMIGRATIONPROBABILITY_H_ */
