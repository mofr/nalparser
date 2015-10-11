#pragma once

#include "Chunk.h"

class ChunkReader
{
public:
    ChunkReader()
    { }

    ~ChunkReader()
    {
        if(file)
        {
            fclose(file);
        }
    }

    bool open(const char * filename)
    {
        file = fopen(filename, "rb");
        return file != nullptr;
    }

    std::shared_ptr<Chunk> readNext()
    {
        std::shared_ptr<Chunk> chunk = readChunk();
        if(previous)
        {
            previous->setNext(chunk);
        }
        previous = chunk;
        return chunk;
    }

private:
    std::shared_ptr<Chunk> readChunk()
    {
        if(feof(file))
        {
            return nullptr;
        }

        std::shared_ptr<Chunk> chunk(new Chunk(ChunkSize));
        chunk->size = fread(chunk->data, 1, ChunkSize, file);
        chunk->offset = offset;
        offset += chunk->size;
        return chunk;
    }

private:
    FILE * file = nullptr;
    long offset = 0;
    std::shared_ptr<Chunk> previous;

    static const int ChunkSize = 256*1024;
};
