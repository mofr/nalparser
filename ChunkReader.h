#pragma once

#include "Chunk.h"

class ChunkReader
{
public:
    ChunkReader() {}
    ~ChunkReader();

    bool open(const char * filename);
    std::shared_ptr<Chunk> readNext();

private:
    std::shared_ptr<Chunk> readChunk();

private:
    FILE * file = nullptr;
    long offset = 0;
    std::shared_ptr<Chunk> previous;

    static const int ChunkSize = 256*1024;
};
