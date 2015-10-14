#include "NalUnitIterator.h"

NalUnitIterator::NalUnitIterator(std::shared_ptr<Chunk> chunk) :
    chunk(chunk),
    startCodePrefixes(chunk->getStartCodePrefixes())
{
    i = begin(startCodePrefixes);

    std::shared_ptr<Chunk> prev = chunk;
    std::shared_ptr<Chunk> next = prev->getNext();
    for(; next; next = next->getNext())
    {
        if(!next->getStartCodePrefixes().empty())
        {
            lastOffset = next->offset + next->getStartCodePrefixes().front().offset;
            break;
        }
        prev = next;
    }

    if(!next)
    {
        lastOffset = prev->offset + prev->size;
    }
}

bool NalUnitIterator::getNext(NalUnit &nalUnit)
{
    if(i == end(startCodePrefixes))
    {
        return false;
    }

    long nextOffset;
    if(i + 1 == end(startCodePrefixes))
    {
        nextOffset = lastOffset;
    }
    else
    {
        nextOffset = (i+1)->offset + chunk->offset;
    }

    long offset = i->offset + chunk->offset;
    long size = nextOffset - offset;
    int startCodePrefixLength = i->length;
    unsigned char firstByte;
    if (chunk->size > offset - chunk->offset + startCodePrefixLength)
    {
        firstByte = chunk->data[offset - chunk->offset + startCodePrefixLength];
    }
    else
    {
        std::shared_ptr<Chunk> nextChunk = chunk->getNext();
        if (nextChunk)
        {
            firstByte = nextChunk->data[offset - chunk->size - chunk->offset + startCodePrefixLength];
        }
        else
        {
            return false;
        }
    }

    nalUnit.offset = offset;
    nalUnit.size = size;
    nalUnit.type = (firstByte & 0b01111110) >> 1;
    nalUnit.first = chunk->offset == 0 && i == begin(startCodePrefixes);

    ++i;
    return true;
}
