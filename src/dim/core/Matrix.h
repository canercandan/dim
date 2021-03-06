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

#ifndef _CORE_MATRIX_H_
#define _CORE_MATRIX_H_

#include <vector>
#include <eo>

namespace dim
{
    namespace core
    {

	template < typename Atom >
	class SquareMatrix : public std::vector< Atom >, public eoObject, public eoPersistent
	{
	public:
	    // Ctor : sets size
	    SquareMatrix(unsigned _s = 0) : std::vector< Atom >(_s*_s), rSize(_s) {}

	    /** simple accessor */
	    const Atom& operator()(unsigned _i, unsigned _j) const
	    {
		return this->operator[](_i*rSize + _j);
	    }

	    /** reference - to set values */
	    Atom& operator()(unsigned _i, unsigned _j)
	    {
		return this->operator[](_i*rSize + _j);
	    }

	    /** returns a vector value in the row _i */
	    std::vector< Atom > operator()(unsigned _i)
	    {
		typename std::vector< Atom >::iterator begin = this->begin();
		std::advance(begin, _i * rSize);
		typename std::vector< Atom >::iterator end = begin;
		std::advance(end, rSize);
		std::vector< Atom > vec(begin, end);
		return vec;
	    }

	    /** just in case */
	    virtual void printOn(std::ostream & _os) const
	    {
		unsigned index=0;
		for (unsigned i=0; i<rSize; i++)
		    {
			for (unsigned j=0; j<rSize; j++)
			    {
				_os << this->operator[](index++) << " " ;
			    }
			_os << std::endl;
		    }
		_os << std::endl;
	    }

	    virtual void readFrom(std::istream& /*_is*/)
	    {
	    }

	    virtual std::string className() const {return "SquareMatrix";};

	    size_t size() const {return rSize;}

	private:
	    unsigned rSize;              // row size (== number of columns!)
	};

	class MigrationMatrix : public SquareMatrix< unsigned >
	{
	public:
	    MigrationMatrix(unsigned s = 0) : SquareMatrix< unsigned >(s) {}

	    virtual void printOn(std::ostream & os) const
	    {
		os << "Migration probability (in %) among islands" << std::endl;

		for (size_t i = 0; i < this->size(); ++i)
		    {
			os << "\t" << i;
		    }

		os << std::endl;

		for (size_t i = 0; i < this->size(); ++i)
		    {
			os << i;

			for (size_t j = 0; j < this->size(); ++j)
			    {
				os << "\t" << floor((*this)(i,j) * 100) / 1000;
			    }

			os << std::endl;
		    }

		os << std::endl;
		os << "sum";

		for (size_t i = 0; i < this->size(); ++i)
		    {
			unsigned sum = 0;
			for (size_t j = 0; j < this->size(); ++j)
			    {
				sum = sum + (*this)(j,i);
			    }
			os << "\t" << floor(sum * 100) / 1000;
		    }
		os << std::endl;
	    }
	};

	class InitMatrix : public eoUF< SquareMatrix< unsigned >&, void >
	{
	public:
	    InitMatrix(bool initG = false, double same = 90.) : _initG(initG), _same(unsigned(same*10.)) {}

	    void operator()(SquareMatrix< unsigned >& matrix)
	    {
		for (size_t i = 0; i < matrix.size(); ++i)
		    {
			unsigned sum = 0;

			for (size_t j = 0; j < matrix.size(); ++j)
			    {
				if (i == j)
				    {
					matrix(i,j) = _same;
				    }
				else
				    {
					if (_initG)
					    matrix(i,j) = (1000 - _same) / (matrix.size()-1);
					else
					    matrix(i,j) = eo::rng.rand();
					sum += matrix(i,j);
				    }
			    }

			for (size_t j = 0; j < matrix.size(); ++j)
			    {
				if (i != j)
				    {
					if (sum == 0)
					    matrix(i,j) = 0;
					else
					    matrix(i,j) = double(matrix(i,j)) / sum * (1000. - _same);
				    }
			    }
		    }
	    }


	private:
	    bool _initG;
	    unsigned _same;
	};

    } // !core
} // !dim

#endif /* _CORE_MATRIX_H_ */
