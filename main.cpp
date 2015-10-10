#include <iostream>
#include "Arguments.h"
#include "BlockingQueue.h"
#include "ChunkReader.h"
#include "NalParser.h"


void parseChunks(BlockingQueue<std::shared_ptr<Chunk>> &chunkQueue)
{
    long nalUnitCount = 0;
    std::shared_ptr<Chunk> chunk;
    while(chunkQueue.pop(chunk))
    {
        // @todo parse nal units
        for(auto & startCodePrefixPos : chunk->getStartCodePrefixes())
        {
            std::cout << startCodePrefixPos << std::endl;
            ++nalUnitCount;
        }
        // @todo output to collector
    }

    std::cout << "Nal unit count: " << nalUnitCount << std::endl;
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

    BlockingQueue<std::shared_ptr<Chunk>> chunkQueue(args.threadCount * 2);

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

    return 0;
}
