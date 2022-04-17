cmake_minimum_required( VERSION 2.8 )
project ( iot_module )

# set(CMAKE_C_COMPILER "gcc")
# set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_C_FLAGS "-std=c99")

include_directories("./")

set(LIBRARY_OUTPUT_PATH ./libs)
set(EXECUTABLE_OUTPUT_PATH ./bin)

# Add block directories
add_subdirectory(softap)
add_subdirectory(hal)
add_subdirectory(example/ap_config)
add_subdirectory(thirdlib/cJSON)

