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

import numpy as np
from numpy import *
from decorators import *
import logging
from threading import Thread, Barrier, BrokenBarrierError, Lock

logger = logging.getLogger("dim.core")

class MutableValue:
    """A mutable generic value class used for Statistic classes.

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

    @accepts('skip', (float,int,int64))
    def set_fitness(self, new_value):
        self.__last_fitness = self.__fitness
        self.__fitness = new_value
        if self.__status < 2: self.__status += 1

    @returns(float,int,None,int64)
    def fitness(self): return self.__fitness

    @returns(float,int,None,int64)
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

    @accepts('skip', (int,int64))
    def set_last_island(self, new_value):
        self.__last_island = new_value

    @returns(int,int64,None)
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

    @returns(int)
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

    >>> pop = Population(5, lambda ind: ind.append(0))
    >>> pop.size()
    5
    >>> pop
    [[0], [0], [0], [0], [0]]
    >>> apply(pop, lambda ind: ind.set_fitness(sum(ind)))
    >>> pop.best_element()
    [0]
    >>> pop.worse_element()
    [0]
    >>> pop.empty()
    False

    >>> pop = Population(5, lambda ind: ind.append(False))
    >>> random.seed(0)
    >>> def eval(ind): ind.set_fitness( sum(ind ) )
    >>> def flip(ind):
    ...         pos = random.randint(0,ind.size())
    ...         ind[pos] = not ind[pos]
    >>> apply(pop, eval)
    >>> pop[0].fitness()
    0
    >>> pop[0].last_fitness()
    0
    >>> apply(pop, flip); apply(pop, eval)
    >>> pop[0].fitness()
    1
    >>> pop[0].last_fitness()
    0
    >>> apply(pop, flip); apply(pop, eval)
    >>> pop[0].fitness()
    0
    >>> pop[0].last_fitness()
    1
    >>> print(pop)
    5 0 0
    0 1 0
    0 1 0
    0 1 0
    0 1 0
    0 1 0
    <BLANKLINE>
    """

    def __init__(self, size=0, init=None):
        for i in range(size):
            ind = Individual()
            if init: init(ind)
            self += [ind]

    @returns(int)
    def size(self): return len(self)

    @returns(bool)
    def empty(self): return len(self) <= 0

    def __gt__(self, other): return self.best_element()  > other.best_element()
    def __ge__(self, other): return self.best_element() >= other.best_element()
    def __lt__(self, other): return self.best_element()  < other.best_element()
    def __le__(self, other): return self.best_element() <= other.best_element()
    def __eq__(self, other): return self.best_element() == other.best_element()
    def __ne__(self, other): return self.best_element() != other.best_element()

    @returns((Individual,None))
    def best_element(self):
        if self.empty():
            logger.warning("Population: Empty population, when calling best_element().")
            return None
        best = self[0]
        for ind in self:
            if ind > best: best = ind
        return best

    @returns((Individual,None))
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

    @returns(bool)
    def empty(self):
        with self.lock:
            return len(self) <= 0

    @returns(int)
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
    array([ 0.,  0.,  0.,  0.])
    >>> data.proba
    array([ 0.,  0.,  0.,  0.])
    """

    def __init__(self, rank=0, size=0):
        self.feedbacks = repeat(0., size)
        self.proba = repeat(0., size)
        #feedbackerSendingQueue = Queue()
        self.feedbackerReceivingQueue = Queue()
        #migratorSendingQueue = Queue()
        self.migratorReceivingQueue = Queue()
        self.toContinue = True

        self.rank = rank
        self.size = size
        self.feedbackerBarrier = Barrier(size)
        self.migratorBarrier = Barrier(size)

class IslandOperator:
    def firstCall(self, pop, data): pass
    def __call__(self, pop, data): pass
    def lastCall(self, pop, data): pass

class InitMatrix:
    """Class used to initialized a matrix values.

    Example:
    >>> init = InitMatrix(4, 1/4*100)
    >>> mat = init(); print(mat)
    [[ 25.  25.  25.  25.]
     [ 25.  25.  25.  25.]
     [ 25.  25.  25.  25.]
     [ 25.  25.  25.  25.]]
    """

    def __init__(self, size, common_value=90, randomize=False, diagonal=False):
        self.size = size
        self.randomize = randomize
        self.common_value = common_value
        self.diagonal = diagonal

    @returns(ndarray)
    def __call__(self):
        n = self.size
        if self.diagonal:
            M = repeat((100-self.common_value)/(n-1),n**2).reshape(n,n)
            for i in range(n): M[i,i] = self.common_value
            return M

        if not self.randomize:
            return repeat(float(self.common_value), n**2).reshape(n,n)

        return array([m/m.sum() for m in random.random(n**2).reshape(n,n)])

if __name__ == "__main__":
    import doctest
    doctest.testmod()
