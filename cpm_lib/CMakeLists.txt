cmake_minimum_required(VERSION 3.5)
project(cpmlib)
option(BUILD_ARM OFF)

set (CMAKE_CXX_STANDARD 11)
add_definitions(-Wall -Wextra -Werror=return-type -O2)
include_directories(src)
include_directories(include)

file(GLOB SOURCE_DDS "dds_idl_cpp/*")

set(SOURCE_CPM
    src/ParticipantSingleton.cpp
    include/cpm/ParticipantSingleton.hpp

        
    include/cpm/ParticipantSingleton.hpp
    src/ParticipantSingleton.cpp
    
    include/cpm/Participant.hpp
    src/Participant.cpp

    include/cpm/ReaderParent.hpp
    include/cpm/AsyncReader.hpp

    include/cpm/Parameter.hpp
    include/cpm/ParameterReceiver.hpp
    src/ParameterReceiver.cpp

    
    include/cpm/CommandLineReader.hpp
    src/CommandLineReader.cpp

    include/cpm/type_creation_helper.hpp
    src/type_creation_helper.cpp

    include/cpm/Reader.hpp
    include/cpm/ReaderAbstract.hpp
    include/cpm/Writer.hpp
    include/cpm/MultiVehicleReader.hpp

    include/cpm/Timer.hpp
    src/Timer.cpp

    include/cpm/stamp_message.hpp
    
    include/cpm/TimerFD.hpp
    src/TimerFD.cpp
    include/cpm/SimpleTimer.hpp
    src/SimpleTimer.cpp
    src/TimerSimulated.hpp
    src/TimerSimulated.cpp
    include/cpm/exceptions.hpp
    src/exceptions.cpp
    

    src/Logging.cpp
    include/cpm/Logging.hpp
    include/cpm/InternalConfiguration.hpp
    src/InternalConfiguration.cpp
    include/cpm/init.hpp
    include/cpm/get_time_ns.hpp
    src/get_time_ns.cpp
    include/cpm/RTTTool.hpp
    src/RTTTool.cpp
    include/cpm/TimeMeasurement.hpp
    src/TimeMeasurement.cpp
)


include_directories(dds_idl_cpp)

if(BUILD_ARM)
    # eProsima gets compiled together with the ARM files
    option(THIRDPARTY "" ON)
    option(BUILD_TESTING "" OFF)
    add_subdirectory(thirdparty/Fast-DDS/)

    add_definitions(-fpic -DRTI_UNIX -DRTI_LINUX -march=armv6 -mfpu=vfp -mfloat-abi=hard -mlong-calls)
else()
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
endif()


add_library(cpm SHARED
    ${SOURCE_DDS}
    ${SOURCE_CPM}
)

#include_directories(${CMAKE_SOURCE_DIR}/build/thirdparty/Fast-DDS/include)
#include_directories(${CMAKE_SOURCE_DIR}/thirdparty/Fast-DDS/include)

if(BUILD_ARM)
    target_link_libraries(cpm fastrtps fastcdr)
else()
    target_link_libraries(cpm fastrtps fastcdr foonathan_memory)
endif()


if(NOT BUILD_ARM)
    add_executable(unittest
        test/catch.cpp
        test/test_logging.cpp
        test/test_AsyncReader.cpp
        test/test_rtt.cpp
        test/test_parameter.cpp
        test/test_simple_timer.cpp
        test/test_simple_timer_stop.cpp
        test/test_timer.cpp
        test/test_timer_stop.cpp
        test/test_timer_stop_running.cpp
        test/test_timer_start_again.cpp
        test/test_timer_simulated.cpp
        test/test_VehicleIDFilteredTopic.cpp
        test/test_Participant.cpp
        test/test_Reader.cpp
        test/test_ReaderAbstract.cpp
        test/test_Writer.cpp
        test/test_MultiVehicleReader.cpp
        test/test_CommandLineReader.cpp
        test/test_InternalConfiguration.cpp

    )

    target_link_libraries(unittest cpm fastrtps fastcdr foonathan_memory)

    add_executable(SHM_READER
        test_apps/test_shm_reader.cpp
    )
    target_link_libraries(SHM_READER cpm fastrtps)

    add_executable(SHM_WRITER
        test_apps/test_shm_writer.cpp
    )
    target_link_libraries(SHM_WRITER cpm fastrtps)

    add_executable(MatlabEprosimaReceiveTest
        test_apps/MatlabEprosimaReceiveTest.cpp
    )

    target_link_libraries(MatlabEprosimaReceiveTest cpm fastrtps fastcdr foonathan_memory)

    add_executable(MatlabEprosimaWriteTest
        test_apps/MatlabEprosimaWriteTest.cpp
    )

    target_link_libraries(MatlabEprosimaWriteTest cpm fastrtps fastcdr foonathan_memory)

    add_executable(EprosimaTopicListener
        test_apps/EprosimaTopicListener.cpp
    )

    target_link_libraries(EprosimaTopicListener cpm fastrtps fastcdr foonathan_memory)
endif()