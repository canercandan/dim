# File: FindParadisEO-problems.cmake
# CMAKE commands to actually use the ParadisEO-problems library
# Version: 0.0.1
#
# The following variables are filled out:
# - ParadisEO-problems_INCLUDE_DIRS
# - ParadisEO-problems_FOUND
#

IF(NOT ParadisEO-problems_INCLUDE_DIRS)
  FIND_PATH(
    ParadisEO-problems_INCLUDE_DIRS
    problems.h
    PATHS
    /usr/include/paradiseo-problems
    /usr/local/include/paradiseo-problems
    /usr/include/paradiseo-problems/src
    )
ENDIF(NOT ParadisEO-problems_INCLUDE_DIRS)

IF(ParadisEO-problems_INCLUDE_DIRS)
  SET(ParadisEO-problems_FOUND 1)
  MARK_AS_ADVANCED(ParadisEO-problems_FOUND)
  MARK_AS_ADVANCED(ParadisEO-problems_INCLUDE_DIRS)
ENDIF(ParadisEO-problems_INCLUDE_DIRS)
