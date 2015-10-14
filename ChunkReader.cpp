#include "ChunkReader.h"

ChunkReader::ChunkReader(int chunkSize) :
    chunkSize(chunkSize)
{
}

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

    std::shared_ptr<Chunk> chunk(new Chunk(chunkSize));
    chunk->size = fread(chunk->data, 1, chunkSize, file);
    chunk->offset = offset;
    offset += chunk->size;
    return chunk;
}
