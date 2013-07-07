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

logger = logging.getLogger("dim.evolver")

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

if __name__ == "__main__":
    import doctest
    doctest.testmod()
