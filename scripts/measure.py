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

import logging
from parser import Parser
import numpy as np
from collections import OrderedDict
import sys

logger = logging.getLogger("measure")
timeunits = OrderedDict([('seconds', 10**0), ('milliseconds', 10**3), ('microseconds', 10**6), ('nanoseconds', 10**9),])

class DataParser:
    def __init__(self, args):
        self.args = args

        self.openFiles()

    def openFiles(self):
        self.files = []
        try:
            for i in range(self.args.islands):
                island = {}
                for f in self.args.files:
                    fn = '%s%s%s%d' % (self.args.prefix, f, self.args.suffix, i)
                    island[f] = open(fn)
                self.files += [island]
        except FileNotFoundError as e:
            logger.error('File not found: %s' % e)
            sys.exit()

    def parse(self):
        data = []
        reopen = True

        while reopen:
            reopen = False
            for i in range(self.args.islands):
                island = {}
                for f in self.args.files:
                    island[f] = self.files[i][f].readlines()

                    if not island[f]:
                        self.openFiles()
                        data = []
                        reopen = True
                        break

                if reopen: break

                data += [island]

        for i in range(self.args.islands):
            for f in self.args.files:
                d = [int(x)*timeunits[self.args.time]/10**6 for x in data[i][f]]
                m = np.mean(d) if d else 0;
                data[i][f] = {'mean': m, 'data': d}

            if self.args.percent:
                s = sum([island[f]['mean'] for f in self.args.percent_files])
                for f in self.args.percent_files:
                    data[i][f]['percent'] = (data[i][f]['mean']/s) * 100

        return data

    def __call__(self):
        while True: yield self.parse()

class DataUpdater:
    def __init__(self, args):
        if not args.animate: return
        import pylab as pl
        import matplotlib.animation as animation

        self.args = args

        self.fig = pl.figure()

        self.axes = []
        self.lines = []

        for i in range(args.islands):
            ax = self.fig.add_subplot(2,args.islands/2,i+1)

            line = {}
            for f in args.files:
                line[f], = ax.plot([0]*args.scale)

            ax.set_ylim(0, args.yscale)
            ax.set_title('island %d' % i)

            ax.set_xlabel( '%s%s' % (args.xlabel, ' (affinity: %s)' % args.affinity if args.affinity > 1 else '') )
            ax.set_ylabel('%s (%s)' % (args.ylabel, args.time))
            ax.grid(args.no_grid)
            # ax.set_title('%s %s' % (args.selectFile, args.title))

            self.axes += [ax]
            self.lines += [line]

        self.fig.legend([self.lines[0][f] for f in args.files], loc='upper left', labels=args.files)

        self.dp = DataParser(args)
        self.anim = animation.FuncAnimation(self.fig, self, self.dp, interval=args.interval)

        pl.show()

    def __call__(self, data):
        if not data: return

        logger.debug(data)

        for i in range(self.args.islands):
            for f in self.args.files:
                d = data[i][f]['data'][::self.args.affinity]

                if not self.args.scale:
                    self.lines[i][f].set_xdata(range(len(self.lines[i][f].get_xdata()) + len(d)))
                    self.lines[i][f].set_ydata(np.append(self.lines[i][f].get_ydata(), d))
                    self.axes[i].relim()
                    self.axes[i].autoscale()
                    self.axes[i].set_ylim(0, self.args.yscale)
                else:
                    self.lines[i][f].set_ydata(np.append(self.lines[i][f].get_ydata(), d)[-self.args.scale:])

def print_data(args, data):
    if args.clear:
        from os import system
        system('clear')
        # print(chr(27) + "[2J")

    print("Time unit: %s" % args.time)

    print("%10s " % " ", end=' ')
    for i in range(args.islands):
        if args.percent and args.percentMean:
            print("%17d" % i, end=' ')
        else:
            print("%12d" % i, end=' ')
    print()

    notpercent = []
    for f in args.files:
        if f not in args.percent_files:
            notpercent += [f]

    for f in args.files:
        print("%10s:" % f, end=' ')
        for i in range(args.islands):
            if args.percent and f not in notpercent:
                if args.percentMean:
                    print( '%(mean)6.2f (%(percent)6.2f %%)' % data[i][f], end=' ' )
                else:
                    print( '%(percent)10.2f %%' % data[i][f], end=' ' )
            else:
                if args.percent and args.percentMean:
                    print( '%(mean)17.2f' % data[i][f], end=' ' )
                else:
                    print( '%(mean)12.2f' % data[i][f], end=' ' )
        print()

def trace_data(args, data):
    if not args.trace: return
    import pylab as pl

    if not args.traceFiles:
        for i in range(args.islands):
            pl.plot(data[i][args.selectFile]['data'][::args.affinity])
        pl.legend(["island %d" % i for i in range(args.islands)])
    else:
        for f in args.files:
            pl.plot(data[args.selectIsland][f]['data'][::args.affinity])
        pl.legend(args.files)

    pl.xlabel( '%s%s' % (args.xlabel, ' (affinity: %s)' % args.affinity if args.affinity > 1 else '') )
    pl.ylabel('%s (%s)' % (args.ylabel, args.time))
    pl.grid(args.no_grid)
    pl.title('%s %s' % (args.selectFile, args.title))

    if args.show:
        pl.show()
    else:
        pl.savefig(args.outputFile)

def main():
    parser = Parser(description='To measure execution time for each part of the algorithm of DIM.', verbose='error')
    parser.add_argument('--islands', '-n', help='number of islands', type=int, default=4)
    parser.add_argument('--files', '-f', help='list of files prefixes to display, separated by comma', default='gen_sync,evolve,feedback,update,memorize,migrate')
    parser.add_argument('--prefix', help='set the prefix time files', default='result.')
    parser.add_argument('--suffix', help='set the suffix time files', default='.time.')
    parser.add_argument('--time', '-t', choices=[x for x in timeunits.keys()], help='select a time unit', default='milliseconds')
    parser.add_argument('--seconds', '-s', action='store_const', const='seconds', dest='time', help='time in seconds')
    parser.add_argument('--milliseconds', '-m', action='store_const', const='milliseconds', dest='time', help='time in milliseconds')
    parser.add_argument('--microseconds', '-u', action='store_const', const='microseconds', dest='time', help='time in microseconds')
    parser.add_argument('--nanoseconds', '-N', action='store_const', const='nanoseconds', dest='time', help='time in nanoseconds')
    parser.add_argument('--percent', '-p', action='store_true', help='use percents')
    parser.add_argument('--percentMean', action='store_true', help='use percents + means')
    parser.add_argument('--percent_files', default='evolve,feedback,update,memorize,migrate', help='used fields to compute percents')
    parser.add_argument('--trace', '-P', help='plot measures', action='store_true')
    parser.add_argument('--animate', '-A', help='animate measures', action='store_true')
    parser.add_argument('--scale', type=int, default=100, help='scale of animation view (0 means dynamic)')
    parser.add_argument('--yscale', type=int, default=50, help='scale of animation view for y-axe')
    parser.add_argument('--interval', type=int, default=100, help='interval data of animation view')
    parser.add_argument('--selectFile', help='plot the defined file', default='gen_sync')
    parser.add_argument('--selectIsland', help='plot the defined island', type=int, default=0)
    parser.add_argument('--traceFiles', help='plot the whole of defined files instead islands (see -f and --selectIsland)', action='store_true')
    parser.add_argument('--affinity', '-a', help='plot the defined file', type=int, default=1)
    parser.add_argument('--outputFile', '-O', help='output file where the figure will be saved', default='output.png')
    parser.add_argument('--show', '-S', action='store_true', help='plot instead creating a new file')
    parser.add_argument('--clear', '-C', action='store_true', help='clear console screen for each update')
    parser.add_argument('--xlabel', help='label for x-axe', default='generations')
    parser.add_argument('--ylabel', help='label for y-axe', default='time')
    parser.add_argument('--title', help='title', default='')
    parser.add_argument('--no-grid', action='store_false', help='dont trace grid')
    args = parser()

    args.files = args.files.split(',')
    args.percent_files = args.percent_files.split(',')

    if args.animate:
        du = DataUpdater(args)
    else:
        dp = DataParser(args)
        data = dp.parse()
        print_data(args, data)
        trace_data(args, data)

# when executed, just run main():
if __name__ == '__main__':
    logger.debug('### script started ###')
    main()
    logger.debug('### script ended ###')
