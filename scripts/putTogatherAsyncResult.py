#!/usr/bin/env python3

#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2
# as published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
#
# Authors:
# Caner Candan <caner@candan.fr>, http://caner.candan.fr
#

import logging, sys
from parser import Parser
import numpy as np
import math

logger = logging.getLogger("putTogatherAsyncResult")

def getTimeMax(datafiles):
    """
    Compute the maximum seconds used by the set of islands.
    """

    max_values = []

    for data in datafiles:
        if data:
            max_values += [float(data[-1][1])]

    return round(max(max_values))

def createTimelines(timemax, datafiles):
    """
    Create the timelines.
    """

    timelines = [None]*len(datafiles)
    for i in range(len(datafiles)):
        timelines[i] = [None]*(timemax+1)

    for i in range(len(datafiles)):
        for line in datafiles[i]:
            time = math.floor(float(line[1]))
            timelines[i][time] = line[2:]
    return timelines

def getTimelineInfo(timeline, N):
    """
    Parse the info data provided by a timeline and return.
    """

    if not timeline: return None

    info = {}
    info['nb'], info['avg'], info['delta'], info['best'], info['input'], info['output'] = timeline[:6]
    info['probas'] = [ float(x) for x in timeline[6:6+N] ]
    info['input_proba'] = float(timeline[6+N])
    info['migrants'] = [ int(x) for x in timeline[6+N+1:] ]

    convertToInt = ['nb', 'input', 'output',]
    convertToFloat = ['avg', 'delta', 'best']

    for key in convertToInt: info[key] = int(info[key])
    for key in convertToFloat: info[key] = float(info[key])

    return info

def writeInfoToResult(info, newfile):
    """
    Write down info to the new result file.
    """

    newfile.write('%(nb)s %(avg)s %(delta)s %(best)s %(input)s %(output)s ' % info)
    for val in info['probas']: newfile.write("%s " % val)
    newfile.write('%(input_proba)s ' % info)
    for val in info['migrants']: newfile.write("%s " % val)

def main():
    parser = Parser(description='Put togather multiprocessed DIM execution file results.')
    parser.add_argument('--prefix', '-p', help='monitor prefix name', default='result')
    parser.add_argument('--islands', '-n', help='number of islands', type=int, default=4)
    args = parser()

    datafiles = [None]*args.islands
    for i in range(args.islands):
        datafiles[i] = [ line.split() for line in open("%s_monitor_%d" % (args.prefix, i), 'r').readlines()[1:] ]

    newfile = open("%s_monitor" % args.prefix, 'w')
    newfile.write("".join(open("%s/result_header.txt" % sys.path[0]).readlines()))

    timemax = getTimeMax(datafiles)
    logger.info("Timemax:", timemax)

    timelines = createTimelines(timemax, datafiles)

    for time in range(timemax):
        newfile.write('%s ' % time)

        best = []
        nbindi = []
        avg = []
        evl = []

        for i in range(args.islands):
            info = getTimelineInfo(timelines[i][time], args.islands)

            if not info: continue
            logger.debug(info)

            writeInfoToResult(info, newfile)

            best += [info['best']]
            nbindi += [info['nb']]
            avg += [info['avg'] * info['nb']]

        newfile.write( '%d %d %d' % (max(best), sum(avg) / sum(nbindi), 0) )

        newfile.write('\n')

    logger.info("Done")

# when executed, just run main():
if __name__ == '__main__':
    logger.debug('### script started ###')
    main()
    logger.debug('### script ended ###')
