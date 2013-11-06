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

#ifndef _UTILS_MEASURE_H_
#define _UTILS_MEASURE_H_

#ifdef MEASURE

#if __cplusplus > 199711L
	namespace std_or_boost = std;
#else
	namespace std_or_boost = boost;
#endif

# define DO_MEASURE(op, measureFiles, name)				\
    {									\
	std_or_boost::chrono::time_point< std_or_boost::chrono::system_clock > start = std_or_boost::chrono::system_clock::now(); \
	op;								\
	std_or_boost::chrono::time_point< std_or_boost::chrono::system_clock > end = std_or_boost::chrono::system_clock::now();	\
	unsigned elapsed = std_or_boost::chrono::duration_cast<std_or_boost::chrono::microseconds>(end-start).count(); \
	*(measureFiles[name]) << elapsed << std::endl; measureFiles[name]->flush(); \
    }

#else

# define DO_MEASURE(op, measureFiles, name) { op; }

#endif // !MEASURE

#endif // !_UTILS_MEASURE_H_
