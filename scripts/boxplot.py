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

import argparse, logging, sys

logger = logging.getLogger("boxplot")

class Parser(argparse.ArgumentParser):
    """Wrapper class added logging support"""

    def __init__(self, description='', formatter_class=argparse.ArgumentDefaultsHelpFormatter):
        """
        We add all the common options to manage verbosity.
        """

        argparse.ArgumentParser.__init__(self, description=description, formatter_class=formatter_class)

        self.levels = {'debug': logging.DEBUG,
                       'info': logging.INFO,
                       'warning': logging.WARNING,
                       'error': logging.ERROR,
                       'quiet': logging.CRITICAL
        }

        self.add_argument('-v', '--verbose', choices=[x for x in self.levels.keys()], default='quiet', help='set a verbosity level')
        self.add_argument('-l', '--levels', action='store_true', default=False, help='list all the verbosity levels')
        self.add_argument('-o', '--output', help='all the logging messages are redirected to the specified filename.')
        self.add_argument('-d', '--debug', action='store_const', const='debug', dest='verbose', help='Diplay all the messages.')
        self.add_argument('-i', '--info', action='store_const', const='info', dest='verbose', help='Diplay the info messages.')
        self.add_argument('-w', '--warning', action='store_const', const='warning', dest='verbose', help='Only diplay the warning and error messages.')
        self.add_argument('-e', '--error', action='store_const', const='error', dest='verbose', help='Only diplay the error messages')
        self.add_argument('-q', '--quiet', action='store_const', const='quiet', dest='verbose', help='Quiet level of verbosity only displaying the critical error messages.')

    def __call__(self):
        args = self.parse_args()

        if args.levels:
            print("Here's the verbose levels available:")
            for keys in self.levels.keys():
                print("\t", keys)
            sys.exit()

        if (args.output):
            logging.basicConfig(
                level=logging.DEBUG,
                format='%(asctime)s %(name)-12s %(levelname)-8s %(message)s',
                filename=args.output, filemode='a'
                )
            return

        logging.basicConfig(
            level=self.levels.get(args.verbose, logging.NOTSET),
            format='%(name)-12s: %(levelname)-8s %(message)s'
            )

        return args

def main():
    parser = Parser(description='To trace boxplot.')
    parser.add_argument('--prefix', '-p', help='monitor prefix name', default='result')
    parser.add_argument('--islands', '-n', help='number of islands', type=int, default=4)
    parser.add_argument('--times', '-t', help='number of times', type=int, default=30)
    parser.add_argument('--runs', '-r', help='number of runs', type=int, default=21)
    parser.add_argument('--column', '-c', help='column to select', type=int, default=2)
    parser.add_argument('--notplot', '-np', help='disable plotting', action='store_true')
    parser.add_argument('--space', '-s', help='padding between each boxplot', type=float, default=.15)
    parser.add_argument('--colors', '-C', help='colors used in order to color each boxplot, use a comma to add more than one color', default='green,black,orange,red,blue,gray,yellow')
    args = parser()

    V = []
    for i in range(args.islands):
        files = []

        if not args.runs:
            fn = "%s_monitor_%d" % (args.prefix, i)
            logger.info("Pre-opening of run files... %s" % fn)
            f = open(fn)
            files += [(fn, f)]
            f.readline() # to ignore first line
        else:
            for r in range(args.runs):
                fn = "%s_%d_monitor_%d" % (args.prefix, r+1, i)
                logger.info("Pre-opening of run files... %s" % fn)
                f = open(fn)
                files += [(fn, f)]
                f.readline() # to ignore first line

        T = []
        for t in range(args.times):
            R = []
            for fn, f in files:
                nbindi = int(f.readline().split()[args.column])
                logger.info( "Reading... %s with time... %d and nbindi %d" % (fn, t, nbindi) )
                R += [nbindi]
            T += [R]
        V += [T]

    logger.debug(V)

    if not args.notplot:
        import pylab as pl
        import numpy as np

        colors = args.colors.split(',')
        pos = np.linspace(args.islands/2*args.space, -args.islands/2*args.space, args.islands).T

        for i in range(args.islands):
            r = pl.boxplot(V[i], positions=[x-pos[i] for x in range(args.times)], widths=0.1)
            for value in r.values(): pl.setp(value, color=colors[i%len(colors)])

        pl.legend(["isl %s (%s)" % (i, colors[i]) for i in range(args.islands)])
        pl.xlabel('Time')
        pl.ylabel('Nb individuals')
        pl.xticks([x for x in range(args.times)])
        pl.xlim(-args.islands/2*args.space*2, (args.times-1)+args.islands/2*args.space*2)
        pl.show()

# when executed, just run main():
if __name__ == '__main__':
    logger.debug('### script started ###')
    main()
    logger.debug('### script ended ###')
