cmake_minimum_required(VERSION 3.26)
PROJECT(clusterServer)

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -g)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/include/server)
include_directories(${PROJECT_SOURCE_DIR}/thirdpart)
include_directories(${PROJECT_SOURCE_DIR}/include/server/db)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

add_subdirectory(src)