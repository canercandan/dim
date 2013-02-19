ADD_EXECUTABLE(simu-threaded threaded.cpp)
TARGET_LINK_LIBRARIES(simu-threaded ${PROJECT_NAME} ${MPI_CXX_LIBRARIES} boost_mpi ${Boost_LIBRARIES} ${EO_LIBRARIES})
