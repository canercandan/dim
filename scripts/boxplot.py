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

# http://stackoverflow.com/questions/14952401/creating-double-boxplots-i-e-two-boxes-for-each-x-value

import sys, os
import argparse, logging
import pylab as pl
import numpy as np

logger = logging.getLogger("boxplot")

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='To trace boxplot.', formatter_class=argparse.ArgumentDefaultsHelpFormatter)

    parser.add_argument('--prefix', '-p', help='monitor prefix name', default='result')
    parser.add_argument('--islands', '-N', help='number of islands', type=int, default=4)
    parser.add_argument('--times', '-T', help='number of times', type=int, default=10000)
    parser.add_argument('--runs', '-R', help='number of runs', type=int, default=21)

    # if len(sys.argv) < 5:
    #     print("Usage:", os.path.basename(sys.argv[0]), "[MONITOR_PREFIX_NAME] [NUMBER_OF_ISLANDS] [NUMBER_OF_TIMES] [NUMBER_OF_RUNS]")
    #     sys.exit()

    args = parser.parse_args()

    # prefix = sys.argv[1]
    # Ni = int(sys.argv[2])
    # Nt = int(sys.argv[3])
    # Nr = int(sys.argv[4])

    V = []
    for i in range(args.islands):
        files = []
        for r in range(args.runs):
            fn = "%s_%d_monitor_%d" % (args.prefix, r+1, i)
            print("Pre-opening of run files…", fn)
            f = open(fn)
            files += [(fn, f)]
            f.readline() # to ignore first line

        T = []
        for t in range(args.times):
            R = []
            for r in range(args.runs):
                fn, f = files[r]
                nbindi = int(f.readline().split()[2])
                print("Reading…", fn, "with time…", t, "and nbindi", nbindi)
                R += [nbindi]
            T += [R]

        V += [T]

    colors = ['green', 'black', 'orange', 'red', 'blue', 'gray', 'yellow']
    space = .15
    pos = np.linspace(args.islands/2*space, -args.islands/2*space, args.islands).T

    for i in range(args.islands):
        r = pl.boxplot(V[i], positions=[x-pos[i] for x in range(args.times)], widths=0.1)
        for value in r.values(): pl.setp(value, color=colors[i%len(colors)])

    pl.legend(["isl %s (%s)" % (i, colors[i]) for i in range(args.islands)])
    pl.xlabel('Time')
    pl.ylabel('Nb individuals')
    pl.xticks([x for x in range(args.times)])
    pl.xlim(-args.islands/2*space*2, (args.times-1)+args.islands/2*space*2)
    pl.show()
