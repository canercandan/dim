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

logger = logging.getLogger("measure")

def main():
    times = {'seconds': 1e0, 'milliseconds': 1e3, 'microseconds': 1e6, 'nanoseconds': 1e9,}
    parser = Parser(description='To measure execution time for each part of the algorithm of DIM.')
    parser.add_argument('--islands', '-n', help='number of islands', type=int, default=4)
    parser.add_argument('--files', '-f', help='list of files prefixes to display, separated by comma', default='gen,evolve,feedback,update,memorize,migrate')
    parser.add_argument('--time', '-t', choices=[x for x in times.keys()], help='select a time unit', default='seconds')
    parser.add_argument('--seconds', '-s', action='store_const', const='seconds', dest='time', help='time in seconds')
    parser.add_argument('--milliseconds', '-m', action='store_const', const='milliseconds', dest='time', help='time in milliseconds')
    parser.add_argument('--microseconds', '-u', action='store_const', const='microseconds', dest='time', help='time in microseconds')
    parser.add_argument('--nanoseconds', '-N', action='store_const', const='nanoseconds', dest='time', help='time in nanoseconds')
    args = parser()

    print(args)

    print("Time unit: %s" % args.time)

    print("%10s " % " ", end=' ')
    for i in range(args.islands):
        print("%10d" % i, end=' ')
    print()

    for f in args.files.split(','):
        print("%10s:" % f, end=' ')
        for i in range(args.islands):
            fn = '%s.time.%d' % (f, i)
            d = open(fn).readline().split()
            d = [int(x) for x in d]
            print( '%10.2f' % (np.mean(d)/1000), end=' ' )
        print()

# when executed, just run main():
if __name__ == '__main__':
    logger.debug('### script started ###')
    main()
    logger.debug('### script ended ###')
