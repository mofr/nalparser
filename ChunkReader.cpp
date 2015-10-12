#include "ChunkReader.h"

ChunkReader::~ChunkReader()
{
    if(file)
    {
        fclose(file);
    }
}

bool ChunkReader::open(const char * filename)
{
    file = fopen(filename, "rb");
    return file != nullptr;
}

std::shared_ptr<Chunk> ChunkReader::readNext()
{
    std::shared_ptr<Chunk> chunk = readChunk();
    if(previous)
    {
        previous->setNext(chunk);
    }
    previous = chunk;
    return chunk;
}

std::shared_ptr<Chunk> ChunkReader::readChunk()
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