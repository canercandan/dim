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

from core import *
from datetime import datetime as dt
import logging

logger = logging.getLogger("dim.stats")

class Base(MutableValue):
    def addTo(self, cp):
        cp.stats += [self]
        return self

    def __call__(self, pop, data): pass
    def lastcall(self, pop, data): pass

class IslandRank(Base):
    def __call__(self, pop, data): self.set(data.rank)

class ElapsedTime(Base):
    def __init__(self): self.start = dt.now()
    def __call__(self, pop, data): self.set(round((dt.now() - self.start).total_seconds(), 2))

class ElapsedTimeBetweenGenerations(Base):
    def __init__(self): self.start = dt.now()
    def __call__(self, pop, data):
        now = dt.now()
        self.set(round((now - self.start).total_seconds(), 2))
        self.start = now

class Generation(Base):
    def __call__(self, pop, data): self += 1

class PopSize(Base):
    def __call__(self, pop, data): self.set(pop.size())

class AverageFitness(Base):
    def __call__(self, pop, data):
        if pop.size(): self.set(round(mean([x.fitness() for x in pop]), 2))

class BestFitness(Base):
    def __call__(self, pop, data):
        if pop.size(): self.set(pop.best_element().fitness())

class BestOfBestFitness(Base):
    def __init__(self, pop_set, data_set):
        self.pop_set = pop_set
        self.data_set = data_set

    def __call__(self, pop, data):
        self.set( max([p.best_element().fitness() if p.size() else 0 for p in self.pop_set]) )

class Probabilities(Base):
    def __call__(self, pop, data): self.set( (array(data.proba)).round(1) )

class CommingProbabilities(Base):
    def __init__(self, pop_set, data_set):
        self.pop_set = pop_set
        self.data_set = data_set

    def __call__(self, pop, data):
        self.set( (sum([d.proba for d in self.data_set], axis=0)/data.size).round(1) )

class SumOfGoingProbabilities(Base):
    def __call__(self, pop, data): self.set( sum(data.proba) )

class SumOfCommingProbabilities(Base):
    def __init__(self, pop_set, data_set):
        self.pop_set = pop_set
        self.data_set = data_set

    def __call__(self, pop, data):
        self.set( round(sum([d.proba[data.rank] for d in self.data_set])/(data.size*100)*100, 2) )

class Feedbacks(Base):
    @DataShortcuts
    def __call__(self, pop, data, F, P, fq, mq, k, n): self.set(F.round(2))

class NormalizedFeedbacks(Base):
    def __call__(self, pop, data): F = data.feedbacks.copy(); self.set(F/F.sum()*100)

class BestFeedbacks(Base):
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

if __name__ == "__main__":
    import doctest
    doctest.testmod()
