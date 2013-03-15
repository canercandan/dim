ADD_EXECUTABLE(simu-threaded threaded.cpp)
TARGET_LINK_LIBRARIES(simu-threaded ${PROJECT_NAME}_shared boost_mpi_shared ${Boost_LIBRARIES} ${EO_LIBRARIES})
