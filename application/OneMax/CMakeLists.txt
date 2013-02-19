ADD_EXECUTABLE(onemax-nonthreaded nonthreaded.cpp)
TARGET_LINK_LIBRARIES(onemax-nonthreaded ${PROJECT_NAME} ${MPI_CXX_LIBRARIES} boost_mpi ${Boost_LIBRARIES} ${EO_LIBRARIES})

ADD_EXECUTABLE(onemax-threaded threaded.cpp)
TARGET_LINK_LIBRARIES(onemax-threaded ${PROJECT_NAME} ${MPI_CXX_LIBRARIES} boost_mpi ${Boost_LIBRARIES} ${EO_LIBRARIES})
