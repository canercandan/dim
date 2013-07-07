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
import logging

logger = logging.getLogger("dim.feedbacker")

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

if __name__ == "__main__":
    import doctest
    doctest.testmod()
