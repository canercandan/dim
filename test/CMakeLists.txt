SET(SOURCES
  t-test
  t-Thread
  t-mpi-persistent
  t-boost-mpi-thread
  t-boost-mpi-thread-multi
  t-boost-mpi-thread-multi-with-trace
  t-boost-mpi-thread-multi-nonblocking-count
  t-boost-mpi-thread-multi-thread-recv
  t-stack
  t-multithreaded-comm
  t-multithreaded-comm-boost-mpi
  )

# SET(CMAKE_CXX_COMPILER ${MPI_CXX_COMPILER})

LINK_LIBRARIES(${MPI_CXX_LIBRARIES} boost_mpi ${Boost_LIBRARIES} ${EO_LIBRARIES} ${PROJECT_NAME})

FOREACH(current ${SOURCES})
  ADD_EXECUTABLE(${current} ${current}.cpp)
  ADD_TEST(${current} ${current})
  # TARGET_LINK_LIBRARIES(${current} ${EO_LIBRARIES} ${MPI_CXX_LIBRARIES})
  # TARGET_LINK_LIBRARIES(${current} ${EO_LIBRARIES} ${PYTHON_LIBRARIES})
ENDFOREACH()
