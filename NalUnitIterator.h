#pragma once

#include "Chunk.h"
#include "NalUnit.h"

class NalUnitIterator
{
public:
    NalUnitIterator(std::shared_ptr<Chunk> chunk);

    bool next(NalUnit & nalUnit);

private:
    std::shared_ptr<Chunk> chunk;
    std::vector<long> startCodePrefixes;
    int i = 0;
};

