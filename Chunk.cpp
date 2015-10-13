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

const std::vector<Chunk::StartCodePrefix> &Chunk::getStartCodePrefixes()
{
    std::unique_lock<std::mutex> lock(mutex);
    if(!parsed)
    {
        findStartCodePrefixes();
        parsed = true;
    }

    return startCodePrefixes;
}

void Chunk::findStartCodePrefixes()
{
    unsigned char * begin = data;
    unsigned char * i = data;
    unsigned char * end = data + size;
    int matchCount = 0;

    StartCodePrefix scp;

    while(true)
    {
        if(i == end)
        {
            if(matchCount == 0)
            {
                return;
            }
            std::shared_ptr<Chunk> next = getNext();
            if(!next)
            {
                //end of file reached
                return;
            }
            begin = next->data;
            i = next->data;
            end = next->data + next->size;
        }

        if((matchCount == 0 && *i == 0x00) ||
           (matchCount == 1 && *i == 0x00) ||
           (matchCount == 2 && *i == 0x00) ||
           (matchCount == 2 && *i == 0x01) ||
           (matchCount == 3 && *i == 0x01))
        {
            if(matchCount == 0)
            {
                scp.offset = i - begin;
            }
            ++matchCount;

            if(*i == 0x01)
            {
                scp.length = matchCount;
                startCodePrefixes.push_back(scp);
                if(data != begin)
                {
                    //already in next chunk
                    return;
                }
                matchCount = 0;
            }
        }
        else
        {
            matchCount = 0;
        }

        ++i;
    }
}