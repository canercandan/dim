######################################################################################
### 1) Set the application properties
######################################################################################

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(dim)

SET(PROJECT_VERSION_MAJOR 0)
SET(PROJECT_VERSION_MINOR 1)
SET(PROJECT_VERSION_PATCH 0)
SET(PROJECT_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")

IF(POLICY CMP0017)
  CMAKE_POLICY(SET CMP0017 OLD) # or even better, NEW
ENDIF(POLICY CMP0017)

######################################################################################


######################################################################################
### 2) Include useful features
######################################################################################

SET(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)

######################################################################################


######################################################################################
### 3) Find and include useful dependencies
######################################################################################

FIND_PACKAGE(MPI)

SET(CMAKE_CXX_COMPILER "${MPI_CXX_COMPILER}")

# FIND_PACKAGE(OpenMP)
# IF(OPENMP_FOUND)
#   SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
#   SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
# ENDIF()

ADD_DEFINITIONS(-DDEPRECATED_MESSAGES) # disable warning deprecated function messages
ADD_DEFINITIONS(-DWITH_MPI)
ADD_DEFINITIONS(-DWITH_BOOST)
FIND_PACKAGE(EO COMPONENTS ga mpi serial)
INCLUDE_DIRECTORIES(${EO_INCLUDE_DIRS})
LINK_DIRECTORIES(${EO_LIBRARY_DIRS})

# FIND_PACKAGE(Gnuplot)
# FIND_PACKAGE(PythonLibs)
# INCLUDE_DIRECTORIES(${PYTHON_INCLUDE_DIRS})

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src)

# SET(Boost_USE_STATIC_LIBS ON)
FIND_PACKAGE(Boost COMPONENTS serialization)

INCLUDE_DIRECTORIES(
  ${Boost_INCLUDE_DIRS}
  contrib
  )

LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})

######################################################################################


######################################################################################
### 4) Set compiler definitions
######################################################################################

# ADD_DEFINITIONS(-DHAVE_GNUPLOT -DHAVE_MATPLOT)

IF(UNIX)
  # enable warnings
  ADD_DEFINITIONS( -Wall -W -Wextra )
  # ADD_DEFINITIONS( -Weffc++)
  # ADD_DEFINITIONS( -g3 )
  ADD_DEFINITIONS(-std=c++11)
ENDIF()

######################################################################################


######################################################################################
### 5) Prepare some variables for CMAKE usage
######################################################################################

SET(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)
LINK_DIRECTORIES(${LIBRARY_OUTPUT_PATH})

IF (ENABLE_CMAKE_TESTING)
  ENABLE_TESTING()
ENDIF (ENABLE_CMAKE_TESTING)

######################################################################################


######################################################################################
### 6) Now where we go ?
######################################################################################

ADD_SUBDIRECTORY(contrib)
ADD_SUBDIRECTORY(src)
ADD_SUBDIRECTORY(application)
ADD_SUBDIRECTORY(test)
ADD_SUBDIRECTORY(doc)

######################################################################################
