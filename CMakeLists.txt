cmake_minimum_required(VERSION 2.8)

project(nal_parser)

#add_definitions("-std=c++11")

set(SOURCES main.cpp BlockingQueue.h)

add_executable(${PROJECT_NAME} ${SOURCES})
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 11)