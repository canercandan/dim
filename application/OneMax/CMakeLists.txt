ADD_EXECUTABLE(nonthreaded nonthreaded.cpp)
TARGET_LINK_LIBRARIES(nonthreaded ${PROJECT_NAME} ${MPI_CXX_LIBRARIES} boost_mpi ${Boost_LIBRARIES} ${EO_LIBRARIES})
