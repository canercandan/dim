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

import dim

class OneMaxFullEval(dim.FullEval):
    def __call__(self, ind):
        ind.set_fitness(sum(ind))

class OneMaxIncrEval(dim.PartialEval):
    def __call__(self, ind, index):
        ind.set_fitness( ind.fitness() + (-1 if ind[index] else 1) )

class DetBitFlip(dim.MonOp):
    def __init__(self, nbits=1):
        self.nbits = nbits

    def __str__(self): return '%s(%d)' % (self.__class__.__name__, self.nbits)

    def __call__(self, ind):
        if not ind.size(): return
        if ind.size() < self.nbits:
            raise ValueError('nbits is smaller than the size of the solution')

        selected = []

        for i in range(self.nbits):
            tmp = None

            while True:
                tmp = dim.random.randint(0, ind.size()-1)
                if tmp not in selected:
                    break

            selected += [tmp]

        for i in selected:
            ind[i] = not ind[i]

class BitMutation(dim.MonOp):
    def __init__(self, rate=0.01, normalize=False):
        self.rate = rate
        self.normalize = normalize

    def __str__(self): return 'BitMutation(%s,%s)' % (self.rate, self.normalize)

    def __call__(self, ind):
        p = self.rate / ind.size() if self.normalize else self.rate
        for i in range(ind.size()):
            if dim.random.random() < p:
                ind[i] = not ind[i]

class OneBitFlip(DetBitFlip):
    def __init__(self):
        DetBitFlip.__init__(self, 1)

logger = dim.logging.getLogger("dim")

def main():
    parser = dim.Parser(description='The python version of the Dynamic Islands Model for OneMax problem.', verbose='error')
    parser.add_argument('--nislands', '-N', help='number of islands', type=int, default=4)
    parser.add_argument('--popSize', '-P', help='size of population', type=int, default=100)
    parser.add_argument('--chromSize', '-n', help='size of problem', type=int, default=1000)
    parser.add_argument('--targetFitness', '-T', help='optimum to reach (0: disable)', type=int)
    parser.add_argument('--maxGen', '-G', help='maximum number of generation (0: disable)', type=int, default=0)
    parser.add_argument('--alpha', '-a', help='the alpha parameter of the learning process', type=float, default=.2)
    parser.add_argument('--beta', '-b', help='the beta parameter of the learning process', type=float, default=.01)
    parser.add_argument('--strategy', '-S', choices=['best', 'avg'], help='set a strategy of rewarding', default='best')
    parser.add_argument('--best', '-B', action='store_const', const='best', dest='strategy', help='best strategy (see -S)')
    parser.add_argument('--avg', '-A', action='store_const', const='avg', dest='strategy', help='average strategy (see -S)')
    args = parser()

    N = args.nislands
    P = args.popSize
    n = args.chromSize
    T = args.targetFitness
    G = args.maxGen

    init = dim.ZerofyInit(n)
    init_matrix = dim.InitMatrix(False,1/N)
    __eval = OneMaxFullEval()

    reward = None
    if args.strategy == 'avg':
        reward = dim.Proportional(args.alpha,args.beta)
    else:
        reward = dim.Best(args.alpha, args.beta)

    updater = dim.Updater(reward)
    memorizer = dim.Memorize()

    matrix = []
    for i in range(N):
        matrix += [[]]
        for j in range(N):
            matrix[-1] += [0]
    init_matrix(matrix)

    pop_set = []
    data_set = []

    for i in range(N):
        pop_set += [dim.Population(P,init)]
        data_set += [dim.IslandData(i,N)]

    islands = []
    for i in range(N):
        pop = pop_set[i]
        data = data_set[i]

        data.proba = matrix[i].copy()
        dim.apply(pop, __eval)

        mon = None
        if data.rank == 0:
            mon = BitMutation(1, True)
        else:
            mon = DetBitFlip( (data.rank-1)*2+1 )

        evolver = dim.Evolver(__eval, mon)
        feedbacker = dim.Feedbacker(pop_set, data_set)
        migrator = dim.Migrator(pop_set, data_set)
        fit = dim.Fit(T if T else n)
        cont = dim.Combined(fit)
        if G:
            maxgen = dim.MaxGen(G)
            cont.add(maxgen)
        checkpoint = dim.Checkpoint(cont)
        island = dim.Algo(evolver, feedbacker, updater, memorizer,
                          migrator, checkpoint, pop_set, data_set)
        t = dim.Thread(target=island, args=[pop,data])
        islands += [t]

    for island in islands:
        island.start()

    try:
        for island in islands:
            island.join()
    except KeyboardInterrupt as e:
        print("Interrupted by user")

# when executed, just run main():
if __name__ == '__main__':
    logger.debug('### script started ###')
    main()
    logger.debug('### script ended ###')
