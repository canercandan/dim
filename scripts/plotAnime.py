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
import matplotlib.animation as animation
import results_mixer
import time

logger = logging.getLogger("plotAnim")

class DataParser:
    def __init__(self, args):
        self.args = args
        self.rm = results_mixer.ResultsMixer(**vars(args))
        self.lastValues = {}
        for t in self.args.topics:
            self.lastValues[t] = [0]*self.args.islands

    def create_sumrank(self, data, key, values, record):
        record[key] = []
        maxVal, index = max((x,i) for i,x in enumerate(values))
        for i in range(self.args.islands):
            val = self.lastValues[key][i] + (1 if i == index else 0)
            self.lastValues[key][i] = val
            record[key] += [val]

    def parse(self):
        data = self.rm.parse()
        if not data: return []

        selectedData = []

        for infos in data:
            record = {}

            # attractiveness
            tmp = []
            for i in range(self.args.islands):
                inputProbas = []
                for j in range(self.args.islands):
                    inputProbas += [float(infos[j]['probas'][i])]
                tmp += [sum(inputProbas)]

            if 'attractiveness_sumrank' in self.args.topics:
                self.create_sumrank(selectedData, 'attractiveness_sumrank', tmp, record)

            if 'attractiveness' in self.args.topics:
                record['attractiveness'] = tmp

            # field
            values = [float(info[self.args.field]) for info in infos]

            if 'field_sumrank' in self.args.topics:
                self.create_sumrank(selectedData, 'field_sumrank', values, record)

            if 'field' in self.args.topics:
                record['field'] = values

            selectedData += [record]

        return selectedData

    def __call__(self):
        while True: yield self.parse()

class DataUpdater:
    def __init__(self, args):
        self.args = args

        self.fig = pl.figure()

        self.axes = {}
        self.lines = {}

        for k, t in enumerate(self.args.topics):
            ax = None
            if len(self.args.topics) == 1:
                ax = self.fig.add_subplot(111)
            else:
                ax = self.fig.add_subplot(2,len(self.args.topics)/2,k+1)

            line = []
            for i in range(args.islands):
                l, = ax.plot([0]*args.scale)
                line += [l]

            ax.set_ylim(0, args.yscale)
            ax.set_title(t)

            ax.set_xlabel( args.xlabel )
            ax.set_ylabel( args.ylabel )
            ax.grid(args.no_grid)

            self.axes[t] = ax
            self.lines[t] = line

        self.fig.legend(self.lines[self.args.topics[0]], loc='upper left', labels=['island %d' % i for i in range(args.islands)])

        self.dp = DataParser(args)
        self.anim = animation.FuncAnimation(self.fig, self, self.dp, interval=args.interval)

        pl.show()

    def __call__(self, data):
        if not data: return

        for t in self.args.topics:
            for i in range(self.args.islands):
                d = [ l[t][i] for l in data[::self.args.affinity] ]

                if not self.args.scale:
                    self.lines[t][i].set_xdata(range(len(self.lines[t][i].get_xdata()) + len(d)))
                    self.lines[t][i].set_ydata(np.append(self.lines[t][i].get_ydata(), d))
                    self.axes[t].relim()
                    self.axes[t].autoscale()
                    self.axes[t].set_ylim(0, self.args.yscale)
                else:
                    self.lines[t][i].set_ydata(np.append(self.lines[t][i].get_ydata(), d)[-self.args.scale:])

def main():
    parser = Parser(description='To trace data.')
    parser.add_argument('--islands', '-n', help='number of islands', type=int, default=4)
    parser.add_argument('--field', '-f', help='field to trace', default='nbindi')
    parser.add_argument('--xlabel', help='label for x-axe', default='time (in seconds)')
    parser.add_argument('--ylabel', help='label for y-axe', default='y')
    parser.add_argument('--affinity', '-a', help='plot the defined file', type=int, default=1)
    parser.add_argument('--no-grid', action='store_false', help='dont trace grid')
    parser.add_argument('--scale', type=int, default=100, help='scale of animation view (0 means dynamic)')
    parser.add_argument('--yscale', type=int, default=50, help='scale of animation view for y-axe')
    parser.add_argument('--interval', type=int, default=100, help='interval data of animation view')
    parser.add_argument('--timeunit', '-t', choices=[x for x in results_mixer.timeunits.keys()], help='select a time unit', default='seconds')
    parser.add_argument('--timeinterval', '-I', type=int, help='interval of times between two records based on the selected time unit (see -t)', default=1)
    parser.add_argument('--respawn', '-R', action='store_true', help='plot instead creating a new file')
    parser.add_argument('--topics', '-T', help='select stats to display', default='attractiveness_sumrank,attractiveness,field_sumrank,field')

    args = parser()
    logger.debug(args)

    args.topics = args.topics.split(',')

    du = DataUpdater(args)

# when executed, just run main():
if __name__ == '__main__':
    logger.debug('### script started ###')
    main()
    logger.debug('### script ended ###')
