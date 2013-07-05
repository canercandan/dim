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
from numpy import *
import logging, sys, functools, inspect
from datetime import datetime as dt

logger = logging.getLogger("dim")

class DecoratorBase:
    """Here an easy-to-use decorator class where you can easily set your
    own decorator by only creating a derivated class from this base
    class.

    Example:
    >>> class Shortcuts(DecoratorBase):
    ...         def __call__(self, *args, **kwargs):
    ...                 data, = args
    ...                 kwargs.update({'a': data.the_first_variable,
    ...                                'b': data.the_second_variable,
    ...                                'c': data.the_third_variable})
    ...                 self.fn(*args, **kwargs)

    >>> class Data:
    ...         def __init__(self):
    ...                 self.the_first_variable = 1
    ...                 self.the_second_variable = 2
    ...                 self.the_third_variable = 3

    >>> @Shortcuts
    ... def foo(data,a,b,c): print(a,b,c)

    >>> data = Data()
    >>> foo(data)
    1 2 3
    """

    def __init__(self, fn): self.fn = fn
    def __get__(self, obj, type=None): return functools.partial(self, obj)

    def __call__(self, *args, **kwargs):
        print("Entering", self.fn.__name__)
        result = self.fn(*args, **kwargs)
        print("Exited", self.fn.__name__)
        return result

def isinstance2(obj, *types):
    """Another version of isinstance that takes in account NoneType value.

    Example:
    >>> try: isinstance(None,None)
    ... except TypeError as e: print(e)
    isinstance() arg 2 must be a type or tuple of types
    >>> isinstance2(None,None)
    True
    >>> isinstance2(None,int)
    False
    >>> isinstance2(None,(None))
    True
    >>> isinstance2(None,(int,bool,None))
    True
    >>> isinstance2(None,(int,bool))
    False
    >>> isinstance2(None,(None,int,bool))
    True
    >>> isinstance2(42,(None,int,bool))
    True
    >>> isinstance2(42.,(None,int,bool))
    False
    >>> isinstance2(42.,(None,int,bool,float))
    True
    """

    # whether types argument is passed as tuple
    if types.__class__ == tuple and \
       len(types) == 1 and \
       types[0].__class__ == tuple:
        types, = types

    return (obj if obj is None else obj.__class__) in types

def accepts(*types):
    """Decorator used to enforce the use of a set of types of arguments.

    Example:
    >>> @accepts(int)
    ... def foo(x): pass

    >>> foo(1)

    >>> try: foo(1.)
    ... except AssertionError as e: print(e)
    arg x = 1.0 (<class 'float'>) does not match <class 'int'>

    >>> @accepts((int,float))
    ... def foo2(x): pass

    >>> foo2(1.)
    """

    class _Accepts(DecoratorBase):
        def __init__(self, fn):
            DecoratorBase.__init__(self, fn)
            self.fn_args = inspect.getargspec(fn).args
            assert len(types) == len(self.fn_args), "length of types are different from args (%d <> %d) (%s <> %s)" % (len(types), len(self.fn_args), types, self.fn_args)

        def __call__(self, *args, **kwargs):
            for f, a, t in zip(self.fn_args, args, types):
                if t == 'skip': continue
                assert isinstance2(a,t), "arg %s = %r (%s) does not match %s" % (f,a,a.__class__,t)
            return self.fn(*args, **kwargs)

    return _Accepts

def returns(*rtype):
    """Decorator used to enforce the use of a set of types of arguments.

    Example:
    >>> @returns(int)
    ... def foo(x): return x

    >>> foo(42)
    42

    >>> try: foo(42.)
    ... except AssertionError as e: print(e)
    return value 42.0 (<class 'float'>) does not match (<class 'int'>,)

    >>> @returns(int,None)
    ... @accepts((float,int,None))
    ... def foo(x): return x

    >>> foo(42)
    42
    >>> foo(None)
    """

    class _Returns(DecoratorBase):
        def __call__(self, *args, **kwargs):
            result = self.fn(*args, **kwargs)
            assert isinstance2(result,*rtype), "return value %r (%s) does not match %s" % (result,result.__class__,rtype)
            return result

    return _Returns

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

class Init:
    def __init__(self, size): self.size = size
    def __call__(self, ind): pass

class ZerofyInit(Init):
    """Set all the bits of the solution to false.

    >>> ind = Individual()
    >>> init = ZerofyInit(10)
    >>> init(ind)
    >>> ind
    [0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    """

    def __init__(self, size): Init.__init__(self, size)
    def __call__(self, ind): ind += [0] * self.size

class DefinedInit(Init):
    """Set a defined number of bits to true and the rest to false.

    >>> ind = Individual()
    >>> init = DefinedInit(10,5)
    >>> init(ind)
    >>> ind
    [0, 0, 0, 0, 0, 1, 1, 1, 1, 1]
    """

    def __init__(self, size, defined_size):
        Init.__init__(self, size)
        self.defined_size = defined_size

    def __call__(self, ind):
        ind += [0] * (self.size-self.defined_size) + [1] * self.defined_size

class RandomInit(Init):
    """Set all the bits of solution to a random value.

    >>> random.seed(0)
    >>> ind = Individual()
    >>> init = RandomInit(10)
    >>> init(ind)
    >>> ind
    [0, 1, 1, 0, 1, 1, 1, 1, 1, 1]
    """

    def __init__(self, size): Init.__init__(self, size)
    def __call__(self, ind): ind += random.choice(2,size=self.size)

class FullEval:
    def __call__(self, ind): pass

class PartialEval:
    def __call__(self, ind, index): pass

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

class DataShortcuts(DecoratorBase):
    """Decorator in order to create variables used such as shortcuts.
    """

    def __call__(self, *args, **kwargs):
        cls,pop,data = args
        kwargs.update({
            'F': data.feedbacks,
            'P': data.proba,
            'fq': data.feedbackerReceivingQueue,
            'mq': data.migratorReceivingQueue,
            'k': data.rank,
            'n': data.size,
        })
        return self.fn(*args, **kwargs)

class IslandOperator:
    def firstCall(self, pop, data): pass
    def __call__(self, pop, data): pass
    def lastCall(self, pop, data): pass

class Statistic(MutableValue):
    def addTo(self, cp):
        cp.stats += [self]
        return self

    def __call__(self, pop, data): pass
    def lastcall(self, pop, data): pass

class IslandRank(Statistic):
    def __call__(self, pop, data): self.set(data.rank)

class ElapsedTime(Statistic):
    def __init__(self): self.start = dt.now()
    def __call__(self, pop, data): self.set(round((dt.now() - self.start).total_seconds(), 2))

class ElapsedTimeBetweenGenerations(Statistic):
    def __init__(self): self.start = dt.now()
    def __call__(self, pop, data):
        now = dt.now()
        self.set(round((now - self.start).total_seconds(), 2))
        self.start = now

class Generation(Statistic):
    def __call__(self, pop, data): self += 1

class PopSize(Statistic):
    def __call__(self, pop, data): self.set(pop.size())

class AverageFitness(Statistic):
    def __call__(self, pop, data):
        if pop.size(): self.set(round(mean([x.fitness() for x in pop]), 2))

class BestFitness(Statistic):
    def __call__(self, pop, data):
        if pop.size(): self.set(pop.best_element().fitness())

class BestOfBestFitness(Statistic):
    def __init__(self, pop_set, data_set):
        self.pop_set = pop_set
        self.data_set = data_set

    def __call__(self, pop, data):
        self.set( max([p.best_element().fitness() if p.size() else 0 for p in self.pop_set]) )

class Probabilities(Statistic):
    def __call__(self, pop, data): self.set( (array(data.proba)).round(1) )

class CommingProbabilities(Statistic):
    def __init__(self, pop_set, data_set):
        self.pop_set = pop_set
        self.data_set = data_set

    def __call__(self, pop, data):
        self.set( (sum([d.proba for d in self.data_set], axis=0)/data.size).round(1) )

class SumOfGoingProbabilities(Statistic):
    def __call__(self, pop, data): self.set( sum(data.proba) )

class SumOfCommingProbabilities(Statistic):
    def __init__(self, pop_set, data_set):
        self.pop_set = pop_set
        self.data_set = data_set

    def __call__(self, pop, data):
        self.set( round(sum([d.proba[data.rank] for d in self.data_set])/(data.size*100)*100, 2) )

class Feedbacks(Statistic):
    @DataShortcuts
    def __call__(self, pop, data, F, P, fq, mq, k, n): self.set(F.round(2))

class NormalizedFeedbacks(Statistic):
    def __call__(self, pop, data): F = data.feedbacks.copy(); self.set(F/F.sum()*100)

class BestFeedbacks(Statistic):
    """
    Display only the best feedback among others.

    >>> best = BestFeedbacks()
    >>> data = IslandData()
    >>> random.seed(0)
    >>> data.feedbacks = random.random(4)
    >>> best(None, data); best
    [0 1 0 0]
    """

    def __call__(self, pop, data):
        self.set((array(data.feedbacks == data.feedbacks.max(), dtype=int)))

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

    @returns(bool)
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

    @returns(bool)
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

    @returns(bool)
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

    @returns(bool)
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

    @returns(bool)
    def empty(self):
        with self.lock:
            return len(self) <= 0

    @returns(int)
    def size(self):
        with self.lock:
            return len(self)

class Evolver(IslandOperator):
    """Evolving step of the DIM algo.

    >>> random.seed(0)
    >>> def flip(ind): ind[random.randint(0,ind.size())] = 1
    >>> def eval(ind): ind.set_fitness(sum(ind))
    >>> def dummy(ind):
    ...         ind.append(0)
    ...         ind.set_last_island(0)
    >>> pop = Population(5, dummy)
    >>> apply(pop, eval)
    >>> data = IslandData()
    >>> pop
    [[0], [0], [0], [0], [0]]
    >>> evolve = Evolver(eval, flip)
    >>> evolve(pop, data)
    >>> pop
    [[1], [1], [1], [1], [1]]
    """

    def __init__(self, __eval, op):
        self.eval = __eval
        self.op = op

    def __call__(self, pop, data):
        for ind in pop:
            candidate = Individual()
            candidate.copy_from(ind)

            self.op(candidate)
            self.eval(candidate)

            if candidate.fitness() > ind.fitness():
                ind.copy_from(candidate)

class Mean(MutableValue):
    """Computes mean step by step.

    >>> m = Mean()
    >>> d = [1,2,3,4]
    >>> for x in d: m(x)
    >>> from numpy import mean
    >>> m == mean(d)
    True
    """

    def __init__(self):
        MutableValue.__init__(self)
        self.n = 0

    def __call__(self, x):
        self.n += 1
        self.set( ((x-self.v)/self.n)+self.v )

class Feedbacker(IslandOperator):
    """Feedback sender step.

    >>> from threading import Thread
    >>> random.seed(0)
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
    (array([ 0.01,  0.  ]), array([ 0.  ,  0.02]))
    """

    def __init__(self, pop_set=None, data_set=None, alpha=0.01):
        self.pop_set = pop_set
        self.data_set = data_set
        self.alpha = alpha

    @DataShortcuts
    @returns(list)
    def make_effectivenesses(self, pop, data, F, P, fq, mq, k, n):
        """Computes effectiveness of each island relatively to the current island.
        """

        means = []
        for i in range(n): means += [Mean()]
        for ind in pop: means[ind.last_island()]( ind.fitness() - ind.last_fitness() )
        return means

    @DataShortcuts
    def send(self, pop, data, F, P, fq, mq, k, n):
        effectivenesses = self.make_effectivenesses(pop, data)
        # print(effectivenesses)
        for i in range(n):
            self.data_set[i].feedbackerReceivingQueue.push( (effectivenesses[i], k) )

    @DataShortcuts
    def recv(self, pop, data, F, P, fq, mq, k, n):
        a = self.alpha
        while not fq.empty():
            Fi, __from = fq.pop()
            F[__from] = (1-a) * F[__from] + a * Fi

    def __call__(self, pop, data):
        self.send(pop, data)
        self.data_set[0].feedbackerBarrier.wait()
        self.recv(pop, data)

    def lastCall(self, __pop, __data):
        self.data_set[0].feedbackerBarrier.abort()

class Migrator(IslandOperator):
    def __init__(self, pop_set=None, data_set=None):
        self.pop_set = pop_set
        self.data_set = data_set

    @DataShortcuts
    def send(self, pop, data, F, P, fq, mq, k, n):
        output_sizes = [0]*n

        for ind in pop:
            # selection
            s = 0.
            r = random.randint(0, 1000)+1

            i = 0
            while i < n and r >= s:
                s += P[i]
                i += 1
            i -= 1

            self.data_set[i].migratorReceivingQueue.push( (ind, k) )

        pop.clear()

    @DataShortcuts
    def recv(self, pop, data, F, P, fq, mq, k, n):
        while not mq.empty():
            ind, __from = mq.pop()
            pop.append( ind )

    def __call__(self, pop, data):
        self.send(pop, data)
        self.data_set[0].migratorBarrier.wait()
        self.recv(pop, data)

    def lastCall(self, __pop, __data):
        self.data_set[0].migratorBarrier.abort()

class Reward(IslandOperator): pass

class Best(Reward):
    def __init__(self, alpha=0.2, beta=0.01):
        self.alpha = alpha
        self.beta = beta

    @DataShortcuts
    def __call__(self, pop, data, F, P, fq, mq, k, n):
        best = array(F).argmax()
        N = random.random(n); N = N/N.sum()*100
        a = self.alpha
        b = self.beta

        __sum = 0
        for i in range(n-1):
            if best == -1:
                P[i] = (1-b) *           P[i]            + b * N[i]
            elif best == i:
                P[i] = (1-b) * ( (1-a) * P[i] + a * 100) + b * N[i]
            else:
                P[i] = (1-b) * ( (1-a) * P[i])           + b * N[i]
            __sum += P[i]
        P[-1] = 100-__sum

class Average(Reward):
    def __init__(self, alpha=0.2, beta=0.01):
        self.alpha = alpha
        self.beta = beta

    @DataShortcuts
    def __call__(self, pop, data, F, P, fq, mq, k, n):
        R = array( F.copy() ); R = R/R.sum()*100
        N = random.random(data.size); N = N/N.sum()*100
        a = self.alpha
        b = self.beta

        __sum = 0
        for i in range(n-1):
            P[i] = (1-b) * ( (1-a) * P[i] + a * R[i] ) + b * N[i]
            __sum += P[i]
        P[-1] = 1000-__sum

class Updater(IslandOperator):
    def __init__(self, reward):
        self.reward = reward

    def __call__(self, pop, data):
        self.reward(pop, data)

class Memorize(IslandOperator):
    @DataShortcuts
    def firstCall(self, pop, data, F, P, fq, mq, k, n):
        for ind in pop:
            ind.set_last_island(k)

    @DataShortcuts
    def __call__(self, pop, data, F, P, fq, mq, k, n):
        for ind in pop:
            ind.set_last_island(k)

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

            try:
                for step in self.steps:
                    step(pop, data)
            except BrokenBarrierError as e:
                logger.error("%d %s" % (data.rank, "broken barrier"))
                break

        for step in self.steps:
            step.lastCall(pop, data)

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
