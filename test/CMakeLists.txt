IF(ENABLE_CMAKE_TESTING)

  SET(SOURCES
    t-test
    t-Thread-args
    t-Thread-args2
    t-Thread-args3
    t-mpi-persistent
    #t-boost-mpi-thread
    #t-boost-mpi-thread-multi
    #t-boost-mpi-thread-multi-with-trace
    #t-boost-mpi-thread-multi-nonblocking-count
    #t-boost-mpi-thread-multi-thread-recv
    t-stack
    t-multithreaded-comm
    t-multithreaded-comm-boost-mpi
    t-multithreaded-comm-boost-mpi-functor
    )

  LINK_LIBRARIES(boost_mpi_shared ${EO_LIBRARIES} ${Boost_LIBRARIES} ${PROJECT_NAME}_shared)

  FOREACH(current ${SOURCES})
    ADD_EXECUTABLE(${current} ${current}.cpp)
    ADD_TEST(${current} ${current})
  ENDFOREACH()

ENDIF()
