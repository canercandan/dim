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

import logging, sys, json
from parser import Parser
import numpy as np
from collections import OrderedDict

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

        if not self.args.respawn:
            for i in range(self.args.islands):
                island = {}
                for f in self.args.files:
                    island[f] = self.files[i][f].readlines()
                data += [island]
        else:
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

        # record mean value of files
        for i in range(self.args.islands):
            for f in self.args.files:
                d = [int(x)*timeunits[self.args.time]/10**6 for x in data[i][f]]
                m = np.mean(d) if d else 0;
                data[i][f] = {'value': m, 'data': d}

        # set percent for wished fields
        for i in range(self.args.islands):
            if self.args.percent:
                s = sum([data[i][f]['value'] for f in self.args.percentFields])
                for f in self.args.percentFields:
                    data[i][f]['percent'] = (data[i][f]['value']/s) * 100

        try:
            with open(self.args.json_file) as f:
                new_data = json.load(f)
                for i in range(self.args.islands):
                    for key in new_data:
                        data[i][key] = {'value': new_data[key]}
                    # data[i].update(new_data)
        except FileNotFoundError:
            pass

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
    tabular = 17 if args.percent and args.percentValue else 12

    if args.clear:
        from os import system
        system('clear')
        # print(chr(27) + "[2J")

    print("Time unit: %s" % args.time)

    print("%%%ds " % args.tabulation_size % " ", end=' ')
    for i in range(args.islands):
        print("%%%dd" % tabular % i, end=' ')
    print()

    notpercent = []
    for f in args.fields:
        if f not in args.percentFields:
            notpercent += [f]

    for f in args.fields:
        print("%%%ds:" % args.tabulation_size % f, end=' ')
        if f in args.files:
            for i in range(args.islands):
                if args.percent and f not in notpercent:
                    if args.percentValue:
                        print( '%(value)6.2f (%(percent)6.2f %%)' % data[i][f], end=' ' )
                    else:
                        print( '%(percent)10.2f %%' % data[i][f], end=' ' )
                else:
                    print( '%%(value)%d.2f' % tabular % data[i][f], end=' ' )
        else:
            print( '%%(value)%ds' % tabular % data[i][f], end=' ' )
        print()

    for f in args.agregateFields:
        print("%%%ds:" % args.tabulation_size % f, end=' ')
        for op, op_func in [('+', lambda a,b: a+b), ('-', lambda a,b: a-b), ('*', lambda a,b: a*b), ('/', lambda a,b: a/b)]:
            if op in f:
                op1,op2 = f.split(op)
                for i in range(args.islands):
                    try:
                        op1_value = int(op1)
                    except ValueError:
                        op1_value = float(data[i][op1]['value'])

                    try:
                        op2_value = int(op2)
                    except ValueError:
                        op2_value = float(data[i][op2]['value'])

                    print( '%12.2f' % op_func(op1_value, op2_value), end=' ' )
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
    parser.add_argument('--files', '-f', help='list of files prefixes to process, separated by comma', default='gen_sync,evolve,feedback,update,memorize,migrate,feedback_wait,migrate_wait')
    parser.add_argument('--fields', '-F', help='list of fields to display, separated by comma', default='gen_sync,evolve,feedback,update,memorize,migrate,feedback_wait,migrate_wait')
    parser.add_argument('--agregateFields', '-g', help='list of pair of fields to compute, separated by comma', default='feedback-feedback_wait,migrate-migrate_wait')
    parser.add_argument('--json_file', '-j', help='set a json status file with further data to use with other fields', default='tsp.status.json')
    parser.add_argument('--tabulation_size', '-T', help='tabulation size for file names', type=int, default=10)
    parser.add_argument('--prefix', help='set the prefix time files', default='result.')
    parser.add_argument('--suffix', help='set the suffix time files', default='.time.')
    parser.add_argument('--time', '-t', choices=[x for x in timeunits.keys()], help='select a time unit', default='milliseconds')
    parser.add_argument('--seconds', '-s', action='store_const', const='seconds', dest='time', help='time in seconds')
    parser.add_argument('--milliseconds', '-m', action='store_const', const='milliseconds', dest='time', help='time in milliseconds')
    parser.add_argument('--microseconds', '-u', action='store_const', const='microseconds', dest='time', help='time in microseconds')
    parser.add_argument('--nanoseconds', '-N', action='store_const', const='nanoseconds', dest='time', help='time in nanoseconds')
    parser.add_argument('--percent', '-p', action='store_true', help='use percents')
    parser.add_argument('--percentValue', action='store_true', help='use percents + values')
    parser.add_argument('--percentFields', default='evolve,feedback,update,memorize,migrate', help='used fields to compute percents')
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
    parser.add_argument('--respawn', '-R', action='store_true', help='plot instead creating a new file')
    args = parser()

    args.files = args.files.split(',')
    args.fields = args.fields.split(',')
    if args.agregateFields:
        args.agregateFields = args.agregateFields.split(',')
    args.percentFields = args.percentFields.split(',')

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
