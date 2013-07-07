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

logger = logging.getLogger("dim.vectorupdater")

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

if __name__ == "__main__":
    import doctest
    doctest.testmod()
