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
    /*
     * NAL unit processing function callback. Called in threads.
     */
    typedef std::function<void(const NalUnit & nalUnit)> ProcessFunction;

    /*
     * Output callback signature. Called in threads.
     */
    typedef std::function<void(int index, const NalUnit & nalUnit)> OutputFunction;

    NalParser(int threadCount,
              int maxQueueLength,
              ProcessFunction processFunction,
              OutputFunction outputFunction);
    ~NalParser();

    /*
     * Push chunk in queue. Will block and wait if queue length greater then maxQueueLength.
     * For each NAL unit found ProcessFunction will be called, and then OutputFunction.
     */
    void parse(std::shared_ptr<Chunk> chunk);

    /*
     * Wait until no input chunks remained, then stop working threads.
     */
    void close();

    /*
     * @return NAL unit count that was parsed.
     */
    int count() const;

private:
    void process(NalUnit & nalUnit);
    void collect(const NalUnit & nalUnit);
    void output(const NalUnit & nalUnit);

private:
    BlockingQueue<std::shared_ptr<Chunk>> chunkQueue;
    std::vector<std::thread> threads;
    bool closed = false;
    ProcessFunction processFunction;
    OutputFunction outputFunction;

    long waitingOffset = 0;
    long nalUnitCount = 0;
    std::list<NalUnit> nalUnitList;
    mutable std::mutex mutex;
};

