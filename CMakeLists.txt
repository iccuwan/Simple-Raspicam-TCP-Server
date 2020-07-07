# CMakeList.txt : CMake project for Drone, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project ("Drone")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add source to this project's executable.
add_executable (Drone "Drone.cpp" "Drone.h")

# TODO: Add tests and install targets if needed.
target_link_libraries(Drone)