cmake_minimum_required(VERSION 3.1)
add_definitions(-Wall -Wextra -Werror=return-type -O2)
set (CMAKE_CXX_STANDARD 17)

# FastRTPS
option(THIRDPARTY "" ON)
option(BUILD_TESTING "" OFF)
add_subdirectory(../../cpm_lib/thirdparty/Fast-DDS/ build)

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