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

import sys, subprocess
import argparse, logging
from pprint import pprint

logger = logging.getLogger("mpirun")

def main():
    parser = argparse.ArgumentParser(description='Run mpi program with threads sequentialized.')

    parser.add_argument('program', nargs='+', help='the program arguments')
    parser.add_argument('-np', help='number of nodes used', type=int, required=True)
    parser.add_argument('-gdb', help='gdb mode', action='store_true')

    args = parser.parse_args()
    cmd = 'mpirun %s' % ' : '.join(["-np 1 hwloc-bind pu:%d %s%s" % (i, 'xterm -e gdb --args ' if args.gdb else '', ' '.join(args.program)) for i in range(args.np)])

    # print(cmd)

    try:
        subprocess.call(cmd.split())
    except KeyboardInterrupt:
        print("Interrupted by user.")

# when executed, just run main():
if __name__ == '__main__':
    logger.debug('### script started ###')
    main()
    logger.debug('### script ended ###')
