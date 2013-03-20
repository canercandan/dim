#!/usr/bin/env python3

import sys, os
import pylab as pl;

if __name__ == '__main__':
    if len(sys.argv) < 3:
        print("Usage:", os.path.basename(sys.argv[0]), "[MONITOR_PREFIX_NAME] [NUMBER_OF_ISLAND]")
        sys.exit()

    prefix = sys.argv[1]
    N = int(sys.argv[2])
    exec_path = sys.path[0]

    files = []
    for i in range(N):
        files += [open("%s_monitor_%d" % (prefix, i), 'r')]

    newfile = open("%s_monitor" % prefix, 'w')
    newfile.write("".join(open("%s/result_header.txt" % exec_path).readlines()))

    # on ignore la premiere ligne
    for i in range(N):
        files[i].readline()

    total = []

    for line in files[0]:
        t0 = line.split()
        newfile.write('%s %s ' % (t0[1], ' '.join(t0[2:]) ) )
        best = []
        nbindi = []
        avg = []
        evl = []
        best += [float(t0[5])]
        nbindi += [float(t0[2])]
        avg += [float(t0[3]) * float(t0[2])]
        for i in range(1,N):
            ti = files[i].readline().split()
            newfile.write('%s ' % ' '.join(ti[2:]) )
            best += [float(ti[5])]
            nbindi += [float(ti[2])]
            avg += [float(ti[3]) * float(ti[2])]
        newfile.write('%d %d %d' % (max(best), sum(avg) / sum(nbindi), 0))
        newfile.write('\n')

        total += [nbindi]

    pl.plot(total)
    pl.show()

    # print(total)
    print("Done")