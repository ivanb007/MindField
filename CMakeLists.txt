cmake_minimum_required(VERSION 3.23)

project(
    MindField 
    VERSION 0.1.0 
    LANGUAGES C CXX)

add_executable(MindField engine.cpp main.cpp search.cpp threadpool.cpp uci.cpp)

include(CTest)
enable_testing()

