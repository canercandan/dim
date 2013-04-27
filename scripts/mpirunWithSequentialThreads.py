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

import subprocess
import logging
from parser import Parser

logger = logging.getLogger("mpirun")

def main():
    parser = Parser(description='Run mpi program with threads sequentialized.')
    parser.add_argument('program', nargs='+', help='the program arguments')
    parser.add_argument('-np', help='number of nodes used', type=int, default=4)
    parser.add_argument('-gdb', help='gdb mode', action='store_true')
    parser.add_argument('--gdb_cmd', help='gdb command to use', default='xterm -e gdb --args')
    parser.add_argument('--hwloc_cmd', help='hwloc command to use', default='hwloc-bind pu:')
    parser.add_argument('--mpi_cmd', help='mpi command to use', default='-np 1')
    args = parser()

    logger.debug(args)

    cmd = 'mpirun %s' % ' : '.join(["%s %s%d %s%s" % (args.mpi_cmd, args.hwloc_cmd, i, '%s ' % args.gdb_cmd if args.gdb else '', ' '.join(args.program)) for i in range(args.np)])

    logger.debug(cmd)

    try:
        subprocess.call(cmd.split())
    except KeyboardInterrupt:
        print("Interrupted by user.")

# when executed, just run main():
if __name__ == '__main__':
    logger.debug('### script started ###')
    main()
    logger.debug('### script ended ###')
