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
                fn = '%s%s%d' % (f, args.suffix, i)
                d = [int(x) for x in open(fn).readline().split()]
                island[f] = np.mean(d)*timeunits[args.time]/1000

            if args.percent:
                s = sum([island[f] for f in args.percent_files])
                for f in args.percent_files:
                    island[f] = (island[f]/s) * 100

            data[i] = island
    except FileNotFoundError as e:
        logger.error('File not found: %s' % e)
        sys.exit()
    return data

def print_data(args, data):
    print("Time unit: %s" % args.time)

    print("%10s " % " ", end=' ')
    for i in range(args.islands):
        print("%12d" % i, end=' ')
    print()

    for f in args.files:
        print("%10s:" % f, end=' ')
        for i in range(args.islands):
            if args.percent and f not in ['gen', 'total']:
                print( '%10.2f %%' % data[i][f], end=' ' )
            else:
                print( '%12.2f' % data[i][f], end=' ' )
        print()

def main():
    parser = Parser(description='To measure execution time for each part of the algorithm of DIM.', verbose='error')
    parser.add_argument('--islands', '-n', help='number of islands', type=int, default=4)
    parser.add_argument('--files', '-f', help='list of files prefixes to display, separated by comma', default='gen,evolve,feedback,update,memorize,migrate')
    parser.add_argument('--suffix', help='set the suffix time files', default='.time.')
    parser.add_argument('--time', '-t', choices=[x for x in timeunits.keys()], help='select a time unit', default='milliseconds')
    parser.add_argument('--seconds', '-s', action='store_const', const='seconds', dest='time', help='time in seconds')
    parser.add_argument('--milliseconds', '-m', action='store_const', const='milliseconds', dest='time', help='time in milliseconds')
    parser.add_argument('--microseconds', '-u', action='store_const', const='microseconds', dest='time', help='time in microseconds')
    parser.add_argument('--nanoseconds', '-N', action='store_const', const='nanoseconds', dest='time', help='time in nanoseconds')
    parser.add_argument('--percent', '-p', action='store_true', help='use percents')
    parser.add_argument('--percent_files', default='evolve,feedback,update,memorize,migrate', help='used fields to compute percents')
    args = parser()

    args.files = args.files.split(',')
    args.percent_files = args.percent_files.split(',')

    data = parse_data(args)
    print_data(args, data)

# when executed, just run main():
if __name__ == '__main__':
    logger.debug('### script started ###')
    main()
    logger.debug('### script ended ###')
