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

import logging

logger = logging.getLogger("dim.algo")

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

if __name__ == "__main__":
    import doctest
    doctest.testmod()
