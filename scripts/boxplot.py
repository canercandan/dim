#!/usr/bin/env python3
# http://stackoverflow.com/questions/14952401/creating-double-boxplots-i-e-two-boxes-for-each-x-value

import sys, os
import pylab as pl
import numpy as np

if __name__ == '__main__':
    if len(sys.argv) < 5:
        print("Usage:", os.path.basename(sys.argv[0]), "[MONITOR_PREFIX_NAME] [NUMBER_OF_ISLANDS] [NUMBER_OF_TIMES] [NUMBER_OF_RUNS]")
        sys.exit()

    prefix = sys.argv[1]
    Ni = int(sys.argv[2])
    Nt = int(sys.argv[3])
    Nr = int(sys.argv[4])

    V = []
    for i in range(Ni):
        files = []
        for r in range(Nr):
            fn = "%s_%d_monitor_%d" % (prefix, r+1, i)
            print("Pre-opening of run files…", fn)
            f = open(fn)
            files += [(fn, f)]
            f.readline() # to ignore first line

        T = []
        for t in range(Nt):
            R = []
            for r in range(Nr):
                fn, f = files[r]
                nbindi = int(f.readline().split()[2])
                print("Reading…", fn, "with time…", t, "and nbindi", nbindi)
                R += [nbindi]
            T += [R]

        V += [T]

    colors = ['green', 'black', 'orange', 'red', 'blue', 'gray', 'yellow']
    space = .15
    pos = np.linspace(Ni/2*space, -Ni/2*space, Ni).T

    for i in range(Ni):
        r = pl.boxplot(V[i], positions=[x-pos[i] for x in range(Nt)], widths=0.1)
        for value in r.values(): pl.setp(value, color=colors[i%len(colors)])

    pl.legend(["isl %s (%s)" % (i, colors[i]) for i in range(Ni)])
    pl.xlabel('Time')
    pl.ylabel('Nb individuals')
    pl.xticks([x for x in range(Nt)])
    pl.xlim(-Ni/2*space*2, (Nt-1)+Ni/2*space*2)
    pl.show()
