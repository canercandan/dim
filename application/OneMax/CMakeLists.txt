ADD_EXECUTABLE(onemax-nonthreaded nonthreaded.cpp)
TARGET_LINK_LIBRARIES(onemax-nonthreaded ${PROJECT_LIB} boost_mpi_shared ${Boost_LIBRARIES} ${EO_LIBRARIES})
