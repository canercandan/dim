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
from collections import OrderedDict
import sys, time as t

timeunits = OrderedDict([('seconds', 10**0), ('milliseconds', 10**3), ('microseconds', 10**6), ('nanoseconds', 10**9),])

defaults = {
    'prefix': 'result',
    'islands': 4,
    'trace': False,
    'traceAgragatedDataField': None,
    'traceIslandsField': None,
    'time_idx': 1,
    'start_idx': 2,
    'inputFields': 'migration,nbindi,sending_queue_size,receiving_queue_size,avg,delta,best,input,output,probas,probasum,inputproba,migrants',
    'outputFields': 'nbindi,avg,delta,best,input,output,probas,inputproba,migrants',
    'agregatedFields': 'best,nbindi,avg/nbindi,eval',
    'agregatedFunctions': 'max,sum,avg,none',
    'timemin': None,
    'timemax': None,
    'timeunit': 'seconds',
    'timeinterval': 1,
    'header': False,
    'detailedFields': False,
    'printTimelines': False,
    'writeToFile': False,
    'respawn': False,
}

class ResultsMixer:
    def __init__(self, *args, **kwargs):
        self.kwargs = defaults.copy()
        self.kwargs.update(kwargs)

        for k in self.kwargs.keys():
            if k in defaults:
                self.__setattr__(k, self.kwargs[k])

        self.openFiles()

    def openFiles(self):
        self.files = []
        try:
            for i in range(self.islands):
                self.files += [open("%s_monitor_%d" % (self.prefix, i), 'r')]
                self.files[-1].readline() # just to skip first line
        except FileNotFoundError as e:
            print('File not found: %s' % e)
            sys.exit()

    def getTimeMax(self, datafiles):
        """
        Compute the maximum seconds used by the set of islands.
        """

        values = []

        # walk through the island result files and get the time of the last record in order to get the maximum
        for data in datafiles:
            if data:
                values += [float(data[-1][self.time_idx])]

        # round to convert to integer
        return round(max(values)) if values else 0

    def getTimeMin(self, datafiles):
        """
        Compute the minimum seconds used by the set of islands.
        """

        values = []

        # walk through the island result files and get the time of the first record in order to get the minimum
        for data in datafiles:
            if data:
                values += [float(data[0][self.time_idx])]

        # round to convert to integer
        return round(min(values)) if values else 0

    def createTimelines(self, timemin, timemax, datafiles):
        """
        Create the timelines.
        """

        length = timemax - timemin

        # create a list of N lists
        timelines = []
        for i in range(self.islands):
            timelines += [{}]

        for data, timeline in zip(datafiles, timelines):
            for line in data:
                time = floor(float(line[self.time_idx]) * timeunits[self.timeunit])
                time = round(time / self.timeinterval) * self.timeinterval
                if time > timemax: break
                timeline[time] = line[self.start_idx:]

        return timelines

    def fillEmptyTimelines(self, timemin, timemax, timelines):
        """
        Fill out empty record line with the previous ones
        """

        def findLastLine(timeline):
            for time in range(timemin, timemax, self.timeinterval):
                if time in timeline:
                    return timeline[time]
            return None

        for timeline in timelines:
            lastline = findLastLine(timeline)
            for time in range(timemin, timemax, self.timeinterval):
                if time in timeline:
                    lastline = timeline[time].copy()
                else:
                    if lastline:
                        timeline[time] = lastline.copy()

    def getTimelineInfo(self, timeline):
        """
        Parse the data fields provided by a timeline and return it as an information.
        """

        if not timeline: return None

        fields = self.inputFields.split(',')

        def cutData(name):
            """Get the list information at the position of the given keyname as parameter and remove it from the timeline list"""

            class List(list):
                """Wrapper changing print behavior"""
                def __str__(self): return ' '.join([str(x) for x in self])

            pos = fields.index(name)
            data = List(timeline[pos:pos+self.islands])
            del timeline[pos:pos+self.islands]
            del fields[pos]
            return data

        info = {}

        info['probas'] = cutData('probas')
        info['migrants'] = cutData('migrants')

        info.update( dict(zip(fields, timeline)) )

        for key in ['nbindi', 'input', 'output',]: info[key] = int(info[key])
        for key in ['avg', 'delta', 'best',]: info[key] = float(info[key])

        return info

    def writeInfoToResult(self, info, newfile):
        """
        Write down info to the new result file.
        """

        fields = self.outputFields.split(',')
        for name in fields:
            if self.detailedFields:
                newfile.write('%s(%s) ' % (name, info[name]))
            else:
                newfile.write('%s ' % info[name])

    def writeTimeline(self, infos, newfile, time, agregatedCollectToTrace, collectToTrace):
        if self.detailedFields:
            newfile.write('time(%s) ' % time)
        else:
            newfile.write('%s ' % time)

        fields = self.agregatedFields.split(',')
        functions = self.agregatedFunctions.split(',')

        agregatedInfos = {}
        metaAgregatedInfos = {}

        for name, function in zip(fields, functions):
            if function in ['avg']:
                metaAgregatedInfos[name] = []
            agregatedInfos[name] = []

        for i, info in zip(range(self.islands), infos):
            self.writeInfoToResult(info, newfile)

            if self.traceIslandsField:
                collectToTrace[i] += [info[self.traceIslandsField]]

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
            if self.detailedFields:
                newfile.write('%s_%s(%s) ' % (function, name, agregatedInfos[name]))
            else:
                newfile.write('%s ' % agregatedInfos[name])
        newfile.write('\n')

        if self.traceAgragatedDataField:
            agregatedCollectToTrace += [ agregatedInfos[self.traceAgragatedDataField] ]

    def parse(self):
        # forward all the contents of result files to a two dimentionals table with island*stats records
        datafiles = []

        if not self.respawn:
            for i in range(self.islands):
                datafiles += [self.files[i].readlines()]
        else:
            reopen = True
            while reopen:
                reopen = False
                for i in range(self.islands):
                    datafile = self.files[i].readlines()

                    if not datafile:
                        self.openFiles()
                        datafile = []
                        reopen = True
                        break

                    datafiles += [datafile]

        for i in range(self.islands):
            datafiles[i] = [ line.split() for line in datafiles[i] ]

        # create the new file
        newfile = None
        if self.writeToFile:
            newfile = open("%s_monitor" % self.prefix, 'w')

        # add a common header for future use
        if self.header and self.writeToFile:
            newfile.write("".join(open("%s/result_header.txt" % sys.path[0]).readlines()))

        # fixed or estimated timemax computed with timeunit
        timemin = (self.timemin if self.timemin else self.getTimeMin(datafiles) * timeunits[self.timeunit])
        timemax = (self.timemax if self.timemax else self.getTimeMax(datafiles) * timeunits[self.timeunit])

        # create a table with a size to timemax and added stat record to the dedicated timeline
        timelines = self.createTimelines(timemin, timemax, datafiles)

        # fill out the empty timeline thanks to the strategy used for (duplicate previous timeline)
        self.fillEmptyTimelines(timemin, timemax, timelines)

        agregatedCollectToTrace = [] # to trace results

        collectToTrace = []
        for i in range(self.islands): collectToTrace += [ [] ]

        structuredTimelines = []

        # walk through the timeline until timemax with a fixed interval
        for time in range(timemin, timemax, self.timeinterval):
            # get the parsed info from the N islands result for one specific timeline
            # if one island result is wrong we skip this timeline
            infos = []
            ok = True
            for i in range(self.islands):
                info = self.getTimelineInfo(timelines[i][time])

                if not info:
                    ok = False
                    break

                infos += [info]

            # skip if info is not complete
            if not ok:
                print("not ok")
                continue

            if self.printTimelines or not self.writeToFile:
                structuredTimelines += [infos]
            else:
                self.writeTimeline(infos, newfile, time, agregatedCollectToTrace, collectToTrace)

        if self.trace:
            import pylab as pl

            if self.traceAgragatedDataField:
                pl.plot(agregatedCollectToTrace)
            elif self.traceIslandsField:
                pl.plot(collectToTrace)
            pl.show()

        if self.printTimelines and self.writeToFile:
            newfile.write("%s" % structuredTimelines)

        if not self.writeToFile:
            return structuredTimelines
        else:
            return None

    def __call__(self):
        while True: yield self.parse()

logger = logging.getLogger("ResultsMixer")

def main():
    parser = Parser(description='Put togather file results for multiprocessed DIM synchronious execution.')
    parser.add_argument('--prefix', '-p', help='monitor prefix name', default='result')
    parser.add_argument('--islands', '-n', help='number of islands', type=int, default=4)
    parser.add_argument('--trace', '-P', help='plot values', action='store_true')
    parser.add_argument('--traceAgragatedDataField', help='select the agregated data field to trace (see -P)')
    parser.add_argument('--traceIslandsField', help='select the islands data field to trace (see -P)')
    parser.add_argument('--time_idx', type=int, default=1, help='time index')
    parser.add_argument('--start_idx', type=int, default=2, help='dump data started by this position')
    parser.add_argument('--inputFields', '-f', default='migration,nbindi,sending_queue_size,receiving_queue_size,avg,delta,best,input,output,probas,probasum,inputproba,migrants', help='fields order of the input result files')
    parser.add_argument('--outputFields', '-F', default='nbindi,avg,delta,best,input,output,probas,inputproba,migrants', help='fields order of the output result file')
    parser.add_argument('--agregatedFields', '-a', default='best,nbindi,avg/nbindi,eval', help='agregated fields order of the output result file')
    parser.add_argument('--agregatedFunctions', '-A', default='max,sum,avg,none', help='agregated functions order of the output result file')
    parser.add_argument('--timemin', type=int, help='fix the timemin value instead estimating it automagically')
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

    rm = ResultsMixer(**vars(args))

    import time

    for data in rm():
        print(data)
        time.sleep(1)

# when executed, just run main():
if __name__ == '__main__':
    logger.debug('### script started ###')
    main()
    logger.debug('### script ended ###')
