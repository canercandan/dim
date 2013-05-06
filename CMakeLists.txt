######################################################################################
### 1) Set the application properties
######################################################################################

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(dim)

# Current version
SET(PROJECT_VERSION_MAJOR 0)
SET(PROJECT_VERSION_MINOR 1)
SET(PROJECT_VERSION_PATCH 0)
SET(PROJECT_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")

IF(POLICY CMP0017)
  CMAKE_POLICY(SET CMP0017 OLD) # or even better, NEW
ENDIF(POLICY CMP0017)

INCLUDE(dim-conf.cmake OPTIONAL)

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

ADD_DEFINITIONS(-DDEPRECATED_MESSAGES) # disable warning deprecated function messages
ADD_DEFINITIONS(-DWITH_MPI)
ADD_DEFINITIONS(-DWITH_BOOST)

FIND_PACKAGE(EO COMPONENTS ga mpi serial)
FIND_PACKAGE(Boost COMPONENTS serialization system chrono thread)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src ${EO_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS} contrib)
LINK_DIRECTORIES(${EO_LIBRARY_DIRS} ${Boost_LIBRARY_DIRS})

######################################################################################


######################################################################################
### 4) Set compiler definitions
######################################################################################

# ADD_DEFINITIONS(-DHAVE_GNUPLOT -DHAVE_MATPLOT)

IF(UNIX)
  # enable warnings
  # ADD_DEFINITIONS( -Wall -W -Wextra )
  # ADD_DEFINITIONS( -Weffc++)
  # ADD_DEFINITIONS( -g3 )
  # ADD_DEFINITIONS(-std=c++11 -D_GLIBCXX_USE_NANOSLEEP)
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

IF(ENABLE_STATIC_LIBRARY)
  SET(PROJECT_LIB "${PROJECT_NAME}_static")
ELSE()
  SET(PROJECT_LIB "${PROJECT_NAME}_shared")
ENDIF()

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
