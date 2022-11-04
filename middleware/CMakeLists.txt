cmake_minimum_required(VERSION 3.1)
add_definitions(-Wall -Wextra -Werror=return-type -O2)
set (CMAKE_CXX_STANDARD 20)

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

target_link_libraries(middleware cpm fastrtps fastcdr foonathan_memory)

add_executable(unittest 
    test/catch.cpp
    test/test_goals_to_hlc.cpp
    test/test_hlc_to_vehicle.cpp
    test/test_vehicle_to_middleware.cpp
    test/test_middleware_to_hlc.cpp
    test/test_vehicle_read.cpp
    ${SOURCES}
)

target_link_libraries(unittest cpm fastrtps fastcdr foonathan_memory)