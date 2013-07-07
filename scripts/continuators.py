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

import stats
from decorators import *
import logging

logger = logging.getLogger("dim.continuators")

class Base:
    def addTo(self, cp):
        cp.conts += [self]
        return self

    def __call__(self, pop, data): pass
    def lastcall(self, pop, data): pass

class Combined(Base, list):
    def __init__(self, cont):
        self += [cont]

    def add(self, cont):
        self += [cont]

    @returns(bool)
    def __call__(self, pop, data):
        for cont in self:
            if not cont(pop, data): return False
        return True

class Checkpoint(Base):
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

class MaxGen(Base, stats.Base):
    def __init__(self, maxgen=100):
        stats.Base.__init__(self)
        self.maxgen = maxgen

    @returns(bool)
    def __call__(self, pop, data):
        if self < self.maxgen:
            self += 1
            return True

        logger.warning("STOP in MaxGen: maximum number of generation has reached %s" % self)
        self.set(0)
        return False

class Fit(Base):
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

if __name__ == "__main__":
    import doctest
    doctest.testmod()
