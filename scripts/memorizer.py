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

logger = logging.getLogger("dim.memorizer")

class Memorize(IslandOperator):
    @DataShortcuts
    def firstCall(self, pop, data, F, P, fq, mq, k, n):
        for ind in pop:
            ind.set_last_island(k)

    @DataShortcuts
    def __call__(self, pop, data, F, P, fq, mq, k, n):
        for ind in pop:
            ind.set_last_island(k)

if __name__ == "__main__":
    import doctest
    doctest.testmod()
