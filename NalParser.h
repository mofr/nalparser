#pragma once

#include <memory>
#include <thread>
#include <list>

#include "NalUnit.h"
#include "Chunk.h"
#include "BlockingQueue.h"

class NalParser
{
public:
    typedef std::function<void(int index, const NalUnit & nalUnit)> Callback;

    NalParser(int threadCount);
    ~NalParser();

    void setCallback(Callback callback);
    void parse(std::shared_ptr<Chunk> chunk);
    void close();
    int count() const;

private:
    void collect(NalUnit nalUnit);
    void output(NalUnit nalUnit);
    static void parseChunks(NalParser & self);

private:
    BlockingQueue<std::shared_ptr<Chunk>> chunkQueue{100};
    std::vector<std::thread> threads;
    bool closed = false;
    Callback callback;

    long waitingOffset = 0;
    long nalUnitCount = 0;
    long collectedCount = 0;
    std::list<NalUnit> nalUnitList;
    mutable std::mutex mutex;
};

