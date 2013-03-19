#!/usr/bin/env python3

import sys, os

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

    for line in files[0]:
        t0 = line.split()
        newfile.write('%s %s ' % (t0[1], ' '.join(t0[2:]) ) )
        best = []
        nbindi = []
        avg = []
        evl = []
        best += [float(t0[6])]
        nbindi += [float(t0[3])]
        avg += [float(t0[4]) * float(t0[3])]
        for i in range(1,4):
            ti = files[i].readline().split()
            newfile.write('%s ' % ' '.join(ti[2:]) )
            best += [float(ti[6])]
            nbindi += [float(ti[3])]
            avg += [float(ti[4]) * float(ti[3])]
        newfile.write('%d %d %d' % (max(best), sum(avg) / sum(nbindi), 0))
        newfile.write('\n')

    print("Done")
