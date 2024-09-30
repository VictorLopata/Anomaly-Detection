cmake_minimum_required(VERSION 3.10)
project(Anomaly-Detection)

set(CMAKE_CXX_STANDARD 11)

add_executable(Anomaly-Detection src/main.cpp
        src/Calculator.cpp

)

add_executable(prova src/con2db/main.cpp
        src/con2db/pgsql.cpp)
