cmake_minimum_required(VERSION 3.16)
project(ModernParkingSystem)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(parking_system
    src/main.cpp
    src/ParkingSystem.cpp
)
