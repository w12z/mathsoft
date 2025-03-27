cmake_minimum_required(VERSION 3.10)
project(MandelbrotGPU)

set(CMAKE_CXX_STANDARD 14)

find_package(OpenCV REQUIRED)
find_package(OpenCL REQUIRED)


include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(Test mandelbrot.cpp)
target_link_libraries(Test ${OpenCV_LIBS} OpenCL::OpenCL)
target_include_directories(Test PRIVATE ${OpenCL_INCLUDE_DIRS})

