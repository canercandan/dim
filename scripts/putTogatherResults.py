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
from math import floor
from parser import Parser
import numpy as np

logger = logging.getLogger("putTogatherResults")
timeunits = {'seconds': 10**0, 'milliseconds': 10**3, 'microseconds': 10**6, 'nanoseconds': 10**9,}

def getTimeMax(args, datafiles):
    """
    Compute the maximum seconds used by the set of islands.
    """

    max_values = []

    # walk through the island result files and get the time of the last record in order to get the maximum
    for data in datafiles:
        if data:
            max_values += [float(data[-1][args.time_idx])]

    # round to convert to integer
    return round(max(max_values))

def createTimelines(args, datafiles):
    """
    Create the timelines.
    """

    # create a list of N lists
    timelines = [None]*args.islands
    for i in range(args.islands):
        timelines[i] = [None]*(args.timemax + 1)

    for data, timeline in zip(datafiles, timelines):
        for line in data:
            time = floor(float(line[args.time_idx]) * timeunits[args.timeunit])
            if time > args.timemax: break
            timeline[time] = line[args.start_idx:]

    return timelines

def fillEmptyTimelines(args, timelines):
    """
    Fill out empty record line with the previous ones
    """

    def findLastLine(timeline):
        for line in timeline:
            if line: return line
        return None

    for timeline in timelines:
        lastline = findLastLine(timeline)
        for i in range(len(timeline)):
            if timeline[i]:
                lastline = timeline[i].copy()
            else:
                if lastline:
                    timeline[i] = lastline.copy()
                # logger.debug(timeline[i])

def getTimelineInfo(args, timeline):
    """
    Parse the data fields provided by a timeline and return it as an information.
    """

    if not timeline: return None

    fields = args.inputFields.split(',')

    def cutData(name):
        """Get the list information at the position of the given keyname as parameter and remove it from the timeline list"""

        class List(list):
            """Wrapper changing print behavior"""
            def __str__(self): return ' '.join([str(x) for x in self])

        pos = fields.index(name)
        data = List(timeline[pos:pos+args.islands])
        del timeline[pos:pos+args.islands]
        del fields[pos]
        return data

    info = {}

    info['probas'] = cutData('probas')
    info['migrants'] = cutData('migrants')

    info.update( dict(zip(fields, timeline)) )

    for key in ['nbindi', 'input', 'output',]: info[key] = int(info[key])
    for key in ['avg', 'delta', 'best',]: info[key] = float(info[key])

    return info

def writeInfoToResult(args, info, newfile):
    """
    Write down info to the new result file.
    """

    fields = args.outputFields.split(',')
    for name in fields:
        if args.detailedFields:
            newfile.write('%s(%s) ' % (name, info[name]))
        else:
            newfile.write('%s ' % info[name])

def writeTimeline(args, infos, newfile, time, agregatedCollectToTrace, collectToTrace):
    if args.detailedFields:
        newfile.write('time(%s) ' % time)
    else:
        newfile.write('%s ' % time)

    fields = args.agregatedFields.split(',')
    functions = args.agregatedFunctions.split(',')

    agregatedInfos = {}
    metaAgregatedInfos = {}

    for name, function in zip(fields, functions):
        if function in ['avg']:
            metaAgregatedInfos[name] = []
        agregatedInfos[name] = []

    for i, info in zip(range(args.islands), infos):
        writeInfoToResult(args, info, newfile)

        if args.traceIslandsField:
            collectToTrace[i] += [info[args.traceIslandsField]]

        for name, function in zip(fields, functions):
            if function in ['max', 'sum']:
                agregatedInfos[name] += [info[name]]
            elif function in ['avg']:
                x,w = name.split('/')
                metaAgregatedInfos[name] += [info[w]]
                agregatedInfos[name] += [ info[x] * info[w] ]

    for name, function in zip(fields, functions):
        if function in ['max']:
            agregatedInfos[name] = max(agregatedInfos[name])
        elif function in ['sum']:
            agregatedInfos[name] = sum(agregatedInfos[name])
        elif function in ['avg']:
            agregatedInfos[name] = round(sum(agregatedInfos[name]) / sum(metaAgregatedInfos[name]), 2) if sum(metaAgregatedInfos[name]) else 0
        else:
            agregatedInfos[name] = 0

    for name, function in zip(fields, functions):
        if args.detailedFields:
            newfile.write('%s_%s(%s) ' % (function, name, agregatedInfos[name]))
        else:
            newfile.write('%s ' % agregatedInfos[name])
    newfile.write('\n')

    if args.traceAgragatedDataField:
        agregatedCollectToTrace += [ agregatedInfos[args.traceAgragatedDataField] ]

def main():
    parser = Parser(description='Put togather file results for multiprocessed DIM synchronious execution.')
    parser.add_argument('--prefix', '-p', help='monitor prefix name', default='result')
    parser.add_argument('--islands', '-n', help='number of islands', type=int, default=4)
    parser.add_argument('--trace', '-P', help='plot values', action='store_true')
    parser.add_argument('--traceAgragatedDataField', help='select the agregated data field to trace (see -P)')
    parser.add_argument('--traceIslandsField', help='select the islands data field to trace (see -P)')
    parser.add_argument('--time_idx', type=int, default=1, help='time index')
    parser.add_argument('--start_idx', type=int, default=2, help='dump data started by this position')
    parser.add_argument('--inputFields', '-f', default='nbindi,avg,delta,best,input,output,probas,probasum,inputproba,migrants', help='fields order of the input result files')
    parser.add_argument('--outputFields', '-F', default='nbindi,avg,delta,best,input,output,probas,inputproba,migrants', help='fields order of the output result file')
    parser.add_argument('--agregatedFields', '-a', default='best,nbindi,avg/nbindi,eval', help='agregated fields order of the output result file')
    parser.add_argument('--agregatedFunctions', '-A', default='max,sum,avg,none', help='agregated functions order of the output result file')
    parser.add_argument('--timemax', '-T', type=int, help='fix the timemax value instead estimating it automagically')
    parser.add_argument('--timeunit', '-t', choices=[x for x in timeunits.keys()], help='select a time unit', default='seconds')
    parser.add_argument('--timeinterval', '-I', type=int, help='interval of times between two records based on the selected time unit (see -t)', default=1)
    parser.add_argument('--seconds', '-s', action='store_const', const='seconds', dest='timeunit', help='timeunit in seconds')
    parser.add_argument('--milliseconds', '-m', action='store_const', const='milliseconds', dest='timeunit', help='timeunit in milliseconds')
    parser.add_argument('--microseconds', '-u', action='store_const', const='microseconds', dest='timeunit', help='timeunit in microseconds')
    parser.add_argument('--nanoseconds', '-N', action='store_const', const='nanoseconds', dest='timeunit', help='timeunit in nanoseconds')
    parser.add_argument('--header', '-H', action='store_true', help='add the common header to the output file')
    parser.add_argument('--detailedFields', '-D', action='store_true', help='add the name of each field into the result file')
    parser.add_argument('--printTimelines', '-L', action='store_true', help='print timelines using python syntax instead the default behavior')
    args = parser()

    # forward all the contents of result files to a two dimentionals table with island*stats records
    datafiles = [None]*args.islands
    for i in range(args.islands):
        datafiles[i] = [ line.split() for line in open("%s_monitor_%d" % (args.prefix, i), 'r').readlines()[1:] ]

    # create the new file
    newfile = open("%s_monitor" % args.prefix, 'w')

    # add a common header for future use
    if args.header:
        newfile.write("".join(open("%s/result_header.txt" % sys.path[0]).readlines()))

    # fixed or estimated timemax computed with timeunit
    args.timemax = (args.timemax if args.timemax else getTimeMax(args, datafiles) * timeunits[args.timeunit])

    logger.info("Timemax: %d %s" % (args.timemax, args.timeunit))

    # create a table with a size to timemax and added stat record to the dedicated timeline
    timelines = createTimelines(args, datafiles)

    # fill out the empty timeline thanks to the strategy used for (duplicate previous timeline)
    fillEmptyTimelines(args, timelines)

    agregatedCollectToTrace = [] # to trace results

    collectToTrace = [None]*args.islands
    for i in range(args.islands): collectToTrace[i] = []

    structuredTimelines = []

    # walk through the timeline until timemax with a fixed interval
    for time in range(0, args.timemax, args.timeinterval):
        # get the parsed info from the N islands result for one specific timeline
        # if one island result is wrong we skip this timeline
        infos = []
        ok = True
        for i in range(args.islands):
            info = getTimelineInfo(args, timelines[i][time])

            if not info:
                ok = False
                break

            infos += [info]

        # skip if info is not complete
        if not ok: continue

        if args.printTimelines:
            structuredTimelines += [infos]
        else:
            writeTimeline(args, infos, newfile, time, agregatedCollectToTrace, collectToTrace)

    if args.trace:
        import pylab as pl

        if args.traceAgragatedDataField:
            # logger.debug(agregatedCollectToTrace)
            pl.plot(agregatedCollectToTrace)
        elif args.traceIslandsField:
            # logger.debug(collectToTrace)
            pl.plot(collectToTrace)
        pl.show()

    if args.printTimelines:
        newfile.write("%s" % structuredTimelines)

    logger.info("Done")

# when executed, just run main():
if __name__ == '__main__':
    logger.debug('### script started ###')
    main()
    logger.debug('### script ended ###')
