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
    parser.add_argument('-mca', help='modular component architectur modules', default='btl self,sm,tcp')
    parser.add_argument('--gdb_cmd', help='gdb command to use', default='xterm -e gdb --args')
    parser.add_argument('--hwloc_cmd', help='hwloc command to use', default='hwloc-bind pu:')
    parser.add_argument('--mpi_cmd', help='mpi command to use', default='-np 1')
    parser.add_argument('--test', '-t', action='store_true', help='just display the wrapped command line without executing it')
    args = parser()

    args.gdb = '%(gdb_cmd)s ' % args.__dict__ if args.gdb else ''
    args.mca = '-mca \'%(mca)s\' ' % args.__dict__ if args.mca else ''
    args.program = ' '.join(args.program)

    parameters = ["%(mpi_cmd)s %(mca)s %(hwloc_cmd)s%(i)d %(gdb)s %(program)s" % args.__dict__ for args.__dict__['i'] in range(args.np)]

    cmd = 'mpirun %s' % ' : '.join(parameters)

    if args.test:
        print(cmd)
        return

    try:
        subprocess.call(cmd.split())
    except KeyboardInterrupt:
        print("Interrupted by user.")

# when executed, just run main():
if __name__ == '__main__':
    logger.debug('### script started ###')
    main()
    logger.debug('### script ended ###')
