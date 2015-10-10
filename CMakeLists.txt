cmake_minimum_required(VERSION 2.8)

project(nal_parser)

set(CMAKE_CXX_STANDARD 11)

set(SOURCES main.cpp BlockingQueue.h ProcessingQueue.h Arguments.h Chunk.h ChunkReader.h NalParser.h)

add_executable(${PROJECT_NAME} ${SOURCES})