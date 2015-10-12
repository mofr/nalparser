#include "NalUnitIterator.h"

NalUnitIterator::NalUnitIterator(std::shared_ptr<Chunk> chunk) :
    chunk(chunk)
{
    startCodePrefixes = chunk->getStartCodePrefixes();
    if(startCodePrefixes.empty())
    {
        return;
    }

    for(auto & offset : startCodePrefixes)
    {
        offset += chunk->offset;
    }

    for(std::shared_ptr<Chunk> next = chunk->getNext(); next != nullptr; next = next->getNext())
    {
        const std::vector<long> & nextStartCodePrefixes = next->getStartCodePrefixes();
        if(!nextStartCodePrefixes.empty())
        {
            startCodePrefixes.push_back(nextStartCodePrefixes.front() + next->offset);
            break;
        }
    }
}

bool NalUnitIterator::next(NalUnit &nalUnit)
{
    if(i + 1 >= startCodePrefixes.size())
    {
        return false;
    }

    long offset = startCodePrefixes[i];
    long size = startCodePrefixes[i + 1] - offset - StartCodePrefixLength;
    unsigned char firstByte;
    if (chunk->size > offset - chunk->offset)
    {
        firstByte = chunk->data[offset - chunk->offset];
    }
    else
    {
        std::shared_ptr<Chunk> next = chunk->getNext();
        if (next)
        {
            firstByte = chunk->getNext()->data[offset - chunk->size - chunk->offset];
        }
        else
        {
            return false;
        }
    }

    nalUnit.offset = offset;
    nalUnit.size = size;
    nalUnit.type = (firstByte & 0x01111110) >> 1;
    nalUnit.first = chunk->offset == 0 && i == 0;

    ++i;
    return true;
}
