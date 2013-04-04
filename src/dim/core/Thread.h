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

#if __cplusplus > 199711L
#include <thread>
// #include <mutex>
// #include <condition_variable>
#else
#include <boost/thread.hpp>
// #include <boost/thread/mutex.hpp>
// #include <boost/thread/condition_variable.hpp>
#endif

#include <vector>

namespace dim
{
    namespace core
    {
#if __cplusplus > 199711L
	namespace std_or_boost = std;
#else
	namespace std_or_boost = boost;
#endif

#if __cplusplus > 199711L
	template <class... Args>
#else
	template <typename EOT>
#endif
	class ThreadsRunner;

#if __cplusplus > 199711L
	template <class... Args>
#else
	template <typename EOT>
#endif
	class Thread
	{
	public:
	    virtual ~Thread() {}

#if __cplusplus > 199711L
	    virtual void operator()(Args...) = 0;
#else
	    virtual void operator()(Pop<EOT>&, IslandData<EOT>&) = 0;
#endif

	private:
#if __cplusplus > 199711L
	    friend class ThreadsRunner<Args...>;
#else
	    friend class ThreadsRunner<EOT>;
#endif

#if __cplusplus > 199711L
	    void build(Args... args)
#else
	    void build(Pop<EOT>& pop, IslandData<EOT>& data)
#endif
	    {
#if __cplusplus > 199711L
		t = std_or_boost::thread( std_or_boost::ref(*this), std_or_boost::ref(args)... );
#else
		t = std_or_boost::thread( std_or_boost::ref(*this), std_or_boost::ref(pop), std_or_boost::ref(data) );
#endif
	    }

	    inline void join() { t.join(); }

	protected:
	    std_or_boost::thread t;
	};

#if __cplusplus > 199711L
	template <class... Args>
#else
	template <typename EOT>
#endif
	class ThreadsHandler
	{
	public:
	    virtual ~ThreadsHandler() {}

#if __cplusplus > 199711L
	    virtual void addTo(ThreadsRunner<Args...>&) = 0;
#else
	    virtual void addTo(ThreadsRunner<EOT>&) = 0;
#endif
	};

#if __cplusplus > 199711L
	template <class... Args>
#else
	template <typename EOT>
#endif
	class ThreadsRunner
	{
	public:
#if __cplusplus > 199711L
	    void operator()(Args... args)
#else
	    void operator()(Pop<EOT>& pop, IslandData<EOT>& data)
#endif
	    {
#if __cplusplus > 199711L
		for (auto& t : _vt) { t->build(args...); }
		for (auto& t : _vt) { t->join(); }
#else
		for (size_t i = 0; i < _vt.size(); ++i) { _vt[i]->build(pop, data); }
		for (size_t i = 0; i < _vt.size(); ++i) { _vt[i]->join(); }
#endif
	    }

#if __cplusplus > 199711L
	    ThreadsRunner& add(Thread<Args...>& t)
#else
	    ThreadsRunner& add(Thread<EOT>& t)
#endif
	    {
		_vt.push_back(&t);
		return *this;
	    }

#if __cplusplus > 199711L
	    ThreadsRunner& add(Thread<Args...>* t)
#else
	    ThreadsRunner& add(Thread<EOT>* t)
#endif
	    {
		_vt.push_back(t);
		return *this;
	    }

#if __cplusplus > 199711L
	    ThreadsRunner& addHandler(ThreadsHandler<Args...>& t)
#else
	    ThreadsRunner& addHandler(ThreadsHandler<EOT>& t)
#endif
	    {
		t.addTo(*this);
		return *this;
	    }

	private:
#if __cplusplus > 199711L
	    std::vector< Thread<Args...>* > _vt;
#else
	    std::vector< Thread<EOT>* > _vt;
#endif
	};
    } // !core
} // !dim

#endif /* _CORE_THREAD_H_ */
