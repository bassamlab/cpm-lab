cmake_minimum_required(VERSION 3.1)
add_definitions(-g -Wall -Wextra -Werror=return-type)
set (CMAKE_CXX_STANDARD 17)

# Fast DDS / eProsima
if(NOT fastcdr_FOUND)
    find_package(fastcdr REQUIRED)
endif()

if(NOT foonathan_memory_FOUND)
    find_package(foonathan_memory REQUIRED)
endif()

if(NOT fastrtps_FOUND)
    find_package(fastrtps REQUIRED)
endif()

SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -m64 -Wl,--no-as-needed")

# CPM lib
include_directories(SYSTEM ../../../../cpm_lib/include/)
include_directories(SYSTEM ../../../../cpm_lib/include/cpm/dds/)
link_directories(../../../../cpm_lib/build/)


# exe
include_directories(src)
link_libraries(dl nsl m pthread rt)

add_executable(distributed_routing
    src/main.cpp
    include/lane_graph_one_lane/lane_graph.hpp
    src/lane_graph_tools.hpp
    src/lane_graph_tools.cpp
    src/VehicleTrajectoryPlanningState.hpp
    src/VehicleTrajectoryPlanningState.cpp
    src/VehicleTrajectoryPlanner.hpp
    src/VehicleTrajectoryPlanner.cpp
    src/CouplingGraph.cpp
    src/CouplingGraph.hpp
)
target_include_directories(distributed_routing PUBLIC
    include/lane_graph_full)
target_link_libraries(distributed_routing cpm fastrtps fastcdr foonathan_memory)

add_executable(tests
    src/tests.cpp
    include/lane_graph_full/lane_graph.hpp
    src/lane_graph_tools.hpp
    src/lane_graph_tools.cpp
)
target_include_directories(tests PUBLIC
    include/lane_graph_full)
target_link_libraries(tests cpm fastrtps fastcdr foonathan_memory)
