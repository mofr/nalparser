cmake_minimum_required(VERSION 2.8)

project(nalparser)

find_package(Threads)

set(CMAKE_CXX_STANDARD 14)

if( GCC OR MINGW )
    set(CMAKE_CXX_FLAGS "-Wreturn-type -Werror=return-type")
endif()

set(SOURCES
        main.cpp
        Arguments.h
        Configuration.h Configuration.cpp
        Chunk.h Chunk.cpp
        ChunkReader.h ChunkReader.cpp
        BlockingQueue.h
        NalUnit.h
        NalUnitIterator.h NalUnitIterator.cpp
        NalParser.h NalParser.cpp
        XmlDocument.h XmlDocument.cpp)

add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT})
