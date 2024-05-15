cmake_minimum_required(VERSION 3.25)
project(HanZip)

set(CMAKE_CXX_STANDARD 23)

set(SOURCES ./src/main.cpp)
set(HEADERS include/Fract.h)
include_directories(include)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})
