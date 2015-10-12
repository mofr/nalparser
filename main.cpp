#include "Arguments.h"
#include "XmlDocument.h"
#include "ChunkReader.h"
#include "NalParser.h"
#include <iostream>

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

    NalParser nalParser(args.threadCount);
    nalParser.setCallback([&](int index, const NalUnit & nalUnit){
        std::cout << index << ": " << nalTypeAsString(nalUnit.type);
        std::cout << " offset=" << nalUnit.offset;
        std::cout << " size=" << nalUnit.size;
        std::cout << " " << nalUnit.elapsedMillis << " ms";
        std::cout << std::endl;
    });

    XmlDocument configFile;
    if(!configFile.load(args.configFilename))
    {
        std::cerr << "Can't load config file '" << args.configFilename << "': " << configFile.getLastError() << std::endl;
        return 1;
    }
    // @todo config from xmlDocument
    for(auto & task : configFile.getRoot().children)
    {
        task.print(std::cout);
    }

    while(std::shared_ptr<Chunk> chunk = reader.readNext())
    {
        nalParser.parse(chunk);
    }
    nalParser.close();

    std::cout << "Nal unit count: " << nalParser.count() << std::endl;

    return 0;
}
