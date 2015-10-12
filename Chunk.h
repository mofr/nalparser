#pragma once

#include "NalParser.h"

struct Chunk
{
    long offset = 0;
    long size = 0;
    unsigned char * data = nullptr;

    Chunk(long size) : size(size)
    {
        data = new unsigned char[size];
    }

    ~Chunk()
    {
        delete[] data;
    }

    void setNext(std::shared_ptr<Chunk> next)
    {
        std::unique_lock<std::mutex> lock(nextMutex);
        this->next = next;
        this->nextInitialized = true;
        nextCondition.notify_all();
    }

    std::shared_ptr<Chunk> getNext() const
    {
        std::unique_lock<std::mutex> lock(nextMutex);
        nextCondition.wait(lock, [this](){return nextInitialized;});
        return next;
    }

    const std::vector<long> & getStartCodePrefixes()
    {
        std::unique_lock<std::mutex> lock(mutex);
        if(!parsed)
        {
            findStartCodePrefixes();
            parsed = true;
        }

        return startCodePrefixes;
    }

private:
    // @todo find splitted prefixes
    void findStartCodePrefixes()
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

private:
    std::mutex mutex;
    bool parsed = false;
    std::vector<long> startCodePrefixes;

    mutable std::mutex nextMutex;
    mutable std::condition_variable nextCondition;
    bool nextInitialized = false;
    std::shared_ptr<Chunk> next;
};