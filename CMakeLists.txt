cmake_minimum_required(VERSION 3.31.0)
project(coin-recognition VERSION 0.1.0 LANGUAGES C CXX)

add_executable(coin-recognition coin-detection.cc)

include(CTest)
enable_testing()

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

target_link_libraries(coin-recognition ${OpenCV_LIBS})


set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
