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

"""
This is the Dynamic Islands Model module.

This module supplies several classes among problem representations, population, problem initializer, evaluation, statistic generator, checkpoint, variation operators, island data representations as well as DIM main classes like evolver, feedbacker, migrator, updater and the main algorithm.
"""

from parser import Parser
from threading import Thread, Barrier, BrokenBarrierError, Lock
import numpy as np
import random, sys
import logging

logger = logging.getLogger("dim")

class MutableValue:
    """A mutable generic value class used for Statistic classes

    >>> m = MutableValue(2)
    >>> m += 1; m
    3
    >>> m.set(10); m
    10
    >>> m == 10
    True
    >>> m < 20
    True
    >>> m > 50
    False
    >>> m.v
    10
    >>> print(m)
    10
    >>> m != 10
    False
    >>> m -= 10; m
    0
    >>> m *= 10; m
    0
    >>> m += 50
    >>> m /= 10; m
    5.0

    """

    def __init__(self, v=0): self.v = v

    def __iadd__(self, o): self.v += o; return self
    def __isub__(self, o): self.v -= o; return self
    def __imul__(self, o): self.v *= o; return self
    def __idiv__(self, o): self.v = self.v / o; return self
    def __itruediv__(self, o): self.v = self.v / o; return self

    def __add__(self, o): return self.v + o
    def __sub__(self, o): return self.v - o
    def __mul__(self, o): return self.v * o
    def __div__(self, o): return self.v / o
    def __truediv__(self, o): return self.v / o

    def __radd__(self, o): return self.v + o
    def __rsub__(self, o): return self.v - o
    def __rmul__(self, o): return self.v * o
    def __rdiv__(self, o): return self.v / o
    def __rtruediv__(self, o): return self.v / o

    def __gt__(self, o): return self.v > o
    def __ge__(self, o): return self.v >= o
    def __lt__(self, o): return self.v < o
    def __le__(self, o): return self.v <= o
    def __eq__(self, o): return self.v == o
    def __ne__(self, o): return self.v != o

    def __nonzero__(self): return self.v

    def __str__(self): return str(self.v)
    def __repr__(self): return str(self.v)

    def set(self, v): self.v = v

class Fitness:
    """A fitness class in order to store the fitness of an individual.

    >>> f1 = Fitness(); f2 = Fitness()
    >>> f1.set_fitness(10)
    >>> f2.set_fitness(10)
    >>> f1 == f2
    True
    >>> f1.fitness()
    10
    >>> f1.set_fitness(20)
    >>> f1.last_fitness()
    10
    >>> f2.last_fitness()
    10
    >>> f3 = Fitness(); f3.copy_from(f1)
    >>> (f3.fitness(), f3.last_fitness())
    (20, 10)
    """

    def __init__(self):
        self.__fitness = None
        self.__last_fitness = None
        self.__status = 0

    def __gt__(self, other): return self.__fitness > other.__fitness
    def __ge__(self, other): return self.__fitness >= other.__fitness
    def __lt__(self, other): return self.__fitness < other.__fitness
    def __le__(self, other): return self.__fitness <= other.__fitness
    def __eq__(self, other): return self.__fitness == other.__fitness
    def __ne__(self, other): return self.__fitness != other.__fitness

    def copy_from(self, other):
        self.__fitness = other.__fitness
        self.__last_fitness = other.__last_fitness
        self.__status = other.__status

    def set_fitness(self, new_value):
        self.__last_fitness = self.__fitness
        self.__fitness = new_value
        if self.__status < 2: self.__status += 1

    def fitness(self): return self.__fitness
    # def last_fitness(self): return self.__last_fitness if self.__status == 2 else self.__fitness

    def last_fitness(self):
        if self.__status == 1:
            return self.__fitness
        elif self.__status == 2:
            return self.__last_fitness
        return -1

class LastIsland:
    """Last island saver class.

    >>> l = LastIsland()
    >>> l.last_island() == None
    True
    >>> l.set_last_island(10)
    >>> l.last_island()
    10
    """

    def __init__(self):
        self.__last_island = None

    def copy_from(self, other):
        self.__last_island = other.__last_island

    def set_last_island(self, new_value):
        self.__last_island = new_value

    def last_island(self): return self.__last_island

class Individual(Fitness, LastIsland, list):
    """Individual problem representation class.

    >>> i = Individual()
    >>> i.set_fitness(1)
    >>> i.fitness()
    1
    >>> print(i)
    1 0
    """

    def size(self): return len(self)

    def __str__(self):
        print(self.fitness(), self.size(), end="")
        for x in self:
            print(" %d" % x, end="")
        return ""

    def copy_from(self, other):
        Fitness.copy_from(self, other)
        LastIsland.copy_from(self, other)
        self.clear()
        self += other

class Population(list):
    """Population of Individual class.

    >>> pop = Population(5, lambda ind: ind.append(False))
    >>> pop.size()
    5
    >>> pop
    [[False], [False], [False], [False], [False]]
    >>> apply(pop, lambda ind: ind.set_fitness(sum(ind)))
    >>> pop.best_element()
    [False]
    >>> pop.worse_element()
    [False]
    >>> pop.empty()
    False

    >>> pop = Population(5, lambda ind: ind.append(False))
    >>> import random; random.seed(0)
    >>> def eval(ind): ind.set_fitness( sum(ind ) )
    >>> def flip(ind):
    ...         pos = random.randint(0,ind.size()-1)
    ...         ind[pos] = not ind[pos]
    >>> apply(pop, eval)
    >>> pop[0].fitness(), pop[0].last_fitness()
    (0, 0)
    >>> apply(pop, flip); apply(pop, eval)
    >>> pop[0].fitness(), pop[0].last_fitness()
    (1, 0)
    >>> apply(pop, flip); apply(pop, eval)
    >>> pop[0].fitness(), pop[0].last_fitness()
    (0, 1)
    """

    def __init__(self, size, init):
        for i in range(size):
            ind = Individual()
            init(ind)
            self += [ind]

    def size(self): return len(self)
    def empty(self): return len(self) <= 0

    def __gt__(self, other): return self.best_element()  > other.best_element()
    def __ge__(self, other): return self.best_element() >= other.best_element()
    def __lt__(self, other): return self.best_element()  < other.best_element()
    def __le__(self, other): return self.best_element() <= other.best_element()
    def __eq__(self, other): return self.best_element() == other.best_element()
    def __ne__(self, other): return self.best_element() != other.best_element()

    def best_element(self):
        if self.empty():
            logger.warning("Population: Empty population, when calling best_element().")
            return None
        best = self[0]
        for ind in self:
            if ind > best: best = ind
        return best

    def worse_element(self):
        worse = self[0]
        for ind in self:
            if ind < worse: worse = ind
        return worse

    def __str__(self):
        print("%s %s %s" % (self.size(), self.worse_element().fitness(),
                                         self.best_element().fitness()))
        for x in self: print(x)
        return ""

def apply(pop, func):
    """As the map function, it applies a function to all individuals of a population.

    >>> pop = Population(5, lambda ind: ind.append(False))
    >>> apply(pop, lambda ind: ind.set_fitness(sum(ind)))
    >>> pop[0].fitness() == 0
    True
    """
    for x in pop: func(x)

class Init:
    def __init__(self, size): self.size = size
    def __call__(self, ind): pass

class ZerofyInit(Init):
    """Set all the bits of the solution to false.

    >>> ind = Individual()
    >>> init = ZerofyInit(10)
    >>> init(ind)
    >>> ind
    [False, False, False, False, False, False, False, False, False, False]
    """

    def __init__(self, size): Init.__init__(self, size)
    def __call__(self, ind): ind += [False] * self.size

class RandomInit(Init):
    """Set all the bits of solution to a random value.

    >>> import random; random.seed(0)
    >>> ind = Individual()
    >>> init = RandomInit(10)
    >>> init(ind)
    >>> ind
    [False, False, True, False, False, False, False, False, False, True]
    """

    def __init__(self, size): Init.__init__(self, size)
    def __call__(self, ind):
        for i in range(self.size):
            ind += [random.choice([True, False])]

class FullEval:
    def __call__(self, ind): pass

class PartialEval:
    def __call__(self, ind, index): pass

class Statistic(MutableValue):
    def addTo(self, cp):
        cp.stats += [self]
        return self

    def __call__(self, pop, data): pass
    def lastcall(self, pop, data): pass

class IslandRank(Statistic):
    def __call__(self, pop, data): self.set(data.rank)

class Generation(Statistic):
    def __call__(self, pop, data): self += 1

class PopSize(Statistic):
    def __call__(self, pop, data): self.set(pop.size())

class AverageFitness(Statistic):
    def __call__(self, pop, data):
        if pop.size(): self.set(round(np.mean([x.fitness() for x in pop]), 2))

class BestFitness(Statistic):
    def __call__(self, pop, data):
        if pop.size(): self.set(pop.best_element().fitness())

class Probabilities(Statistic):
    def __call__(self, pop, data): self.set("%s %d" % ([round(float(x)/10, 2) for x in data.proba], sum(data.proba)/10))

class Feedbacks(Statistic):
    def __call__(self, pop, data): self.set([round(float(x)/10, 3) for x in data.feedbacks])

class NormalizedFeedbacks(Statistic):
    def __call__(self, pop, data): self.set([round(x, 3) for x in Reward.normalize(data.feedbacks.copy(), 100)])

class BestFeedbacks(Statistic):
    def __call__(self, pop, data):
        __max = max(data.feedbacks)
        self.set([1 if x == __max else 0 for x in data.feedbacks])

class Continuator:
    def addTo(self, cp):
        cp.conts += [self]
        return self

    def __call__(self, pop, data): pass
    def lastcall(self, pop, data): pass

class Combined(Continuator, list):
    def __init__(self, cont):
        self += [cont]

    def add(self, cont):
        self += [cont]

    def __call__(self, pop, data):
        for cont in self:
            if not cont(pop, data): return False
        return True

class Checkpoint(Continuator):
    def __init__(self, cont=None):
        self.conts = []
        self.sorted = []
        self.stats = []
        self.updaters = []
        self.monitors = []

        if cont: cont.addTo(self)

    def __call__(self, pop, data):
        sorted_pop = None
        if len(self.sorted):
            sorted_pop = pop.copy()
            sorted_pop.sort()
            for op in self.sorted:
                op(sorted_pop, data)

        for op in self.stats: op(pop, data)
        for op in self.updaters: op()
        for op in self.monitors: op()

        bcontinue = True
        for cont in self.conts:
            if not cont(pop, data): bcontinue = False

        if not bcontinue:
            if len(self.sorted):
                for op in self.sorted: op.lastcall(sorted_pop, data)

            for op in self.stats: op.lastcall(pop, data)
            for op in self.updaters: op.lastcall()
            for op in self.monitors: op.lastcall()

        return bcontinue

    def add(self, cont): cont.addTo(self)

class MaxGen(Continuator, Statistic):
    def __init__(self, maxgen=100):
        Statistic.__init__(self)
        self.maxgen = maxgen

    def __call__(self, pop, data):
        if self < self.maxgen:
            self += 1
            return True

        logger.warning("STOP in MaxGen: maximum number of generation has reached %s" % self)
        self.set(0)
        return False

class Fit(Continuator):
    def __init__(self, optimum):
        self.optimum = optimum

    def __call__(self, pop, data):
        if pop.empty():
            logger.warning("Fit: Population empty")
            return True
        best = pop.best_element().fitness()
        if (best >= self.optimum):
            logger.warning("STOP in Fit: Best fitness has reached %f" % best)
            return False
        return True

class Monitor(list):
    def add(self, stat): self += [stat]

    def addTo(self, cp):
        cp.monitors += [self]
        return self

    def __call__(self): pass
    def lastcall(self): pass

class Updater:
    def lastcall(self): pass

    def addTo(self, cp):
        cp.updaters += [self]
        return self

from datetime import datetime as dt

class PrintMonitor(Monitor):
    def __init__(self, out=sys.stdout, delim="\t", stepTimer=0):
        self.out = out
        self.delim = delim
        self.stepTimer = stepTimer
        self.firstTime = True
        self.start = dt.now()

    def __call__(self):
        if not self.out:
            raise ValueError("OStreamMonitor: Could not write to the output stream")

        if not len(self): return

        if self.stepTimer:
            now = dt.now()
            elapsed = int( ( (now - self.start).total_seconds() * 1000 ) / self.stepTimer )
            if not elapsed: return
            self.start = now

        for stat in self:
            self.out.write( "%s%s" % (stat.__class__.__name__ if self.firstTime else stat, self.delim) )
        self.out.write("\n")
        self.out.flush()

        if self.firstTime: self.firstTime = False

class GenOp:
    def __call__(self, ind): return self.apply(ind)
    def apply(self, ind): pass

class OpContainer(GenOp):
    def __init__(self):
        self.ops = []

    def add(self, op, rate):
        self.ops += [(op, rate)]

class SequentialOp(OpContainer):
    def apply(self, ind):
        for op, rate in self.ops:
            if random.random() < rate:
                logger.debug("%s(%d)" % (op.op.__class__.__name__, op.op.nbits if op.op.__class__.__name__ == "DetBitFlip" else -1))
                for x in op(ind): yield x

class MonOp:
    def __call__(self, ind): pass
    def __str__(self): pass

class Queue(list):
    """Thread-safe queue class using lock while pushes and pops.

    >>> q = Queue()
    >>> q.push(1); q
    [1]
    >>> q.push(10); q
    [1, 10]
    >>> q.push(42); q
    [1, 10, 42]
    >>> q.pop()
    1
    >>> q.size()
    2
    >>> q.empty()
    False
    >>> (q.pop(), q.pop())
    (10, 42)

    >>> from threading import Thread
    >>> q = Queue()
    >>> t1 = Thread(target=lambda: [q.push(i*1) for i in range(10)])
    >>> t2 = Thread(target=lambda: [q.push(i*10) for i in range(10)])
    >>> t1.start(); t2.start()
    >>> t1.join(); t2.join()
    >>> q
    [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 10, 20, 30, 40, 50, 60, 70, 80, 90]
    """

    def __init__(self, values=None):
        self += values if values else []
        self.lock = Lock()

    def push(self, value):
        with self.lock:
            self.append(value)

    def pop(self):
        if not self.size():
            raise ValueError('the queue is empty.')
        with self.lock:
            return list.pop(self,0)

    def empty(self):
        with self.lock:
            return len(self) <= 0

    def size(self):
        with self.lock:
            return len(self)

class IslandData:
    """Island data representation class.

    >>> data = IslandData(0, 4)
    >>> len(data.feedbacks)
    4
    >>> len(data.proba)
    4
    >>> data.feedbacks
    [0, 0, 0, 0]
    >>> data.proba
    [0, 0, 0, 0]
    """

    def __init__(self, rank=0, size=0):
        self.feedbacks = []
        self.proba = []
        #feedbackerSendingQueue = Queue()
        self.feedbackerReceivingQueue = Queue()
        #migratorSendingQueue = Queue()
        self.migratorReceivingQueue = Queue()
        self.toContinue = True

        self.rank = rank
        self.size = size
        self.feedbackerBarrier = Barrier(size)
        self.migratorBarrier = Barrier(size)

        if self.size:
            self.feedbacks = [0]*self.size
            self.proba = [0]*self.size

class IslandOperator:
    def firstCall(self, pop, data): pass
    def __call__(self, pop, data): pass
    def lastCall(self, pop, data): pass

class Evolver(IslandOperator):
    """Evolving step of the DIM algo.

    >>> import random; random.seed(0)
    >>> def flip(ind): ind[random.randint(0,ind.size()-1)] = True
    >>> def eval(ind): ind.set_fitness(sum(ind))
    >>> def dummy(ind):
    ...         ind.append(False)
    ...         ind.set_last_island(0)
    >>> pop = Population(5, dummy)
    >>> apply(pop, eval)
    >>> data = IslandData()
    >>> pop
    [[False], [False], [False], [False], [False]]
    >>> evolve = Evolver(eval, flip)
    >>> evolve(pop, data)
    >>> pop
    [[True], [True], [True], [True], [True]]
    """

    def __init__(self, __eval, op, invalidate=True):
        self.eval = __eval
        self.op = op
        self.invalidate = invalidate

    def __call__(self, pop, data):
        for ind in pop:
            candidate = Individual()
            candidate.copy_from(ind)

            self.op(candidate)
            #if self.invalidate:
            #    candidate.invalidate()
            self.eval(candidate)

            if candidate.fitness() > ind.fitness():
                ind.copy_from(candidate)

class Feedbacker(IslandOperator):
    """Feedback sender step.

    >>> from threading import Thread
    >>> import random; random.seed(0)
    >>> def eval(ind): ind.set_fitness(sum(ind))
    >>> pop_set = []; data_set = []; ts = []
    >>> for i in range(2):
    ...         def dummy(ind):
    ...                 ind += [False, False]
    ...                 ind.set_last_island(i)
    ...         pop_set.append(Population(5, dummy))
    ...         data_set.append(IslandData(i, 2))
    ...         apply(pop_set[i], eval)
    ...         def flip(ind):
    ...                 for k in range(i+1):
    ...                         ind[k] = not ind[k]
    ...         apply(pop_set[i], flip)
    ...         apply(pop_set[i], eval)
    ...         ts.append( Thread(target=Feedbacker(pop_set, data_set), args=[pop_set[i], data_set[i]]) )
    >>> for i in range(2):
    ...         ts[i].start()
    >>> for i in range(2):
    ...         ts[i].join()
    >>> (data_set[0].feedbacks, data_set[1].feedbacks)
    ([0.01, 0.0], [0.0, 0.02])
    """

    def __init__(self, pop_set=None, data_set=None, alpha=0.01):
        self.pop_set = pop_set
        self.data_set = data_set
        self.alpha = alpha

    @staticmethod
    def get_effectivenesses(pop, data):
        """Computes effectiveness of each island relatively to the current island.
        """

        # sums = [0]*data.size
        # nbs = [0]*data.size

        sums = []
        nbs = []
        for i in range(data.size):
            sums.append(0)
            nbs.append(0)

        for ind in pop:
            sums[ind.last_island()] += ind.fitness() - ind.last_fitness()
            # print(data.rank, ind.last_island(), ind.fitness() - ind.last_fitness())
            nbs[ind.last_island()] += 1

        effectivenesses = [v/m if m else 0 for v,m in zip(sums, nbs)]

        # print(sums, nbs, effectivenesses)

        # effectivenesses = []
        # for i in range(data.size):
        #     effectivenesses.append(sums[i] / nbs[i] if nbs[i] > 0 else 0)

        # print(data.rank, effectivenesses)

        return effectivenesses

    def __call__(self, pop, data):
        effectivenesses = Feedbacker.get_effectivenesses(pop, data)

        for i in range(data.size):
            self.data_set[i].feedbackerReceivingQueue.push( (effectivenesses[i], data.rank) )

        self.data_set[0].feedbackerBarrier.wait()

        while not data.feedbackerReceivingQueue.empty():
            Fi, __from = data.feedbackerReceivingQueue.pop()
            data.feedbacks[__from] = (1-self.alpha)*data.feedbacks[__from] + self.alpha*Fi

    def lastCall(self, __pop, __data):
        self.data_set[0].feedbackerBarrier.abort()

class Migrator(IslandOperator):
    def __init__(self, pop_set=None, data_set=None):
        self.pop_set = pop_set
        self.data_set = data_set

    def __call__(self, pop, data):
        output_sizes = [0]*data.size

        for ind in pop:
            # selection
            s = 0.
            r = random.randint(0, 1000+1)

            i = 0
            while i < data.size and r >= s:
                s += data.proba[i]
                i += 1
            i -= 1

            self.data_set[i].migratorReceivingQueue.push( (ind, data.rank) )

        pop.clear()

        self.data_set[0].migratorBarrier.wait()

        while not data.migratorReceivingQueue.empty():
            ind, __from = data.migratorReceivingQueue.pop()
            pop.append( ind )

    def lastCall(self, __pop, __data):
        self.data_set[0].migratorBarrier.abort()

class Reward(IslandOperator):
    @staticmethod
    def normalize(arr, high=1000):
        """Normalizes array with high value.

        >>> Reward.normalize([1,2,0,3],1)
        [0.16666666666666666, 0.3333333333333333, 0.0, 0.5]
        >>> Reward.normalize([1,2,0,3],1000)
        [166.66666666666666, 333.3333333333333, 0.0, 500.0]
        """

        __sum = sum(arr)
        cumul = 0
        for i in range(len(arr)-1):
            arr[i] = (arr[i]/__sum*high) if __sum else 0
            cumul += arr[i]
        arr[-1] = high-cumul
        return arr

class Best(Reward):
    def __init__(self, alpha=0.2, beta=0.01):
        self.alpha = alpha
        self.beta = beta

    def __call__(self, pop, data):
        __max, best = max((x,i) for i,x in enumerate(data.feedbacks))
        N = Reward.normalize( np.random.randint(0,1000,size=data.size) )
        a = self.alpha
        b = self.beta
        P = data.proba

        __sum = 0
        for i in range(data.size-1):
            if best == -1:
                P[i] = (1-b) *           P[i]             + b * N[i]
            elif best == i:
                P[i] = (1-b) * ( (1-a) * P[i] + a * 1000) + b * N[i]
            else:
                P[i] = (1-b) * ( (1-a) * P[i])            + b * N[i]
            __sum += P[i]
        P[-1] = 1000-__sum

class Average(Reward):
    def __init__(self, alpha=0.2, beta=0.01):
        self.alpha = alpha
        self.beta = beta

    def __call__(self, pop, data):
        R = Reward.normalize( data.feedbacks.copy() )
        N = Reward.normalize( np.random.randint(0,1000,size=data.size) )
        a = self.alpha
        b = self.beta
        P = data.proba

        __sum = 0
        for i in range(data.size-1):
            P[i] = (1-b) * ( (1-a) * P[i] + a * R[i] ) + b * N[i]
            __sum += P[i]
        P[-1] = 1000-__sum

class Updater(IslandOperator):
    def __init__(self, reward):
        self.reward = reward

    def __call__(self, pop, data):
        self.reward(pop, data)

class Memorize(IslandOperator):
    def firstCall(self, pop, data):
        for ind in pop:
            ind.set_last_island(data.rank)

    def __call__(self, pop, data):
        for ind in pop:
            ind.set_last_island(data.rank)

class Algo(IslandOperator):
    def __init__(self, evolve, feedback, update, memorize, migrate, checkpoint, pop_set=None, data_set=None):
        self.steps = [evolve, feedback, update, memorize, migrate]
        self.checkpoint = checkpoint
        self.pop_set = pop_set
        self.data_set = data_set

    def __call__(self, pop, data):
        for step in self.steps:
            step.firstCall(pop, data)

        while True:
            self.data_set[0].toContinue &= self.checkpoint(pop, data)
            if not self.data_set[0].toContinue: break

            # logger.info("%d %s %d %s" % (data.rank, data.proba, pop.size(),
            #                              pop.best_element().fitness() if pop.best_element() else None))

            try:
                for step in self.steps:
                    # logger.debug("%d %s" % (data.rank, step))
                    step(pop, data)
            except BrokenBarrierError as e:
                logger.error("%d %s" % (data.rank, "broken barrier"))
                break

        for step in self.steps:
            step.lastCall(pop, data)

        # logger.info("%d %s %d %s" % (data.rank, data.proba, pop.size(),
        #                              pop.best_element().fitness() if pop.best_element() else None))

class InitMatrix:
    def __init__(self, randomize = False, common_value = 0.9):
        self.randomize = randomize
        self.common_value = common_value

    def __call__(self, matrix):
        for i in range(len(matrix)):
            sum = 0
            for j in range(len(matrix)):
                if i == j:
                    matrix[i][j] = self.common_value
                else:
                    if self.randomize:
                        matrix[i][j] = round(random.random(), 3)
                    else:
                        matrix[i][j] = (1 - self.common_value) / (len(matrix) - 1)
                    sum += matrix[i][j]

            for j in range(len(matrix)):
                if i != j:
                    if sum:
                        matrix[i][j] = round(matrix[i][j] / sum * (1 - self.common_value), 3)
                    else:
                        matrix[i][j] = 0

if __name__ == "__main__":
    import doctest
    doctest.testmod()
