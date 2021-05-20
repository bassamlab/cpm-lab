cmake_minimum_required(VERSION 3.1)
add_definitions(-Wall -Wextra -Werror=return-type -O2)
set (CMAKE_CXX_STANDARD 17)

# Configuration for eProsima Fast-DDS 
# Find requirements
# For this to work, the current (2.3.x) version of Fast-DDS (and CDR, foonathan_memory) must be installed
# Do this by creating a build folder in the thirdparty folders for each of those projects
# There: cmake .., then sudo cmake --build . --target install
if(NOT fastcdr_FOUND)
    find_package(fastcdr REQUIRED)
endif()

if(NOT foonathan_memory_FOUND)
    find_package(foonathan_memory REQUIRED)
endif()

if(NOT fastrtps_FOUND)
    find_package(fastrtps REQUIRED)
endif()

# CPM lib
include_directories(SYSTEM ../../cpm_lib/include/)
include_directories(SYSTEM ../../cpm_lib/include/cpm/dds/)
link_directories(../../cpm_lib/build/)


include_directories(src)

set (SOURCES 
)

add_executable( autostart
    src/main.cpp
    ${SOURCES}
)

target_link_libraries(autostart cpm fastrtps)

add_executable( test_receiver
    src/test_receiver.cpp
    ${SOURCES}
)

target_link_libraries(test_receiver cpm fastrtps)

add_executable( download_error_logger
    src_error_logger/main.cpp
    ${SOURCES}
)

target_link_libraries(download_error_logger stdc++fs cpm fastrtps)