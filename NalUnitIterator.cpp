#include "NalUnitIterator.h"
#include <iostream>

NalUnitIterator::NalUnitIterator(std::shared_ptr<Chunk> chunk) :
    chunk(chunk)
{
    startCodePrefixes = chunk->getStartCodePrefixes();

    if(startCodePrefixes.empty())
    {
        return;
    }

    for(auto & startCodePrefix : startCodePrefixes)
    {
        startCodePrefix.offset += chunk->offset;
    }

    for(std::shared_ptr<Chunk> next = chunk->getNext(); next != nullptr; next = next->getNext())
    {
        const std::vector<Chunk::StartCodePrefix> & nextStartCodePrefixes = next->getStartCodePrefixes();
        if(!nextStartCodePrefixes.empty())
        {
            Chunk::StartCodePrefix startCodePrefix = nextStartCodePrefixes.front();
            startCodePrefix.offset += next->offset;
            startCodePrefixes.push_back(startCodePrefix);
            break;
        }
    }
}

bool NalUnitIterator::next(NalUnit &nalUnit)
{
    if(i + 1>= startCodePrefixes.size())
    {
        return false;
    }

    int startCodePrefixLength = startCodePrefixes[i].length;
    long offset = startCodePrefixes[i].offset;
    long size = startCodePrefixes[i + 1].offset - offset;
    unsigned char firstByte;
    if (chunk->size > offset - chunk->offset + startCodePrefixLength)
    {
        firstByte = chunk->data[offset - chunk->offset + startCodePrefixLength];
    }
    else
    {
        std::shared_ptr<Chunk> next = chunk->getNext();
        if (next)
        {
            firstByte = chunk->getNext()->data[offset - chunk->size - chunk->offset + startCodePrefixLength];
        }
        else
        {
            return false;
        }
    }

    nalUnit.offset = offset;
    nalUnit.size = size;
    nalUnit.type = (firstByte & 0b01111110) >> 1;
    nalUnit.first = chunk->offset == 0 && i == 0;

    ++i;
    return true;
}
