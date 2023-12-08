cmake_minimum_required(VERSION 3.0.0)
project(optical-flow VERSION 0.1.0 LANGUAGES C CXX)

include(CTest)
enable_testing()

find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )

add_executable(optical-flow 
    homebrewed.cpp
    optical_flow.cpp
    motor_control.cpp
)

target_link_libraries( optical-flow ${OpenCV_LIBS} )

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
