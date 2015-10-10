#pragma once

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
    // @todo also find splitted prefixes
    void findStartCodePrefixes()
    {
        int matchCount = 0;
        for(long i = 0; i < size; ++i)
        {
            if((matchCount == 0 && data[i] == 0x00) ||
               (matchCount == 1 && data[i] == 0x00) ||
               (matchCount == 2 && data[i] == 0x00) ||
               (matchCount == 3 && data[i] == 0x01))
            {
                ++matchCount;
            }
            else
            {
                if(matchCount == 4)
                {
                    startCodePrefixes.push_back(i);
//                    int nalType = (data[i] & 0x01111110) >> 1;
//                    nalUnits.emplace_back(offset + i - 3, nalType);
                }
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