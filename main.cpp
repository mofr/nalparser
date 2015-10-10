#include <iostream>
#include "Arguments.h"
#include "BlockingQueue.h"

const char* nalTypes[] = {
        "TRAIL_N",
        "TRAIL_R",
        "TSA_N",
        "TSA_R",
        "STSA_N",
        "STSA_R",
        "RADL_N",
        "RADL_R",
        "RASL_N",
        "RASL_R",
        "RSV_VCL_N10",
        "RSV_VCL_N12",
        "RSV_VCL_N14",
        "RSV_VCL_R11",
        "RSV_VCL_R13",
        "RSV_VCL_R15",
        "BLA_W_LP",
        "BLA_W_RADL",
        "BLA_N_LP",
        "IDR_W_RADL",
        "IDR_N_LP",
        "CRA_NUT",
        "RSV_IRAP_VCL22",
        "RSV_IRAP_VCL23",
        "RSV_VCL24",
        "RSV_VCL25",
        "RSV_VCL26",
        "RSV_VCL28",
        "RSV_VCL29",
        "RSV_VCL30",
        "RSV_VCL31",
        "VPS_NUT",
        "SPS_NUT",
        "PPS_NUT",
        "AUD_NUT",
        "EOS_NUT",
        "EOB_NUT",
        "FD_NUT",
        "PREFIX_SEI_NUT",
        "SUFFIX_SEI_NUT",
        "RSV_NVCL41",
        "RSV_NVCL42",
        "RSV_NVCL43",
        "RSV_NVCL44",
        "RSV_NVCL45",
        "RSV_NVCL46",
        "RSV_NVCL47"
};

const char * nalTypeAsString(int type)
{
    if(type < 0 || type > 47)
    {
        return "UNKNOWN";
    }
    else
    {
        return nalTypes[type];
    }
}

struct NalUnit
{
    long offset;
    int type;

    NalUnit(long offset, int type) : offset(offset), type(type)
    { }
};

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
    // @todo parse chunk + chunk->next
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

class ChunkReader
{
public:
    ChunkReader()
    { }

    ~ChunkReader()
    {
        if(file)
        {
            fclose(file);
        }
    }

    bool open(const char * filename)
    {
        file = fopen(filename, "rb");
        return file != nullptr;
    }

    std::shared_ptr<Chunk> readNext()
    {
        if(feof(file))
        {
            if(previous)
            {
                std::shared_ptr<Chunk> chunk = previous;
                previous = nullptr;
                return chunk;
            }
            else
            {
                return nullptr;
            }
        }

        if(!previous)
        {
            previous = readChunk();
        }

        std::shared_ptr<Chunk> chunk = previous;
        chunk->setNext(readChunk());
        previous = chunk->getNext();
        return chunk;
    }

private:
    std::shared_ptr<Chunk> readChunk()
    {
        // @todo chunk pool (threadsafe)
        std::shared_ptr<Chunk> chunk(new Chunk(ChunkSize));
        chunk->size = fread(chunk->data, 1, ChunkSize, file);
        chunk->offset = offset;
        offset += chunk->size;
        return chunk;
    }

private:
    FILE * file = nullptr;
    long offset = 0;
    std::shared_ptr<Chunk> previous;

    static const int ChunkSize = 1024*1024;
};

class Collector
{
public:
    void collect(NalUnit nalUnit)
    {
        std::unique_lock<std::mutex> lock(mutex);
    }

private:
    std::mutex mutex;
};

void chunkParser(BlockingQueue<std::shared_ptr<Chunk>> & chunkQueue)
{
    long nalUnitCount = 0;
    std::shared_ptr<Chunk> chunk;
    while(chunkQueue.pop(chunk))
    {
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
        threads.emplace_back(chunkParser, std::ref(chunkQueue));
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
