#include <iostream>
#include <list>
#include "Arguments.h"
#include "BlockingQueue.h"
#include "ChunkReader.h"
#include "NalParser.h"


long waitingOffset = 0;
long nalUnitCount = 0;
long collectedCount = 0;
std::list<NalUnit> nalUnitList;
std::mutex collectMutex;

void collect(NalUnit nalUnit)
{
    std::unique_lock<std::mutex> lock(collectMutex);

    collectedCount++;
    auto output = [](const NalUnit & nalUnit){
        std::cout << nalUnitCount << ": " << nalTypeAsString(nalUnit.type);
        std::cout << " offset=" << nalUnit.offset;
        std::cout << " size=" << nalUnit.size;
        std::cout << std::endl;

        waitingOffset = nalUnit.offset + nalUnit.size + 4;
        ++nalUnitCount;
    };

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

void parseChunks(BlockingQueue<std::shared_ptr<Chunk>> &chunkQueue)
{
    std::shared_ptr<Chunk> chunk;
    while(chunkQueue.pop(chunk))
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
            long size = startCodePrefixes[i+1] - offset - 4;
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
            collect({offset, size, type, first});
        }
    }
}

/**
 * @todo parse xml config
 */
int main(int argc, char ** argv)
{
    Arguments args(argc, argv);
    std::cout << "Thread count: " << args.threadCount << std::endl;

    ChunkReader reader;
    if(!reader.open(args.filename))
    {
        std::cerr << "Can't open file " << args.filename << std::endl;
        return 1;
    }

    BlockingQueue<std::shared_ptr<Chunk>> chunkQueue(100);

    std::vector<std::thread> threads;
    for(int i = 0; i < args.threadCount; ++i)
    {
        threads.emplace_back(parseChunks, std::ref(chunkQueue));
    }

    while(std::shared_ptr<Chunk> chunk = reader.readNext())
    {
        chunkQueue.push(chunk);
    }
    chunkQueue.close();

    for(auto & thread : threads)
    {
        thread.join();
    }

    std::cout << "Nal unit count: " << nalUnitCount << std::endl;
    std::cout << "Collected count: " << collectedCount << std::endl;

    return 0;
}
