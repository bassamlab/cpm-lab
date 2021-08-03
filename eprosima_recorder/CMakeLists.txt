cmake_minimum_required(VERSION 3.1)
add_definitions(-Wall -Wextra -Werror=return-type)
set (CMAKE_CXX_STANDARD 17)

add_definitions(-Wall -Wextra -Werror=return-type -O2)

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

SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -m64 -Wl,--no-as-needed")

# CPM lib
include_directories(SYSTEM ../cpm_lib/include/)
include_directories(SYSTEM ../cpm_lib/include/cpm/dds/)
link_directories(SYSTEM ../cpm_lib/build/)

include_directories(src)
include_directories(include)

set(SOURCE_EPROSIMA_RECORDER
    include/PublishedTopicsListener.hpp
    src/PublishedTopicsListener.cpp
    include/RecorderManager.hpp
    include/TopicRecorder.hpp
    
    src/main.cpp
)

add_executable(EprosimaRecorder
    ${SOURCE_EPROSIMA_RECORDER}
)

target_link_libraries(EprosimaRecorder cpm fastrtps fastcdr foonathan_memory)