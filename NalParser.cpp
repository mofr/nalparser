#include "NalParser.h"

NalParser::NalParser(int threadCount)
{
    for(int i = 0; i < threadCount; ++i)
    {
        threads.emplace_back(parseChunks, std::ref(*this));
    }
}

NalParser::~NalParser()
{
    if(!closed)
    {
        close();
    }
}

void NalParser::setCallback(Callback callback)
{
    this->callback = callback;
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

void NalParser::collect(NalUnit nalUnit)
{
    std::unique_lock<std::mutex> lock(mutex);

    collectedCount++;

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
        auto iter = begin(nalUnitList);
        for(; iter != end(nalUnitList); ++iter)
        {
            if(iter->offset > nalUnit.offset)
            {
                nalUnitList.insert(iter, nalUnit);
                break;
            }
        }
        if(iter == end(nalUnitList))
        {
            nalUnitList.push_back(nalUnit);
        }
    }
}

void NalParser::output(NalUnit nalUnit)
{
    callback(nalUnitCount, nalUnit);
    waitingOffset = nalUnit.offset + nalUnit.size + StartCodePrefixLength;
    ++nalUnitCount;
}

void NalParser::parseChunks(NalParser & self)
{
    std::shared_ptr<Chunk> chunk;
    while(self.chunkQueue.pop(chunk))
    {
        std::vector<long> startCodePrefixes = chunk->getStartCodePrefixes();
        if(startCodePrefixes.empty())
        {
            continue;
        }

        for(auto & offset : startCodePrefixes)
        {
            offset += chunk->offset;
        }

        std::shared_ptr<Chunk> next = chunk->getNext();
        while(next)
        {
            const std::vector<long> & nextStartCodePrefixes = next->getStartCodePrefixes();
            if(!nextStartCodePrefixes.empty())
            {
                startCodePrefixes.push_back(nextStartCodePrefixes.front() + next->offset);
                break;
            }
            else
            {
                next = next->getNext();
            }
        }

        for(int i = 0; i < startCodePrefixes.size() - 1; ++i)
        {
            long offset = startCodePrefixes[i];
            long size = startCodePrefixes[i+1] - offset - StartCodePrefixLength;
            bool first = chunk->offset == 0 && i == 0;
            unsigned char firstByte;
            if(chunk->size > offset-chunk->offset)
            {
                firstByte = chunk->data[offset-chunk->offset];
            }
            else
            {
                std::shared_ptr<Chunk> next = chunk->getNext();
                if(next)
                {
                    firstByte = chunk->getNext()->data[offset - chunk->size - chunk->offset];
                }
                else
                {
                    break;
                }
            }
            int type = (firstByte & 0x01111110) >> 1;
            int taskDuration = 10;
            std::this_thread::sleep_for(std::chrono::milliseconds(taskDuration));
            self.collect({offset, size, type, first, taskDuration});
        }
    }
}
