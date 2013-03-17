#!/usr/bin/env python3

import numpy as np
import math
import sys, os
from pprint import pprint

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

    timelines = [None]*N
    for i in range(N):
        timelines[i] = [None]*(timemax+1)

    for i in range(N):
        for line in datafiles[i]:
            time = math.floor(float(line[1]))
            timelines[i][time] = line[2:]
    return timelines

def getTimelineInfo(timeline):
    """
    Parse the info data provided by a timeline and return.
    """

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

if __name__ == '__main__':
    if len(sys.argv) < 3:
        print("Usage:", os.path.basename(sys.argv[0]), "[MONITOR_PREFIX_NAME] [NUMBER_OF_ISLAND]")
        sys.exit()

    prefix = sys.argv[1]
    N = int(sys.argv[2])
    exec_path = sys.path[0]

    datafiles = [None]*N
    for i in range(N):
        datafiles[i] = [ line.split() for line in open("%s_monitor_%d" % (prefix, i), 'r').readlines()[1:] ]

    newfile = open("%s_monitor" % prefix, 'w')
    newfile.write("".join(open("%s/result_header.txt" % exec_path).readlines()))

    timemax = getTimeMax(datafiles)
    print("Timemax:", timemax)

    timelines = createTimelines(timemax, datafiles)

    for time in range(timemax):
        newfile.write('%s ' % time)

        best = []
        nbindi = []
        avg = []
        evl = []

        for i in range(N):
            info = getTimelineInfo(timelines[i][time])
            # pprint(info)
            writeInfoToResult(info, newfile)

            best += [info['best']]
            nbindi += [info['nb']]
            avg += [info['avg'] * info['nb']]

        newfile.write( '%d %d %d' % (max(best), sum(avg) / sum(nbindi), 0) )

        newfile.write('\n')

    print("Done")
