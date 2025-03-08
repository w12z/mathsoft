cmake_minimum_required(VERSION 3.10)
project(YourProjectName)

set(CMAKE_CXX_STANDARD 14)

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS} ${TBB_INCLUDE_DIRS})
find_package(TBB REQUIRED)
include_directories()

add_executable(Test mandelbrot.cpp)
target_link_libraries(Test ${OpenCV_LIBS} TBB::tbb)

