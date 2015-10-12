#include "Chunk.h"
#include "NalUnit.h"

Chunk::Chunk(long size) :
        size(size),
        data(new unsigned char[size])
{
}

Chunk::~Chunk()
{
    delete[] data;
}

void Chunk::setNext(std::shared_ptr<Chunk> next)
{
    std::unique_lock<std::mutex> lock(nextMutex);
    this->next = next;
    this->nextInitialized = true;
    nextCondition.notify_all();
}

std::shared_ptr<Chunk> Chunk::getNext() const
{
    std::unique_lock<std::mutex> lock(nextMutex);
    nextCondition.wait(lock, [this](){return nextInitialized;});
    return next;
}

const std::vector<long> &Chunk::getStartCodePrefixes()
{
    std::unique_lock<std::mutex> lock(mutex);
    if(!parsed)
    {
        findStartCodePrefixes();
        parsed = true;
    }

    return startCodePrefixes;
}

// @todo find splitted prefixes
void Chunk::findStartCodePrefixes()
{
    int matchCount = 0;
    for(long i = 0; i < size; ++i)
    {
        if(matchCount < StartCodePrefixLength)
        {
            if(data[i] == StartCodePrefix[matchCount])
            {
                ++matchCount;
            }
            else
            {
                matchCount = 0;
            }
        }
        else
        {
            startCodePrefixes.push_back(i);
            matchCount = 0;
        }
    }
}