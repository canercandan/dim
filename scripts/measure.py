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

def parse_data(args):
    data = [None]*args.islands
    try:
        for i in range(args.islands):
            island = {}
            for f in args.files:
                fn = '%s%s%s%d' % (args.prefix, f, args.suffix, i)
                d = [int(x)*timeunits[args.time]/1000 for x in open(fn).readline().split()]
                m = np.mean(d);
                island[f] = {'mean': m, 'data': d}

            if args.percent:
                s = sum([island[f]['mean'] for f in args.percent_files])
                for f in args.percent_files:
                    island[f]['percent'] = (island[f]['mean']/s) * 100

            data[i] = island
    except FileNotFoundError as e:
        logger.error('File not found: %s' % e)
        sys.exit()
    return data

def print_data(args, data):
    print("Time unit: %s" % args.time)

    print("%10s " % " ", end=' ')
    for i in range(args.islands):
        if args.percent and args.percentMean:
            print("%17d" % i, end=' ')
        else:
            print("%12d" % i, end=' ')
    print()

    for f in args.files:
        print("%10s:" % f, end=' ')
        for i in range(args.islands):
            if args.percent and f not in ['gen', 'total']:
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

    for i in range(args.islands):
        pl.plot(data[i][args.traceFile]['data'][::args.affinity])

    pl.legend(["island %d" % i for i in range(args.islands)])
    pl.xlabel( '%s%s' % (args.xlabel, ' (affinity: %s)' % args.affinity if args.affinity > 1 else '') )
    pl.ylabel('%s (%s)' % (args.ylabel, args.time))
    pl.grid(args.no_grid)
    pl.title(args.traceFile)

    if args.show:
        pl.show()
    else:
        pl.savefig(args.outputFile)

def main():
    parser = Parser(description='To measure execution time for each part of the algorithm of DIM.', verbose='error')
    parser.add_argument('--islands', '-n', help='number of islands', type=int, default=4)
    parser.add_argument('--files', '-f', help='list of files prefixes to display, separated by comma', default='gen,evolve,feedback,update,memorize,migrate')
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
    parser.add_argument('--traceFile', help='plot the defined file', default='gen')
    parser.add_argument('--affinity', '-a', help='plot the defined file', type=int, default=1)
    parser.add_argument('--outputFile', '-O', help='output file where the figure will be saved', default='output.png')
    parser.add_argument('--show', '-S', action='store_true', help='plot instead creating a new file')
    parser.add_argument('--xlabel', help='label for x-axe', default='generations')
    parser.add_argument('--ylabel', help='label for y-axe', default='time')
    parser.add_argument('--no-grid', action='store_false', help='dont trace grid')
    args = parser()

    args.files = args.files.split(',')
    args.percent_files = args.percent_files.split(',')

    data = parse_data(args)
    print_data(args, data)
    trace_data(args, data)

# when executed, just run main():
if __name__ == '__main__':
    logger.debug('### script started ###')
    main()
    logger.debug('### script ended ###')
