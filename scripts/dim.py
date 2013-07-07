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

"""
This is the Dynamic Islands Model module.

This module supplies several classes among problem representations, population, problem initializer, evaluation, statistic generator, checkpoint, variation operators, island data representations as well as DIM main classes like evolver, feedbacker, migrator, updater and the main algorithm.
"""

import numpy as np
import logging, sys
import decorators
import core
import stats
import continuators
import monitors
import ops
import eval
import vectorupdater
import init
import memorizer
import evolver
import feedbacker
import migrator

from parser import Parser
import numpy as np
from numpy import *
import logging, sys
from decorators import *
from core import *
from stats import *
from continuators import *
from monitors import *
from ops import *
from eval import *
from vectorupdater import *
from init import *
from memorizer import *
from evolver import *
from feedbacker import *
from migrator import *


logger = logging.getLogger("dim")

if __name__ == "__main__":
    import doctest
    doctest.testmod()
