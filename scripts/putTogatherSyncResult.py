#!/usr/bin/env python3

import logging, sys
from parser import Parser
import pylab as pl;

logger = logging.getLogger("putTogatherSyncResult")

def main():
    parser = Parser(description='Put togather file results for multiprocessed DIM synchronious execution.')
    parser.add_argument('--prefix', '-p', help='monitor prefix name', default='result')
    parser.add_argument('--islands', '-n', help='number of islands', type=int, default=4)
    parser.add_argument('--trace', '-t', help='plot values', action='store_true')
    args = parser()

    files = []
    for i in range(args.islands):
        files += [open("%s_monitor_%d" % (args.prefix, i), 'r')]

    newfile = open("%s_monitor" % args.prefix, 'w')
    newfile.write("".join(open("%s/result_header.txt" % sys.path[0]).readlines()))

    # on ignore la premiere ligne
    for i in range(args.islands):
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
        for i in range(1,args.islands):
            ti = files[i].readline().split()
            newfile.write('%s ' % ' '.join(ti[2:]) )
            best += [float(ti[5])]
            nbindi += [float(ti[2])]
            avg += [float(ti[3]) * float(ti[2])]
        newfile.write('%d %d %d' % (max(best), sum(avg) / sum(nbindi), 0))
        newfile.write('\n')

        total += [nbindi]

    if trace:
        pl.plot(total)
        pl.show()

    logger.debug(total)
    logger.info("Done")

# when executed, just run main():
if __name__ == '__main__':
    logger.debug('### script started ###')
    main()
    logger.debug('### script ended ###')
