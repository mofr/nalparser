#pragma once

#include <mutex>
#include <condition_variable>
#include <vector>
#include <memory>

struct Chunk
{
    long offset = 0;
    long size = 0;
    unsigned char * const data = nullptr;

    Chunk(long size);
    ~Chunk();

    struct StartCodePrefix
    {
        long offset = 0;
        int length = 0; // 3 or 4

        StartCodePrefix() {}
    };

    void setNext(std::shared_ptr<Chunk> next);
    std::shared_ptr<Chunk> getNext() const;

    const std::vector<StartCodePrefix> & getStartCodePrefixes();

private:
    void findStartCodePrefixes();

private:
    std::mutex mutex;
    bool parsed = false;
    std::vector<StartCodePrefix> startCodePrefixes;

    mutable std::mutex nextMutex;
    mutable std::condition_variable nextCondition;
    bool nextInitialized = false;
    std::shared_ptr<Chunk> next;
};