cmake_minimum_required(VERSION 3.15)
project(ImageScaling)
set(CMAKE_CXX_STANDARD 17)

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(scaling main.cpp
        zoom.cpp
        shrink.cpp
        zoom.h
        shrink.h)
target_link_libraries(scaling ${OpenCV_LIBS})