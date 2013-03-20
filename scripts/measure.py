#!/usr/bin/env python3

import numpy as np
import sys, os

FILES = ['gen', 'evolve', 'feedback', 'update', 'memorize', 'migrate']

if __name__ == '__main__':
    if len(sys.argv) < 2:
        print("Usage:", os.path.basename(sys.argv[0]), "[NUMBER_OF_ISLAND]")
        sys.exit()

    print("Time unit: second")

    N = int(sys.argv[1])

    print("%10s " % " ", end=' ')
    for i in range(N):
        print("%10d" % i, end=' ')
    print()

    for f in FILES:
        print("%10s:" % f, end=' ')
        for i in range(N):
            fn = '%s.time.%d' % (f, i)
            d = open(fn).readline().split()
            d = [int(x) for x in d]
            print( '%10.2f' % (np.mean(d)/1000), end=' ' )
        print()
