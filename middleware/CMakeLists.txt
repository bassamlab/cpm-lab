cmake_minimum_required(VERSION 3.1)
add_definitions(-Wall -Wextra -Werror=return-type)
set (CMAKE_CXX_STANDARD 17)

# Fast-DDS
option(THIRDPARTY "" ON)
option(BUILD_TESTING "" OFF)
add_subdirectory(../cpm_lib/thirdparty/Fast-DDS/ build)

# CPM lib
include_directories(SYSTEM ../cpm_lib/include/)
include_directories(SYSTEM ../cpm_lib/include/cpm/dds/)
link_directories(../cpm_lib/build/)


include_directories(src)

set (SOURCES 
    src/Communication.hpp
    src/TypedCommunication.hpp
    src/TypedCommunication.cpp
)

add_executable( middleware
    src/main.cpp
    ${SOURCES}
)

target_link_libraries(middleware cpm)

add_executable(unittest 
    test/catch.cpp
    test/test_goals_to_hlc.cpp
    test/test_hlc_to_vehicle.cpp
    test/test_vehicle_to_middleware.cpp
    test/test_middleware_to_hlc.cpp
    test/test_vehicle_read.cpp
    ${SOURCES}
)

target_link_libraries(unittest cpm fastrtps)