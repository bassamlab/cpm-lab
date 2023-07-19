cmake_minimum_required(VERSION 3.1)
add_definitions(-Wall -Wextra -Werror=return-type -Wno-unknown-pragmas)
set (CMAKE_CXX_STANDARD 17)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/cmake" CACHE STRING "Modules for CMake" FORCE)

# OpenCV
set(OpenCV_LIBS_IN calib3d core features2d flann highgui imgcodecs imgproc ml objdetect photo stitching videoio video)
find_package(OpenCV 4.0.0 REQUIRED ${OpenCV_LIBS_IN})
link_libraries(${OpenCV_LIBS})

# Pylon
find_package(Pylon REQUIRED)
include_directories(${PYLON_INCLUDE_DIRS})
message("-- found Pylon Version ${PYLON_VERSION}")


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
include_directories(SYSTEM ../cpm_lib/include/)
include_directories(SYSTEM ../cpm_lib/include/cpm/dds/)
link_directories(../cpm_lib/build/)



include_directories(src)
link_libraries(dl nsl m pthread rt)



add_executable(BaslerLedDetection
    src/main_led_detection.cpp
)
target_link_libraries(BaslerLedDetection cpm ${OpenCV_LIBRARIES} ${PYLON_LIBRARIES} fastrtps fastcdr foonathan_memory)



add_executable(ips_pipeline
    src/main_ips_pipeline.cpp
    src/IpsPipeline.cpp
    src/IpsPipeline.hpp
    src/UndistortPoints.cpp
    src/UndistortPoints.hpp
    src/DetectVehicles.cpp
    src/DetectVehicles.hpp
    src/DetectVehicleID.cpp
    src/DetectVehicleID.hpp
    src/PoseCalculation.cpp
    src/PoseCalculation.hpp
)
target_link_libraries(ips_pipeline cpm ${OpenCV_LIBRARIES} fastrtps fastcdr foonathan_memory)


add_executable(unittest
    test/catch.cpp
    test/catch.hpp
    test/test_DetectVehicles.cpp
    test/test_DetectVehicleID.cpp
    src/DetectVehicles.cpp
    src/DetectVehicles.hpp
    src/DetectVehicleID.cpp
    src/DetectVehicleID.hpp
)
target_link_libraries(unittest cpm ${OpenCV_LIBRARIES} fastrtps fastcdr foonathan_memory)