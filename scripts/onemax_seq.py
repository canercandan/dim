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
                tmp = dim.random.randint(0, ind.size())
                if tmp not in selected:
                    break

            selected += [tmp]

        for i in selected:
            ind[i] = not ind[i]

        return True

class BitMutation(dim.MonOp):
    def __init__(self, rate=0.01, normalize=False):
        self.rate = rate
        self.normalize = normalize

    def __str__(self): return 'BitMutation(%s,%s)' % (self.rate, self.normalize)

    def __call__(self, ind):
        p = self.rate / ind.size() if self.normalize else self.rate
        changed_something = False

        for i in range(ind.size()):
            r = dim.random.random()
            if r < p:
                ind[i] = not ind[i]
                changed_something = True

        return changed_something

class OneBitFlip(DetBitFlip):
    def __init__(self):
        DetBitFlip.__init__(self, 1)

logger = dim.logging.getLogger("dim")

def main():
    parser = dim.Parser(description='The python version of the Dynamic Islands Model for OneMax problem.', verbose='error')
    parser.add_argument('--nislands', '-N', help='number of islands', type=int, default=4)
    parser.add_argument('--popSize', '-P', help='size of population', type=int, default=100)
    parser.add_argument('--chromSize', '-n', help='size of problem', type=int, default=1000)
    parser.add_argument('--definedChromSize', help='size of problem set to True', type=int, default=0)
    parser.add_argument('--targetFitness', '-T', help='optimum to reach (0: disable)', type=int)
    parser.add_argument('--maxGen', '-G', help='maximum number of generation (0: disable)', type=int, default=0)
    parser.add_argument('--alpha', '-a', help='the alpha parameter of the learning process', type=float, default=.2)
    parser.add_argument('--alphaF', '-A', help='the alpha parameter of the learning process', type=float, default=.01)
    parser.add_argument('--beta', '-b', help='the beta parameter of the learning process', type=float, default=.01)
    parser.add_argument('--strategy', '-S', help='set a strategy of rewarding', default=dim.Best)
    parser.add_argument('--best', '-B', action='store_const', const=dim.Best, dest='strategy', help='best strategy (see -S)')
    parser.add_argument('--avg', '-M', action='store_const', const=dim.Average, dest='strategy', help='average strategy (see -S)')
    parser.add_argument('--stepTimer', help='step of time used for printing results to files', type=int, default=0)
    parser.add_argument('--seed', help='set the seed value for the generator of pseudo-random numbers', type=int, default=0)
    args = parser()

    if args.seed:
        dim.random.seed(args.seed)
        print("seed:", args.seed)

    N = args.nislands
    P = args.popSize
    n = args.chromSize
    T = args.targetFitness
    G = args.maxGen

    # init = dim.Zerofy(n)
    init = dim.Defined(n,args.definedChromSize)
    init_matrix = dim.InitMatrix(N,1/N*100)
    __eval = OneMaxFullEval()
    reward = args.strategy(args.alpha, args.beta)
    updater = dim.Updater(reward)
    memorizer = dim.Memorize()

    matrix = init_matrix()
    print(matrix)

    pop_set = []
    data_set = []

    for i in range(N):
        pop_set += [dim.Population(P,init)]
        data_set += [dim.IslandData(i,N)]

    evolvers = []
    feedbackers = []
    updaters = [updater]*N
    memorizers = [memorizer]*N
    migrators = []
    checkpoints = []

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

        print(i, mon)

        evolver = dim.Evolver(__eval, mon); evolvers += [evolver]
        feedbacker = dim.Feedbacker(pop_set, data_set, args.alphaF); feedbackers += [feedbacker]
        migrator = dim.Migrator(pop_set, data_set); migrators += [migrator]

        fit = dim.Fit(T if T else n)
        cont = dim.Combined(fit)
        checkpoint = dim.Checkpoint(cont); checkpoints += [checkpoint]

        if G:
            maxgen = dim.MaxGen(G)
            cont.add(maxgen)

        monitor = dim.Print( out=open('result_monitor_%d' % data.rank, 'w'), stepTimer=args.stepTimer )
        monitor.addTo(checkpoint)

        for stat in [dim.IslandRank(),
                     dim.ElapsedTime(),
                     dim.ElapsedTimeBetweenGenerations(),
                     dim.Generation(),
                     dim.PopSize(),
                     dim.AverageFitness(),
                     dim.BestFitness(),
                     dim.BestOfBestFitness(pop_set, data_set),
                     dim.Probabilities(),
                     dim.CommingProbabilities(pop_set, data_set),
                     # dim.SumOfGoingProbabilities(),
                     # dim.SumOfCommingProbabilities(pop_set, data_set),
                     dim.Feedbacks(),
        ]:
            stat.addTo(checkpoint)
            monitor.add(stat)

    for step in [evolvers, feedbackers, updaters, memorizers, migrators]:
        for i in range(N):
            step[i].firstCall(pop_set[i], data_set[i])

    while True:
        stop = False
        for i in range(N):
            if not checkpoints[i](pop_set[i], data_set[i]):
                stop = True
                break
        if stop: break

        for i in range(N): evolvers[i](pop_set[i], data_set[i])

        for i in range(N): feedbackers[i].send(pop_set[i], data_set[i])
        for i in range(N): feedbackers[i].recv(pop_set[i], data_set[i])

        for i in range(N): updaters[i](pop_set[i], data_set[i])
        for i in range(N): memorizers[i](pop_set[i], data_set[i])

        for i in range(N): migrators[i].send(pop_set[i], data_set[i])
        for i in range(N): migrators[i].recv(pop_set[i], data_set[i])

    for step in [evolvers, feedbackers, updaters, memorizers, migrators]:
        for i in range(N):
            step[i].lastCall(pop_set[i], data_set[i])

# when executed, just run main():
if __name__ == '__main__':
    logger.debug('### script started ###')
    main()
    logger.debug('### script ended ###')
