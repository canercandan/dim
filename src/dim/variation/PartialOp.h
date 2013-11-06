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

#ifndef _VARIATION_PARTIALOP_H_
#define _VARIATION_PARTIALOP_H_

namespace dim
{
    namespace variation
    {

	template <typename EOT>
	class PartialOp
	{
	public:
	    virtual bool operator()(EOT& sol, size_t i, size_t j) = 0;
	};

	template <typename EOT>
	class InversionPartialOp : public PartialOp<EOT>
	{
	public:
	    /**
	     * Inversion two components of the given chromosome.
	     * @param sol The cromosome which is going to be changed.
	     */
	    virtual bool operator()(EOT& sol, size_t i, size_t j)
	    {
		unsigned from, to;

		// indexes
		from = std::min(i,j);
		to = std::max(i,j);

		size_t mid = (to-from)/2;

		// inversion
		for(size_t k = 0; k <= mid; ++k)
		    {
			size_t parity = (to-from)%2 == 0 ? 0 : 1;
			std::swap(sol[mid-k], sol[mid+k+parity]);
		    }

		return true;
	    }
	};

	template <typename EOT>
	class ShiftPartialOp : public PartialOp<EOT>
	{
	public:
	    /**
	     * Shift two components of the given chromosome.
	     * @param sol The cromosome which is going to be changed.
	     */
	    virtual bool operator()(EOT& sol, size_t i, size_t j)
	    {
		unsigned from, to;
		typename EOT::AtomType tmp;

		// indexes
		from=std::min(i,j);
		to=std::max(i,j);

		// keep the first component to change
		tmp=sol[to];

		// shift
		for(unsigned int k=to ; k > from ; k--)
		    {
			sol[k]=sol[k-1];
		    }

		// shift the first component
		sol[from]=tmp;

		return true;
	    }
	};

	template <typename EOT>
	class SwapPartialOp : public PartialOp<EOT>
	{
	public:
	    /**
	     * Swap two components of the given chromosome.
	     * @param sol The cromosome which is going to be changed.
	     */
	    virtual bool operator()(EOT& sol, size_t i, size_t j)
	    {
		std::swap(sol[i], sol[j]);
		return true;
	    }
	};

    }
}

#endif /* _VARIATION_PARTIALOP_H_ */
