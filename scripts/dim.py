from threading import Thread, Barrier
from queue import Queue
import random

class MutableValue:
    def __init__(self, v=0): self.v = v

    def __iadd__(self, o): self.v += o; return self
    def __isub__(self, o): self.v -= o; return self

    def __add__(self, o): return self.v + o
    def __sub__(self, o): return self.v - o

    def __radd__(self, o): return self.v + o
    def __rsub__(self, o): return self.v - o

    def __gt__(self, o): return self.v > o
    def __ge__(self, o): return self.v >= o
    def __lt__(self, o): return self.v < o
    def __le__(self, o): return self.v <= o
    def __eq__(self, o): return self.v == o
    def __ne__(self, o): return self.v != o

    def __nonzero__(self): return self.v

    def __str__(self): return str(self.v)

    def set(self, v): self.v = v

class Population(list):
    def __init__(self, size, init):
        for i in range(size):
            ind = Individual()
            init(ind)
            self += [ind]

            self.best_individual = None
            self.worth_individual = None

    def size(self): return len(self)

    def __gt__(self, other): return self.best_individual > other.best_individual
    def __ge__(self, other): return self.best_individual >= other.best_individual
    def __lt__(self, other): return self.best_individual < other.best_individual
    def __le__(self, other): return self.best_individual <= other.best_individual
    def __eq__(self, other): return self.best_individual == other.best_individual
    def __ne__(self, other): return self.best_individual != other.best_individual

    def sort(self):
        best = self.best_individual if self.best_individual else self[0]
        worth = self.worth_individual if self.worth_individual else self[0]

        for ind in self:
            if ind.fitness() > best.fitness(): best = ind
            if ind.fitness() < worth.fitness(): worth = ind

        self.best_individual = best
        self.worth_individual = worth

    def __str__(self):
        self.sort()
        print("%s %s %s" % (self.size(), self.worth_individual.fitness(),
                                         self.best_individual.fitness()))
        for x in self:
            print(x)
        return ""

def apply(pop, func):
    for x in pop: func(x)

class Init:
    def __init__(self, size): self.size = size
    def __call__(self, ind): pass

class ZerofyInit(Init):
    def __init__(self, size): Init.__init__(self, size)
    def __call__(self, ind): ind += [False] * self.size

class RandomInit(Init):
    def __init__(self, size): Init.__init__(self, size)
    def __call__(self, ind):
        for i in range(self.size):
            ind += [random.choice([True, False])]

class FullEval:
    def __call__(self, ind): pass

class PartialEval:
    def __call__(self, ind, index): pass

class OneMaxFullEval(FullEval):
    def __call__(self, ind):
        ind.set_fitness(sum(ind))

class OneMaxIncrEval(PartialEval):
    def __call__(self, ind, index):
        ind.set_fitness( ind.fitness() + (-1 if ind[index] else 1) )

class Statistic(MutableValue):
    def addTo(self, cp):
        cp.stats += [self]
        return self

    def __call__(self, pop): pass
    def lastcall(self, pop): pass

class BestFitness(Statistic):
    def __call__(self, pop): self.set(pop.best_individual.fitness())

class Average(Statistic):
    def __call__(self, pop):
        self.set(np.mean([x.fitness() for x in pop]))

class Continuator:
    def addTo(self, cp):
        cp.conts += [self]
        return self

    def __call__(self, pop): pass
    def lastcall(self, pop): pass

class Checkpoint(Continuator):
    def __init__(self, cont=None):
        self.conts = []
        self.sorted = []
        self.stats = []
        self.updaters = []
        self.monitors = []

        if cont: cont.addTo(self)

    def __call__(self, pop):
        from copy import copy
        sorted_pop = copy(pop)
        if len(self.sorted):
            sorted_pop.sort()
            for op in self.sorted:
                op(sorted_pop)

        for l in [self.stats, self.updaters, self.monitors]:
            for op in l:
                op(pop)

        bcontinue = True
        for cont in self.conts:
            if not cont(pop):
                bcontinue = False

        if not bcontinue:
            if len(self.sorted):
                for op in self.sorted:
                    op.lastcall(sorted_pop)

            for l in [self.stats, self.updaters, self.monitors]:
                for op in l:
                    op.lastcall(pop)

        return bcontinue

    def add(self, cont): cont.addTo(self)

class MaxGen(Continuator):
    def __init__(self, maxgen=100, print_counter=False):
        self.maxgen = maxgen
        self.counter = 0
        self.print_counter = print_counter

    def __call__(self, pop):
        if self.counter < self.maxgen:
            self.counter += 1
            if self.print_counter: print(self.counter,end=" ")
            return True

        self.counter = 0
        return False

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
                print("%s(%d)" % (op.op.__class__.__name__, op.op.nbits if op.op.__class__.__name__ == "DetBitFlip" else -1))
                for x in op(ind): yield x

class IslandData:
    feedbacks = []
    proba = []
    feedbackerSendingQueue = Queue()
    feedbackerReceivingQueue = Queue()
    migratorSendingQueue = Queue()
    migratorReceivingQueue = Queue()
    toContinue = False

    def __init__(self, rank=0, size=0):
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

class Feedbacker(IslandOperator):
    def __init__(self, pop_set=None, data_set=None, alpha=0.01):
        self.pop_set = pop_set
        self.data_set = data_set
        self.alpha = alpha

    def __call__(self, __pop, __data):
        pop = self.pop_set[__data.rank]
        data = self.data_set[__data.rank]

        sums = [0]*data.size
        nbs = [0]*data.size

        for ind in pop:
            sums[ind.last_island()] += ind.fitness() - ind.last_fitness()
            nbs[ind.last_island()] += 1

        for i in range(data.size):
            effectiveness = sums[i] / nbs[i] if nbs[i] > 0 else 0
            self.data_set[i].feedbackerReceivingQueue.put( (effectiveness, data.rank) )

        self.data_set[0].feedbackerBarrier.wait()

        while not data.feedbackerReceivingQueue.empty():
            Fi, __from = data.feedbackerReceivingQueue.get()
            data.feedbacks[__from] = (1-self.alpha)*data.feedbacks[__from] + self.alpha*Fi
            data.feedbackerReceivingQueue.task_done()

class Migrator(IslandOperator):
    def __init__(self, pop_set=None, data_set=None):
        self.pop_set = pop_set
        self.data_set = data_set

    def __call__(self, __pop, __data):
        pop = self.pop_set[__data.rank]
        data = self.data_set[__data.rank]

        output_sizes = [0]*data.size

        for ind in pop:
            # selection
            s = 0.
            r = random.randint(0, 1000 + 1)

            j = 0
            while j < data.size and r > s:
                s += data.proba[j]
                j += 1
            j -= 1

            self.data_set[j].migratorReceivingQueue.put( (ind, data.rank) )

        pop.clear()

        self.data_set[0].migratorBarrier.wait()

        while not data.migratorReceivingQueue.empty():
            ind, __from = data.migratorReceivingQueue.get()
            pop.append( ind )
            data.migratorReceivingQueue.task_done()

class Reward(IslandOperator): pass

class Best(Reward):
    def __init__(self, alpha=0.2, beta=0.01):
        self.alpha = alpha
        self.beta = beta

    def __call__(self, pop, data):
        __max, best = max((x,i) for i,x in enumerate(data.feedbacks))

        epsilon = [0]*data.size
        __sum = 0
        for i in range(data.size):
            epsilon[i] = random.randint(0, 1000)
            __sum += epsilon[i]

        sum_sum = 0
        for i in range(data.size-1):
            epsilon[i] = epsilon[i] / __sum * 1000 if __sum else 0
            sum_sum += epsilon[i]
        epsilon[-1] = 1000-sum_sum

        __sum = 0
        for i in range(data.size-1):
            if best == -1:
                data.proba[i] = (1-self.beta)*data.proba[i] + self.beta*epsilon[i]
            else:
                if i == best:
                    data.proba[i] = (1-self.beta)*( (1-self.alpha)*data.proba[i] + self.alpha*1000) + self.beta*epsilon[i]
                else:
                    data.proba[i] = (1-self.beta)*( (1-self.alpha)*data.proba[i]) + self.beta*epsilon[i]
            __sum += data.proba[i]
        data.proba[-1] = 1000-__sum

class Proportional(Reward):
    def __init__(self, alpha=0.2, beta=0.01):
        self.alpha = alpha
        self.beta = beta

    def __call__(self, pop, data):
        S = data.feedbacks
        __sum = sum(S)

        if __sum:
            sum_sum = 0
            for i in range(data.size-1):
                S[i] = S[i] / __sum * 1000 if S[i] > 0 else 0
                sum_sum += S[i]
            S[-1] = 1000-sum_sum
        else:
            return

        epsilon = [0]*data.size
        __sum = 0
        for i in range(data.size):
            epsilon[i] = random.randint(0, 1000)
            __sum += epsilon[i]

        sum_sum = 0
        for i in range(data.size-1):
            epsilon[i] = epsilon[i] / __sum * 1000 if __sum else 0
            sum_sum += epsilon[i]
        epsilon[-1] = 1000-sum_sum

        __sum = 0
        for i in range(data.size-1):
            data.proba[i] = (1-self.beta)*( (1-self.alpha)*data.proba[i] + self.alpha*S[i] ) + self.beta*epsilon[i]
            __sum += data.proba[i]
        data.proba[-1] = 1000-__sum

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

class Algo(IslandOperator, Thread):
    def __init__(self, evolve, feedback, update, memorize, migrate, checkpoint, pop, data):
        Thread.__init__(self, target=self, args=[pop,data])
        self.steps = [evolve, feedback, update, memorize, migrate]
        self.checkpoint = checkpoint

    def __call__(self, pop, data):
        for step in self.steps:
            step.firstCall(pop, data)

        while self.checkpoint(pop):
            #pop.sort()
            #print(data.rank, data.proba, pop.size(), pop.best_individual.fitness())

            for step in self.steps:
                step(pop, data)

        for step in self.steps:
            step.lastCall(pop, data)

        return data.rank

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

class Fitness:
    def __init__(self):
        self.__fitness = None
        self.__last_fitness = None

    def __gt__(self, other): return self.__fitness > other.__fitness
    def __ge__(self, other): return self.__fitness >= other.__fitness
    def __lt__(self, other): return self.__fitness < other.__fitness
    def __le__(self, other): return self.__fitness <= other.__fitness
    def __eq__(self, other): return self.__fitness == other.__fitness
    def __ne__(self, other): return self.__fitness != other.__fitness

    def copy_from(self, other):
        self.__fitness = other.__fitness
        self.__last_fitness = other.__last_fitness

    def set_fitness(self, new_value):
        self.__last_fitness = self.__fitness
        self.__fitness = new_value

    def fitness(self): return self.__fitness
    def last_fitness(self): return self.__last_fitness if self.__last_fitness else self.__fitness

class LastIsland:
    def __init__(self):
        self.__last_island = None

    def copy_from(self, other):
        self.__last_island = other.__last_island

    def set_last_island(self, new_value):
        self.__last_island = new_value

    def last_island(self): return self.__last_island

class Individual(Fitness, LastIsland, list):
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

class MonOp:
    def __call__(self, ind): pass
    def __str__(self): pass

class DetBitFlip(MonOp):
    def __init__(self, nbits=1):
        self.nbits = nbits

    def __str__(self): return '%s(%d)' % (self.__class__.__name__, self.nbits)

    def __call__(self, ind):
        if not ind.size(): return

        print("before", ind.fitness())

        selected = []

        for i in range(self.nbits):
            tmp = None

            while True:
                tmp = random.randint(0, ind.size()-1)
                if tmp not in selected:
                    break

            selected += [tmp]

        for i in selected:
            #yield i
            ind[i] = not ind[i]

        print("after", ind.fitness())

class BitMutation(MonOp):
    def __init__(self, rate=0.01, normalize=False):
        self.rate = rate
        self.normalize = normalize

    def __str__(self): return 'BitMutation(%s,%s)' % (self.rate, self.normalize)

    def __call__(self, ind):
        p = self.rate / ind.size() if self.normalize else self.rate
        for i in range(ind.size()):
            if random.random() < p:
                #yield i
                ind[i] = not ind[i]

class OneBitFlip(DetBitFlip):
    def __init__(self):
        DetBitFlip.__init__(self, 1)

class Evolver(IslandOperator):
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

            #print(ind.fitness(), candidate.fitness())
            #print(ind.size(), candidate.size())

            if candidate.fitness() > ind.fitness():
                ind.copy_from(candidate)

N = 4
P = 10
n = 1
M = 4

init = ZerofyInit(n)
init_matrix = InitMatrix(False,1/N)
__eval = OneMaxFullEval()
reward = Best(.2,0)
updater = Updater(reward)
memorizer = Memorize()

matrix = []
for i in range(N):
    matrix += [[]]
    for j in range(N):
        matrix[-1] += [0]
init_matrix(matrix)
print(matrix)

pop_set = []
data_set = []

for i in range(N):
    pop_set += [Population(P,init)]
    data_set += [IslandData(i,N)]

for i in range(N):
    pop = pop_set[i]
    data = data_set[i]

    data.proba = matrix[i].copy()
    apply(pop, __eval)

    mon = None
    if data.rank == 0:
        mon = BitMutation(1, True)
    else:
        mon = DetBitFlip( (data.rank-1)*2+1 )

    evolver = Evolver(__eval, mon)
    feedbacker = Feedbacker(pop_set, data_set)
    migrator = Migrator(pop_set, data_set)
    cont = MaxGen(M)
    checkpoint = Checkpoint(cont)
    island = Algo(evolver, feedbacker, updater, memorizer, migrator, checkpoint, pop, data)
    island.start()
