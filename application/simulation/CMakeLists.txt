ADD_EXECUTABLE(simulation simulation.cpp)
TARGET_LINK_LIBRARIES(simulation ${PROJECT_LIB} boost_mpi_shared ${Boost_LIBRARIES} ${EO_LIBRARIES})
