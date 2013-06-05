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

import logging
from parser import Parser
import pylab as pl
import numpy as np
import results_mixer

logger = logging.getLogger("plot")

def main():
    parser = Parser(description='To trace data.')
    parser.add_argument('--islands', '-n', help='number of islands', type=int, default=4)
    parser.add_argument('--field', '-f', help='field to trace', default='nbindi')
    parser.add_argument('--attractiveness', '-a', action='store_true', help='instead tracing a field, trace the attractiveness thanks to coming probability')
    parser.add_argument('--sumrank', '-r', action='store_true', help='trace the cumulate of sum of times the islands lead')
    parser.add_argument('--xlabel', help='label for x-axe', default='time (in seconds)')
    parser.add_argument('--ylabel', help='label for y-axe', default='number of individuals in waiting')
    parser.add_argument('--ylabelAttractiveness', help='label for y-axe if -a is enabled', default='attractiveness')
    parser.add_argument('--ylabelSumrank', help='prefix label for y-axe', default='sum of rank along with the')
    parser.add_argument('--outputFile', '-O', help='output file where the figure will be saved', default='output.png')
    parser.add_argument('--show', '-s', action='store_true', help='plot instead creating a new file')
    parser.add_argument('--smoothing', '-S', type=int, help='smooth the data set, give a level of smoothing (0 = no smoothing)', default=0)
    parser.add_argument('--no-marker', action='store_false', help='dont trace markers')
    parser.add_argument('--no-grid', action='store_false', help='dont trace grid')
    args = parser()

    logger.debug(args)

    rm = results_mixer.ResultsMixer(**vars(args))
    data = rm.parse()

    selectedData = []

    if args.sumrank: selectedData = [[0]*args.islands]

    for infos in data:
        record = []

        if args.attractiveness:
            tmp = []
            for i in range(args.islands):
                inputProbas = []
                for j in range(args.islands):
                    inputProbas += [float(infos[j]['probas'][i])]
                tmp += [sum(inputProbas)]

            if args.sumrank:
                # attractiveness and sumrank

                maxVal, index = max((x,i) for i,x in enumerate(tmp))
                for i in range(args.islands):
                    if (i == index):
                        record += [selectedData[-1][i] + 1]
                    else:
                        record += [selectedData[-1][i]]

                # END attractiveness and sumrank
            else:
                # attractiveness

                record = tmp

                # END attractiveness
        else:
            values = [float(info[args.field]) for info in infos]

            if args.sumrank:
                # trace field and sumrank

                maxVal, index = max((x,i) for i,x in enumerate(values))

                for i in range(args.islands):
                    if (i == index):
                        record += [selectedData[-1][i] + 1]
                    else:
                        record += [selectedData[-1][i]]

                # END trace field and sumrank
            else:
                # trace field

                record = values

                # END trace field

        selectedData += [record]

    # logger.debug(selectedData)

    if args.smoothing:
        DIV = args.smoothing*100
        xn = len(selectedData)
        XN = xn/DIV
        x = np.arange(XN)

        from scipy.interpolate import pchip

        colors = 'bgrcmykw'
        colors *= int(np.ceil(args.islands/len(colors)))

        for c, i in zip(colors, range(args.islands)):
            y = np.array([v[i] for v in selectedData][::DIV])
            interp = pchip(x, y)
            xx = np.linspace(0, XN, XN*100)
            pl.plot(xx, interp(xx), label='island %d' % i)
            if args.no_marker: pl.plot(x, y, c+'o')

        pl.legend()
        pl.xticks(x, np.arange(xn)[::int(DIV)])
        pl.xlim((0,XN))
    else:
        pl.plot(selectedData)
        pl.legend(["island %d" % i for i in range(args.islands)])

    pl.xlabel(args.xlabel)
    pl.ylabel("%s%s" % (args.ylabelSumrank + " " if args.sumrank else "",
                        args.ylabelAttractiveness if args.attractiveness else args.ylabel))
    pl.grid(args.no_grid)

    if args.show:
        pl.show()
    else:
        pl.savefig(args.outputFile)

# when executed, just run main():
if __name__ == '__main__':
    logger.debug('### script started ###')
    main()
    logger.debug('### script ended ###')
