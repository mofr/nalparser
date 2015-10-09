#include <iostream>
#include "Arguments.h"
#include "BlockingQueue.h"

//const char* nal_types[] = {"TRAIL_N",
//"TRAIL_R",
//"TSA_N",
//"TSA_R",
//"STSA_N",
//"STSA_R",
//"RADL_N",
//"RADL_R",
//"RASL_N",
//"RASL_R",
//"RSV_VCL_N10",
//"RSV_VCL_N12",
//"RSV_VCL_N14",
//"RSV_VCL_R11",
//"RSV_VCL_R13",
//"RSV_VCL_R15",
//"BLA_W_LP",
//"BLA_W_RADL",
//"BLA_N_LP",
//"IDR_W_RADL",
//"IDR_N_LP",
//"CRA_NUT",
//"RSV_IRAP_VCL22",
//"RSV_IRAP_VCL23",
//"RSV_VCL24..",
//"RSV_VCL31",
//"VPS_NUT",
//"SPS_NUT",
//"PPS_NUT",
//"AUD_NUT",
//"EOS_NUT",
//"EOB_NUT",
//"FD_NUT"
//"PREFIX_SEI_NUT"
//"SUFFIX_SEI_NUT"
//"RSV_NVCL41"
//"RSV_NVCL47"
//};
//
//struct nal_unit_header
//{
//    int forbidden_zero_bit : 1;
//    unsigned int nal_unit_type : 6;
//    unsigned int nuh_layer_id : 6;
//    unsigned int nuh_temporal_id_plus1 : 3;
//};
//
//int parse_nal(int argc, char ** argv)
//{
//    int nalCount = 0;
//    static const int BUFFER_SIZE = 1024*8;
//    char buf[BUFFER_SIZE];
//    int read;
//    int filesize = 0;
//    int continuous = 0;
//    while(!feof(input))
//    {
//        read = fread(buf, 1, BUFFER_SIZE, input);
//        if(ferror(input))
//        {
//            std::cout << "File read error" << std::endl;
//            break;
//        }
//        filesize += read;
//        for(int i = 0; i < read; ++i)
//        {
//            if((continuous == 0 && buf[i] == 0x00) ||
//               (continuous == 1 && buf[i] == 0x00) ||
//               (continuous == 2 && buf[i] == 0x00) ||
//               (continuous == 3 && buf[i] == 0x01))
//            {
//                continuous += 1;
//            }
//            else
//            {
//                if(continuous == 4)
//                {
//                    nalCount += 1;
//
//                    if(i + 2 >= read)
//                    {
//                        std::cout << "buffer over" << std::endl;
//                    }
//                    else
//                    {
//                        nal_unit_header * h = (nal_unit_header*)(buf + i);
//                        std::cout << h->nal_unit_type << std::endl;
//                    }
//                }
//                continuous = 0;
//            }
//        }
//    }
//
//    std::cout << "File size = " << filesize << std::endl;
//    std::cout << "NAL unit count = " << nalCount << std::endl;
//
//    fclose(input);
//    return 0;
//}


//void variant1()
//{
//    NalReader reader(filename);
//
//    struct NalUnit
//    {
//        int type;
//        int elapsedMillis;
//    };
//
//    auto inputFunction = [&reader](BinaryNalUnit & binaryNalUnit) {
//        return reader.readNext(binaryNalUnit);
//    };
//
//    auto processFunction = [](const BinaryNalUnit & binaryNalUnit, NalUnit & nalUnit) {
//        nalUnit.type = binaryNalUnit.fakeData * 2;
//        nalUnit.elapsedMillis = 400 + (binaryNalUnit.fakeData % 2) * 400;
//        std::this_thread::sleep_for(std::chrono::milliseconds(nalUnit.elapsedMillis));
//    };
//
//    auto outputFunction = [](const NalUnit & nalUnit) {
//        std::cout << nalUnit.type << " " << nalUnit.elapsedMillis << " ms" << std::endl;
//    };
//
//    ProcessingQueue<BinaryNalUnit, NalUnit> processingQueue;
//    processingQueue.start(threadCount, inputFunction, processFunction, outputFunction);
//    processingQueue.waitForFinished();
//}

struct Chunk
{
    long offset = 0;
    long size = 0;
    char * data = nullptr;
    Chunk * next = nullptr;


    Chunk(long size) : size(size)
    {
        data = new char[size];
    }

    ~Chunk()
    {
        delete[] data;
    }

    const std::vector<long> & getStartCodePrefixes()
    {
        std::unique_lock<std::mutex> lock(mutex);
        if(!parsed)
        {
            // @todo find startCodePrefixPositions in data
        }

        return startCodePrefixPositions;
    }

private:
    std::mutex mutex;// @todo read-write lock
    bool parsed = false;
    std::vector<long> startCodePrefixPositions;
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

    Chunk * readNext()
    {
        if(feof(file))
        {
            if(previous)
            {
                Chunk * chunk = previous;
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

        Chunk * chunk = previous;
        chunk->next = readChunk();
        previous = chunk->next;
        return chunk;
    }

private:
    Chunk * readChunk()
    {
        // @todo chunk pool
        Chunk * chunk = new Chunk(ChunkSize);
        chunk->size = fread(chunk->data, 1, ChunkSize, file);
        chunk->offset = offset;
        offset += chunk->size;
        return chunk;
    }

private:
    FILE * file = nullptr;
    long offset = 0;
    Chunk * previous = nullptr;

    static const int ChunkSize = 1024*1024;
};

void chunkParser(BlockingQueue<Chunk*> & chunkQueue)
{
    Chunk * chunk;
    while(chunkQueue.pop(chunk))
    {
        std::cout << chunk->offset << " " << chunk->next << std::endl;
        // @todo parse chunk + chunk->next
        // @todo output to collector

        delete chunk;
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

    BlockingQueue<Chunk*> chunkQueue(args.threadCount * 2);

    std::thread thread(chunkParser, std::ref(chunkQueue));

    while(Chunk * chunk = reader.readNext())
    {
        chunkQueue.push(chunk);
    }
    chunkQueue.close();

    thread.join();

    return 0;
}
