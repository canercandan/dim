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

import numpy as np
from numpy import *

class Base:
    def __init__(self, size): self.size = size
    def __call__(self, ind): pass

class Zerofy(Base):
    """Set all the bits of the solution to false.

    >>> ind = Individual()
    >>> init = Zerofy(10)
    >>> init(ind)
    >>> ind
    [0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    """

    def __init__(self, size): Base.__init__(self, size)
    def __call__(self, ind): ind += [0] * self.size

class Defined(Base):
    """Set a defined number of bits to true and the rest to false.

    >>> ind = Individual()
    >>> init = Defined(10,5)
    >>> init(ind)
    >>> ind
    [0, 0, 0, 0, 0, 1, 1, 1, 1, 1]
    """

    def __init__(self, size, defined_size):
        Base.__init__(self, size)
        self.defined_size = defined_size

    def __call__(self, ind):
        ind += [0] * (self.size-self.defined_size) + [1] * self.defined_size

class Random(Base):
    """Set all the bits of solution to a random value.

    >>> random.seed(0)
    >>> ind = Individual()
    >>> init = Random(10)
    >>> init(ind)
    >>> ind
    [0, 1, 1, 0, 1, 1, 1, 1, 1, 1]
    """

    def __init__(self, size): Base.__init__(self, size)
    def __call__(self, ind): ind += random.choice(2,size=self.size)

if __name__ == "__main__":
    import doctest
    doctest.testmod()
