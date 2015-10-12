cmake_minimum_required(VERSION 2.8)

project(nal_parser)

set(CMAKE_CXX_STANDARD 11)

if( GCC OR MINGW )
    set(CMAKE_CXX_FLAGS "-Wreturn-type -Werror=return-type")
endif()

set(SOURCES
        main.cpp
        BlockingQueue.h
        Arguments.h
        Chunk.h
        Chunk.cpp
        ChunkReader.h
        ChunkReader.cpp
        NalParser.h
        NalParser.cpp
        XmlDocument.h
        XmlDocument.cpp
        NalUnit.h)

add_executable(${PROJECT_NAME} ${SOURCES})