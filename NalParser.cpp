#include "NalParser.h"
#include "NalUnitIterator.h"
#include <iostream>
#include <chrono>

NalParser::NalParser(int threadCount,
                     int maxQueueLength,
                     ProcessFunction processFunction,
                     OutputFunction outputFunction) :
    chunkQueue(maxQueueLength),
    processFunction(processFunction),
    outputFunction(outputFunction)
{
    for(int i = 0; i < threadCount; ++i)
    {
        threads.emplace_back([this](){
            NalUnit nalUnit;
            std::shared_ptr<Chunk> chunk;
            while(chunkQueue.pop(chunk))
            {
                NalUnitIterator nalUnitIterator(chunk);
                while(nalUnitIterator.getNext(nalUnit))
                {
                    process(nalUnit);
                }
            }
        });
    }
}

NalParser::~NalParser()
{
    if(!closed)
    {
        close();
    }

    if(nalUnitList.size() != 0)
    {
        std::cerr << "Error: nalUnitList is not empty: " << nalUnitList.size() << std::endl;
    }
}

void NalParser::parse(std::shared_ptr<Chunk> chunk)
{
    chunkQueue.push(chunk);
}

void NalParser::close()
{
    chunkQueue.close();

    for(auto & thread : threads)
    {
        thread.join();
    }

    closed = true;
}

int NalParser::count() const
{
    std::unique_lock<std::mutex> lock(mutex);
    return nalUnitCount;
}

void NalParser::process(NalUnit & nalUnit)
{
    auto start = std::chrono::system_clock::now();
    processFunction(nalUnit);
    nalUnit.elapsedMillis = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start).count();
    collect(nalUnit);
}

void NalParser::collect(const NalUnit & nalUnit)
{
    std::unique_lock<std::mutex> lock(mutex);

    if(waitingOffset == nalUnit.offset || nalUnit.first)
    {
        output(nalUnit);
        auto iter = begin(nalUnitList);
        for(; iter != end(nalUnitList) && waitingOffset == iter->offset; ++iter)
        {
            output(*iter);
        }
        if(iter != begin(nalUnitList))
        {
            nalUnitList.erase(begin(nalUnitList), iter);
        }
    }
    else
    {
        //insert into nalUnitList, keeping sorted by nalUnit.offset
        bool inserted = false;
        for(auto iter = begin(nalUnitList); iter != end(nalUnitList); ++iter)
        {
            if(iter->offset > nalUnit.offset)
            {
                nalUnitList.insert(iter, nalUnit);
                inserted = true;
                break;
            }
        }
        if(!inserted)
        {
            nalUnitList.push_back(nalUnit);
        }
    }
}

void NalParser::output(const NalUnit & nalUnit)
{
    outputFunction(nalUnitCount, nalUnit);
    waitingOffset = nalUnit.offset + nalUnit.size;
    ++nalUnitCount;
}
