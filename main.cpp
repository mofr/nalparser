#include <iostream>
#include "NalReader.h"
#include "ProcessingQueue.h"

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
//    if(argc < 2)
//    {
//        std::cout << "Usage: executable <filename>" << std::endl;
//        return 1;
//    }
//
//    char * filename = argv[1];
//
//    FILE * input = fopen(filename, "rb");
//    if(!input)
//    {
//        std::cout << "Can't open file '" << filename << "'" << std::endl;
//        return 2;
//    }
//
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

struct NalUnit
{
    int type;
    int elapsedMillis;
};


/**
 * @todo Parse xml config
 */
int main(int argc, char ** argv)
{
    if(argc < 2)
    {
        std::cerr << "Usage: executable FILENAME [THREADS]" << std::endl;
        return 1;
    }

    const char * filename = argv[1];

    int threadCount;
    if(argc > 2)
    {
        threadCount = strtol(argv[2], nullptr, 10);
        if(threadCount <= 0)
        {
            std::cerr << "Invalid thread count: " << argv[2] << std::endl;
            return 2;
        }
    }
    else
    {
        threadCount = std::thread::hardware_concurrency();
    }

    std::cout << "Thread count: " << threadCount << std::endl;

    NalReader reader(filename);

    auto inputFunction = [&reader](BinaryNalUnit & binaryNalUnit) {
        return reader.readNext(binaryNalUnit);
    };

    auto processFunction = [](const BinaryNalUnit & binaryNalUnit, NalUnit & nalUnit) {
        nalUnit.type = binaryNalUnit.fakeData * 2;
        nalUnit.elapsedMillis = 400 + (binaryNalUnit.fakeData % 2) * 400;
        std::this_thread::sleep_for(std::chrono::milliseconds(nalUnit.elapsedMillis));
    };

    auto outputFunction = [](const NalUnit & nalUnit) {
        std::cout << nalUnit.type << " " << nalUnit.elapsedMillis << " ms" << std::endl;
    };

    ProcessingQueue<BinaryNalUnit, NalUnit> processingQueue;
    processingQueue.start(threadCount, inputFunction, processFunction, outputFunction);
    processingQueue.waitForFinished();

    return 0;
}
