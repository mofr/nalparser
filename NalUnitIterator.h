#pragma once

#include "Chunk.h"
#include "NalUnit.h"

class NalUnitIterator
{
public:
    NalUnitIterator(std::shared_ptr<Chunk> chunk);

    /*
     * @return true on success, false if end of file reached
     */
    bool getNext(NalUnit &nalUnit);

private:
    std::shared_ptr<Chunk> chunk;
    const std::vector<Chunk::StartCodePrefix> & startCodePrefixes;
    std::vector<Chunk::StartCodePrefix>::const_iterator i;
    long lastOffset;
};

