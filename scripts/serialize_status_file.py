#!/usr/bin/env python3

import sys, re, json
from pprint import pprint

def load(f):
    m = {}

    for l in f.readlines():
        s = re.search('--(?P<key>\w+)\=(?P<value>\S+)', l)
        if s:
            key, value = s.group('key'), s.group('value')
            if key: m[key] = value

    return m

if __name__ == '__main__':
    if len(sys.argv) < 2: sys.exit(-1)

    fn = sys.argv[1]
    m = load(open(fn))
    json.dump(m, open('%s.json' % fn, 'w'))
