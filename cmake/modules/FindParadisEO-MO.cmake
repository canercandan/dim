# File: FindParadisEO-MO.cmake
# CMAKE commands to actually use the ParadisEO-MO library
# Version: 0.0.1
#
# The following variables are filled out:
# - ParadisEO-MO_INCLUDE_DIRS
# - ParadisEO-MO_FOUND
#

IF(NOT ParadisEO-MO_INCLUDE_DIRS)
  FIND_PATH(
    ParadisEO-MO_INCLUDE_DIRS
    mo.h
    PATHS
    /usr/include/paradiseo-mo
    /usr/local/include/paradiseo-mo
    )
ENDIF(NOT ParadisEO-MO_INCLUDE_DIRS)

# handle the QUIETLY and REQUIRED arguments and set EO_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE(${CMAKE_CURRENT_LIST_DIR}/FindPackageHandleStandardArgs.cmake)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(ParadisEO-MO
  REQUIRED_VARS ParadisEO-MO_INCLUDE_DIRS
  )

MARK_AS_ADVANCED(ParadisEO-MO_INCLUDE_DIRS)
