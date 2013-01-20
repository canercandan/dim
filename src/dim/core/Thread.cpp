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

#include <iostream>
#include <chrono>
#include <stdexcept>

#include "Thread.h"

namespace dim
{
    namespace core
    {

	void Thread::build(std::condition_variable& cv, std::mutex& m)
	{
	    pt_cv = &cv;
	    pt_m = &m;
	    t = std::thread([&]() { (*this)(); });
	}

	void Thread::join()
	{
	    if (pt_cv == nullptr)
		{
		    throw std::runtime_error("condition variable is a nullptr");
		}
	    if (pt_m == nullptr)
		{
		    throw std::runtime_error("mutex variable is a nullptr");
		}
	    t.join();
	}

	void BlockingThread::operator()()
	{
	    std::unique_lock<std::mutex> lk(*pt_m);
	    while (1)
		{
		    pt_cv->wait(lk);
		    std::cout << "b" << id << " "; std::cout.flush();
		}
	}

	void NonBlockingThread::operator()()
	{
	    while (1)
		{
		    std::cout << "nb "; std::cout.flush();
		    pt_cv->notify_one();
		    // std::this_thread::sleep_for(std::chrono::microseconds(1));
		}
	}

	void Compute::operator()()
	{
	    while (1)
		{
		    std::cout << "c "; std::cout.flush();
		    pt_cv->notify_one();
		    // std::this_thread::sleep_for(std::chrono::microseconds(1));
		}
	}

	void Communicate::operator()()
	{
	    std::unique_lock<std::mutex> lk(*pt_m);
	    while (1)
		{
		    pt_cv->wait(lk);
		    std::cout << "C "; std::cout.flush();
		}
	}

	void ThreadsRunner::operator()()
	{
	    for (auto& t : _vt) { t->build(_cv, _m); }
	    for (auto& t : _vt) { t->join(); }
	}

	ThreadsRunner& ThreadsRunner::add(Thread& t)
	{
	    _vt.push_back(&t);
	    return *this;
	}

    } // !core
} // !dim
