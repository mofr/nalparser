#include "Chunk.h"

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
    int zeros = 0;

    StartCodePrefix scp;

    while(true)
    {
        if(i == end)
        {
            if(zeros == 0 || data != begin)
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
            end = next->data + std::min(next->size, 2L); // 2 bytes is enough to capture remaining start code prefix part
        }

        if(*i == 0x01 && zeros >= 2)
        {
            if(zeros > 2)
                scp.length = 4;
            else
                scp.length = 3;
            scp.offset = i - begin - scp.length + 1;
            startCodePrefixes.push_back(scp);
            zeros = 0;
        }
        else if(*i == 0x00)
        {
            ++zeros;
        }
        else
        {
            zeros = 0;
        }

        ++i;
    }
}