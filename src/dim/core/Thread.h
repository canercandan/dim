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

#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>

namespace dim
{
    namespace core
    {

	class ThreadsRunner;

	class Thread
	{
	public:
	    virtual ~Thread() {}

	private:
	    friend class ThreadsRunner;

	    virtual void operator()() = 0;

	    void build(std::condition_variable& cv, std::mutex& m);
	    void join();

	protected:
	    std::thread t;
	    std::condition_variable* pt_cv = nullptr;
	    std::mutex* pt_m = nullptr;
	};

	class BlockingThread : public Thread
	{
	public:
	    BlockingThread(size_t __id = 0) : id(__id) {}

	private:
	    void operator()() override;

	protected:
	    size_t id = 0;
	};

	class NonBlockingThread : public Thread
	{
	private:
	    virtual void operator()() override;
	};

	class Compute : public NonBlockingThread
	{
	private:
	    void operator()() override;
	};

	class Communicate : public BlockingThread
	{
	private:
	    void operator()() override;
	};

	class ThreadsRunner
	{
	public:
	    void operator()();
	    ThreadsRunner& add(Thread& t);

	private:
	    std::vector<Thread*> _vt;
	    std::condition_variable _cv;
	    std::mutex _m;
	};

    } // !core
} // !dim

#endif /* _CORE_THREAD_H_ */
