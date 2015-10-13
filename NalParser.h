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
    typedef std::function<int(const NalUnit & nalUnit)> ProcessFunction;
    typedef std::function<void(int index, const NalUnit & nalUnit)> OutputFunction;

    NalParser(int threadCount, ProcessFunction processFunction, OutputFunction outputFunction);
    ~NalParser();

    void parse(std::shared_ptr<Chunk> chunk);
    void close();
    int count() const;

private:
    void collect(const NalUnit & nalUnit);
    void output(const NalUnit & nalUnit);

private:
    BlockingQueue<std::shared_ptr<Chunk>> chunkQueue{100};
    std::vector<std::thread> threads;
    bool closed = false;
    ProcessFunction processFunction;
    OutputFunction outputFunction;

    long waitingOffset = 0;
    long nalUnitCount = 0;
    std::list<NalUnit> nalUnitList;
    mutable std::mutex mutex;
};

