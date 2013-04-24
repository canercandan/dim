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

#ifndef _CORE_THREAD_H_
#define _CORE_THREAD_H_

// #if __cplusplus > 199711L
// #include <thread>
// #else
#include <boost/thread.hpp>
// #endif

#include <vector>

#include "BF.h"
#include "Pop.h"
#include "IslandData.h"

namespace dim
{
    namespace core
    {
#if __cplusplus > 199711L
	namespace std_or_boost = std;
#else
	namespace std_or_boost = boost;
#endif

	template <typename EOT>
	class ThreadsRunner;

	template <typename EOT>
	class Thread : public BF< Pop<EOT>&, IslandData<EOT>&, void >
	{
	public:
	    virtual ~Thread() {}

	    // virtual void operator()(Pop<EOT>&, IslandData<EOT>&) = 0;

	private:
	    friend class ThreadsRunner<EOT>;

	    void build(Pop<EOT>& pop, IslandData<EOT>& data)
	    {
		t = /*std_or_*/boost::thread( /*std_or_*/boost::ref(*this), /*std_or_*/boost::ref(pop), /*std_or_*/boost::ref(data) );
	    }

	    inline void join() { t.join(); }

	protected:
	    /*std_or_*/boost::thread t;
	};

	template <typename EOT>
	class ThreadsHandler
	{
	public:
	    virtual ~ThreadsHandler() {}

	    virtual void addTo(ThreadsRunner<EOT>&) = 0;
	};

	template <typename EOT>
	class ThreadsRunner
	{
	public:
	    void operator()(Pop<EOT>& pop, IslandData<EOT>& data)
	    {
#if __cplusplus > 199711L
		for (auto& t : _vt) { t->build(pop, data); }
		for (auto& t : _vt) { t->join(); }
#else
		for (size_t i = 0; i < _vt.size(); ++i) { _vt[i]->build(pop, data); }
		for (size_t i = 0; i < _vt.size(); ++i) { _vt[i]->join(); }
#endif
	    }

	    ThreadsRunner& add(Thread<EOT>& t)
	    {
		_vt.push_back(&t);
		return *this;
	    }

	    ThreadsRunner& add(Thread<EOT>* t)
	    {
		_vt.push_back(t);
		return *this;
	    }

	    ThreadsRunner& addHandler(ThreadsHandler<EOT>& t)
	    {
		t.addTo(*this);
		return *this;
	    }

	private:
	    std::vector< Thread<EOT>* > _vt;
	};

#if __cplusplus > 199711L
	// namespace variadic
	// {
	//     template <class... Args>
	//     class ThreadsRunner;

	//     template <class... Args>
	//     class Thread
	//     {
	//     public:
	// 	virtual ~Thread() {}

	// 	virtual void operator()(Args...) = 0;

	//     private:
	// 	friend class ThreadsRunner<Args...>;

	// 	void build(Args... args)
	// 	{
	// 	    t = std::thread( std::ref(*this), std::ref(args)... );
	// 	}

	// 	inline void join() { t.join(); }

	//     protected:
	// 	std::thread t;
	//     };

	//     template <class... Args>
	//     class ThreadsHandler
	//     {
	//     public:
	// 	virtual ~ThreadsHandler() {}

	// 	virtual void addTo(ThreadsRunner<Args...>&) = 0;
	//     };

	//     template <class... Args>
	//     class ThreadsRunner
	//     {
	//     public:
	// 	void operator()(Args... args)
	// 	{
	// 	    for (auto& t : _vt) { t->build(args...); }
	// 	    for (auto& t : _vt) { t->join(); }
	// 	}

	// 	ThreadsRunner& add(Thread<Args...>& t)
	// 	{
	// 	    _vt.push_back(&t);
	// 	    return *this;
	// 	}

	// 	ThreadsRunner& add(Thread<Args...>* t)
	// 	{
	// 	    _vt.push_back(t);
	// 	    return *this;
	// 	}

	// 	ThreadsRunner& addHandler(ThreadsHandler<Args...>& t)
	// 	{
	// 	    t.addTo(*this);
	// 	    return *this;
	// 	}

	//     private:
	// 	std::vector< Thread<Args...>* > _vt;
	//     };
	// } // !variadic
#endif

    } // !core
} // !dim

#endif /* _CORE_THREAD_H_ */
