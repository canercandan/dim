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

import sys
from datetime import datetime as dt
import logging

logger = logging.getLogger("dim.monitors")

class Base(list):
    def add(self, stat): self += [stat]

    def addTo(self, cp):
        cp.monitors += [self]
        return self

    def __call__(self): pass
    def lastcall(self): pass

class Print(Base):
    def __init__(self, out=sys.stdout, delim="\t", stepTimer=0):
        self.out = out
        self.delim = delim
        self.stepTimer = stepTimer
        self.firstTime = True
        self.start = dt.now()

    def __call__(self):
        if not self.out:
            raise ValueError("OStreamMonitor: Could not write to the output stream")

        if not len(self): return

        if self.stepTimer:
            now = dt.now()
            elapsed = int( ( (now - self.start).total_seconds() * 1000 ) / self.stepTimer )
            if not elapsed: return
            self.start = now

        for stat in self:
            self.out.write( "%s%s" % (stat.__class__.__name__ if self.firstTime else stat, self.delim) )
        self.out.write("\n")
        self.out.flush()

        if self.firstTime: self.firstTime = False

if __name__ == "__main__":
    import doctest
    doctest.testmod()
