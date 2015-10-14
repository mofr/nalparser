#pragma once

#include <mutex>
#include <condition_variable>
#include <vector>
#include <memory>

/*
 * @brief Chunk is a binary chunk of file.
 * @param offset offset in bytes from file beginning
 * @param size size in bytes
 */
struct Chunk
{
    long offset = 0;
    long size = 0;
    unsigned char * const data = nullptr;

    Chunk(long size);
    ~Chunk();

    /*
     * @param offset offset within chunk
     */
    struct StartCodePrefix
    {
        long offset = 0;
        int length = 0; // 3 or 4

        StartCodePrefix() {}
    };

    /*
     * @return Next chunk in file, blocks and wait if next is not available (setNext). Returns null poitner if end of file reached.
     */
    std::shared_ptr<Chunk> getNext() const;

    void setNext(std::shared_ptr<Chunk> next);

    /*
     * @return start code prefixes starts in chunk
     */
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