#include "Arguments.h"
#include "Configuration.h"
#include "ChunkReader.h"
#include "NalParser.h"
#include <iostream>
#include <chrono>

int main(int argc, char ** argv)
{
    Arguments args(argc, argv);
    Configuration configuration(args.configFilename);

    ChunkReader reader;
    if(!reader.open(args.filename))
    {
        std::cerr << "Can't open file " << args.filename << std::endl;
        return 1;
    }

    std::cout << "Thread count: " << args.threadCount << std::endl;

    auto processFunction = [&configuration](const NalUnit & nalUnit){
        int millis = configuration.getRandomSleepTime(nalUnit.type);
        std::this_thread::sleep_for(std::chrono::milliseconds(millis));
        return millis;
    };

    auto outputFunction = [](int index, const NalUnit & nalUnit){
        std::cout << index << ": " << nalTypeAsString(nalUnit.type);
        std::cout << " offset=" << nalUnit.offset;
        std::cout << " size=" << nalUnit.size;
        std::cout << " " << nalUnit.elapsedMillis << " ms";
        std::cout << std::endl;
    };

    auto start = std::chrono::system_clock::now();

    NalParser nalParser(args.threadCount, processFunction, outputFunction);
    while(auto chunk = reader.readNext())
    {
        nalParser.parse(chunk);
    }
    nalParser.close();

    std::cout << "NAL unit count: " << nalParser.count() << std::endl;

    auto elapsedMillis = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start).count();
    std::cout << "Elapsed: " << elapsedMillis << " ms" << std::endl;

    return 0;
}
