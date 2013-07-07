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

logger = logging.getLogger("dim.migrator")

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

if __name__ == "__main__":
    import doctest
    doctest.testmod()
