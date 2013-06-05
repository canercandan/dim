# ADD_EXECUTABLE(tsp-nonthreaded nonthreaded.cpp)
# TARGET_LINK_LIBRARIES(tsp-nonthreaded ${PROJECT_LIB} boost_mpi_shared ${Boost_LIBRARIES} ${EO_LIBRARIES})

# ADD_EXECUTABLE(tsp-threaded threaded.cpp)
# TARGET_LINK_LIBRARIES(tsp-threaded ${PROJECT_LIB} boost_mpi_shared ${Boost_LIBRARIES} ${EO_LIBRARIES})

ADD_EXECUTABLE(tsp tsp.cpp)
TARGET_LINK_LIBRARIES(tsp ${PROJECT_LIB} boost_mpi_shared ${Boost_LIBRARIES} ${EO_LIBRARIES})

EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/benchs ${CMAKE_CURRENT_BINARY_DIR}/benchs)
