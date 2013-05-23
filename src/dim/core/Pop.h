// -*- mode: c++; c-indent-level: 4; c++-member-init-indent: 8; comment-column: 35; -*-

//-----------------------------------------------------------------------------
// eoPop.h
// (c) GeNeura Team, 1998
/*
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

  Authors:
  todos@geneura.ugr.es, http://geneura.ugr.es
  jmerelo
  gustavoromero
  mac
  maartenkeijzer
  kuepper
  okoenig
  evomarc
  Johann Dréo <johann.dreo@thalesgroup.com>
*/
//-----------------------------------------------------------------------------

#ifndef _CORE_POP_H_
#define _CORE_POP_H_

#include <algorithm>
#include <iostream>
#include <iterator> // needed for GCC 3.2
#include <vector>
#include <assert.h>

// EO includes
#include <eoOp.h> // for eoInit
#include <eoPersistent.h>
#include <eoInit.h>
#include <utils/rnd_generators.h>  // for shuffle method

#include <boost/mpi.hpp>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include <boost/serialization/vector.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/assume_abstract.hpp>

#if __cplusplus > 199711L
#include <mutex>
#else
#include <boost/thread/mutex.hpp>
#endif

namespace dim
{
    namespace core
    {

#if __cplusplus > 199711L
	namespace std_or_boost = std;
#else
	namespace std_or_boost = boost;
#endif


	/** A std::vector of EO object, to be used in all algorithms
	 *      (selectors, operators, replacements, ...).
	 *
	 * We have no idea if a population can be
	 * some other thing that a std::vector, but if somebody thinks of it, this concrete
	 * implementation can be moved to "generic" and an abstract Population
	 * interface be provided.
	 *
	 * The template can be instantiated with anything that accepts a "size"
	 * and eoInit derived object. in the ctor.
	 * EOT must also have a copy ctor, since temporaries are created and then
	 * passed to the eoInit object
	 *
	 * @ingroup Core
	 */
	template<class EOT>
	class Pop: public std::vector<EOT>, public eoObject, public eoPersistent
	{
	public:
	    friend class boost::serialization::access;

	    using std::vector<EOT>::size;
	    using std::vector<EOT>::resize;
	    using std::vector<EOT>::operator[];
	    using std::vector<EOT>::begin;
	    using std::vector<EOT>::end;

	    typedef typename EOT::Fitness Fitness;
#if defined(__CUDACC__)
	    typedef typename std::vector<EOT>::iterator iterator;
	    typedef typename std::vector<EOT>::const_iterator const_iterator;
#endif

	    /** Default ctor. Creates empty pop
	     */
	    Pop()   : std::vector<EOT>(), eoObject(), eoPersistent(), inputSize(0), outputSize(0)
	    {};

	    /** Ctor for the initialization of chromosomes

		@param _popSize total population size
		@param _chromInit Initialization routine, produces EO's, needs to be an eoInit
	    */
	    Pop( unsigned _popSize, eoInit<EOT>& _chromInit )
		: std::vector<EOT>(), inputSize(0), outputSize(0)
	    {
		resize(_popSize);
		for ( unsigned i = 0; i < _popSize; i++ )
		    {
			_chromInit(operator[](i));
		    }
	    }

	    Pop(const Pop& pop) { *this = pop; }

	    Pop& operator=( const Pop& pop )
	    {
		if ( &pop != this )
		    {
			// resize(pop.size());
			// for ( unsigned i = 0; i < pop.size(); i++ )
			//     {
			// 	operator[](i) = pop[i];
			//     }
			// std::copy(pop.begin(), pop.end(), begin());
			inputSize = pop.inputSize;
			outputSize = pop.outputSize;
			outputSizes = pop.outputSizes;
		    }
		return *this;
	    }

	    /** appends random guys at end of pop.
		Can be used to initialize it pop is empty

		@param _newPopSize total population size
		@param _chromInit Initialization routine, produces EO's, needs to be an eoInit
	    */
	    void append( unsigned _newPopSize, eoInit<EOT>& _chromInit )
	    {
		unsigned oldSize = size();
		if (_newPopSize < oldSize)
		    {
			throw std::runtime_error("New size smaller than old size in pop.append");
			return;
		    }
		if (_newPopSize == oldSize)
		    return;
		resize(_newPopSize);         // adjust the size
		for ( unsigned i = oldSize; i < _newPopSize; i++ )
		    {
			_chromInit(operator[](i));
		    }
	    }


	    /** Ctor from an std::istream; reads the population from a stream,
		each element should be in different lines
		@param _is the stream
	    */
	    Pop( std::istream& _is ) :std::vector<EOT>()
	    {
		readFrom( _is );
	    }


	    /** Empty Dtor */
	    virtual ~Pop() {}


	    /// helper struct for getting a pointer
	    struct Ref { const EOT* operator()(const EOT& eot) { return &eot;}};

	    /// helper struct for comparing on pointers
	    struct Cmp {
		bool operator()(const EOT* a, const EOT* b) const
		{ return b->operator<(*a); }
	    };

	    /// helper struct for comparing (EA or PSO)
	    struct Cmp2
	    {
		bool operator()(const EOT & a,const EOT & b) const
		{
		    return b.operator<(a);
		}
	    };


	    /**
	       sort the population. Use this member to sort in order
	       of descending Fitness, so the first individual is the best!
	    */
	    void sort(void)
	    {
		std::sort(begin(), end(), Cmp2());
	    }


	    /** creates a std::vector<EOT*> pointing to the individuals in descending order */
	    void sort(std::vector<const EOT*>& result) const
	    {
		result.resize(size());

		std::transform(begin(), end(), result.begin(), Ref());

		std::sort(result.begin(), result.end(), Cmp());
	    }


	    /**
	       shuffle the population. Use this member to put the population
	       in random order
	    */
	    void shuffle(void)
	    {
		UF_random_generator<unsigned int> gen;
		std::random_shuffle(begin(), end(), gen);
	    }


	    /** creates a std::vector<EOT*> pointing to the individuals in random order */
	    void shuffle(std::vector<const EOT*>& result) const
	    {
		result.resize(size());

		std::transform(begin(), end(), result.begin(), Ref());

		UF_random_generator<unsigned int> gen;
		std::random_shuffle(result.begin(), result.end(), gen);
	    }


	    /** returns an iterator to the best individual DOES NOT MOVE ANYBODY */
#if defined(__CUDACC__)
	    Pop<EOT>::iterator it_best_element()
	    {
		Pop<EOT>:: iterator it = std::max_element(begin(), end());
		return it;
	    }
#else
	    typename Pop<EOT>::iterator it_best_element()
	    {
                assert( this->size() > 0 );
                typename Pop<EOT>::iterator it = std::max_element(begin(), end());
                return it;
	    }
#endif


	    /** returns an iterator to the best individual DOES NOT MOVE ANYBODY */
	    const EOT & best_element() const
	    {
#if defined(__CUDACC__)
		Pop<EOT>::const_iterator it = std::max_element(begin(), end());
#else
		typename Pop<EOT>::const_iterator it = std::max_element(begin(), end());
#endif
		if( it == end() )
		    throw std::runtime_error("Pop<EOT>: Empty population, when calling best_element().");
		return (*it);
	    }

	    /** returns a const reference to the worse individual DOES NOT MOVE ANYBODY */
	    const EOT & worse_element() const
	    {
#if defined(__CUDACC__)
		Pop<EOT>::const_iterator it = std::min_element(begin(), end());
#else
		assert( this->size() > 0 );
		typename Pop<EOT>::const_iterator it = std::min_element(begin(), end());
#endif
		return (*it);
	    }


	    /** returns an iterator to the worse individual DOES NOT MOVE ANYBODY */
#if defined(__CUDACC__)
	    Pop<EOT>::iterator it_worse_element()
	    {
		Pop<EOT>::iterator it = std::min_element(begin(), end());
		return it;
	    }
#else
	    typename Pop<EOT>::iterator it_worse_element()
	    {
		assert( this->size() > 0 );
		typename Pop<EOT>::iterator it = std::min_element(begin(), end());
		return it;
	    }
#endif


	    /**
	       slightly faster algorithm than sort to find all individuals that are better
	       than the nth individual. INDIVIDUALS ARE MOVED AROUND in the pop.
	    */
#if defined(__CUDACC__)
	    Pop<EOT>::iterator nth_element(int nth)
	    {
		Pop<EOT>::iterator it = begin() + nth;
		std::nth_element(begin(), it, end(), std::greater<EOT>());
		return it;
	    }
#else
	    typename Pop<EOT>::iterator nth_element(int nth)
	    {
		assert( this->size() > 0 );
		typename Pop<EOT>::iterator it = begin() + nth;
		std::nth_element(begin(), it, end(), std::greater<EOT>());
		return it;
	    }
#endif


	    struct GetFitness { Fitness operator()(const EOT& _eo) const { return _eo.fitness(); } };


	    /** returns the fitness of the nth element */
	    Fitness nth_element_fitness(int which) const
	    { // probably not the fastest way to do this, but what the heck

		std::vector<Fitness> fitness(size());
		std::transform(begin(), end(), fitness.begin(), GetFitness());

		typename std::vector<Fitness>::iterator it = fitness.begin() + which;
		std::nth_element(fitness.begin(), it, fitness.end(), std::greater<Fitness>());
		return *it;
	    }


	    /** const nth_element function, returns pointers to sorted individuals
	     * up the the nth
	     */
	    void nth_element(int which, std::vector<const EOT*>& result) const
	    {

		assert( this->size() > 0 );
		result.resize(size());
		std::transform(begin(), end(), result.begin(), Ref());

		typename std::vector<const EOT*>::iterator it  = result.begin() + which;

		std::nth_element(result.begin(), it, result.end(), Cmp());
	    }


	    /** does STL swap with other pop */
	    void swap(Pop<EOT>& other)
	    {
		std::swap(static_cast<std::vector<EOT>& >(*this), static_cast<std::vector<EOT>& >(other));
	    }


	    /**
	     * Prints sorted pop but does NOT modify it!
	     *
	     * @param _os A std::ostream.
	     */
	    virtual void sortedPrintOn(std::ostream& _os) const
	    {
		std::vector<const EOT*> result;
		sort(result);
		_os << size() << '\n';
		for (unsigned i = 0; i < size(); ++i)
		    {
			_os << *result[i] << std::endl;
		    }
	    }


	    /**
	     * Write object. It's called printOn since it prints the object _on_ a stream.
	     * @param _os A std::ostream.
	     */
	    virtual void printOn(std::ostream& _os) const
	    {
		_os << size() << '\n';
		std::copy( begin(), end(), std::ostream_iterator<EOT>( _os, "\n") );
	    }


	    /** @name Methods from eoObject	*/
	    //@{
	    /**
	     * Read object. The EOT class must have a ctor from a stream;
	     * @param _is A std::istream.
	     */
	    virtual void readFrom(std::istream& _is)
	    {
		size_t sz;
		_is >> sz;

		resize(sz);

		for (size_t i = 0; i < sz; ++i) {
		    operator[](i).readFrom( _is );
		}
	    }


	    /** Inherited from eoObject. Returns the class name.
		@see eoObject
	    */
	    virtual std::string className() const {return "Pop";};
	    //@}


	    /** Invalidate the whole population
	     */
	    virtual void invalidate()
	    {
		for (unsigned i=0; i<size(); i++)
		    this->operator[](i).invalidate();
	    }

	    inline void setInputSize( size_t value ) { this->inputSize = value; }
	    inline void setOutputSize( size_t value ) { this->outputSize = value; }
	    inline void setOutputSizes( const std::vector<size_t>& values ) { this->outputSizes = values; }

	    inline size_t getInputSize() const { return this->inputSize; }
	    inline size_t getOutputSize() const { return this->outputSize; }
	    inline const std::vector<size_t>& getOutputSizes() const { return this->outputSizes; }

	public:
	    template<class Archive>
	    void serialize(Archive & ar, const unsigned int /*version*/)
	    {
		ar & boost::serialization::base_object< std::vector<EOT> >(*this);
		ar & inputSize & outputSize & outputSizes;
	    }

	private:
	    size_t inputSize;
	    size_t outputSize;
	    std::vector<size_t> outputSizes;

	// public:
	//     std_or_boost::mutex mutex;

	}; // class Pop

    } // !core
} // !dim

#endif // _CORE_POP_H_
