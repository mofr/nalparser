#include "Configuration.h"
#include "XmlDocument.h"
#include <iostream>
#include <string>
#include <cstdlib>
#include <ctime>

Configuration::Configuration(const char * filename) :
    chunkSize(128 * 1000),
    queueLength(100)
{
    XmlDocument configFile;
    if(!configFile.load(filename))
    {
        std::cerr << "Can't load config file '" << filename << "': " << configFile.getLastError() << std::endl;
        std::cout << "Using default config: zero sleeps, 128 KB chunk, 100 queue length" << std::endl;
        return;
    }

    std::srand(std::time(0));
    sleepRanges.resize(64, {0,0});

    for(auto &item : configFile.getRoot().children)
    {
        if(item.name == "queue_length")
        {
            auto valueAttribute = item.attributes.find("value");
            if(valueAttribute == item.attributes.end())
            {
                std::cerr << "Missing queue_length value attribute" << std::endl;
                std::exit(1);
            }
            queueLength = std::stoi(valueAttribute->second);
        }
        else if(item.name == "chunk_size")
        {
            auto valueAttribute = item.attributes.find("value");
            if(valueAttribute == item.attributes.end())
            {
                std::cerr << "Missing chunk_size value attribute" << std::endl;
                std::exit(1);
            }
            chunkSize = std::stoi(valueAttribute->second) * 1024; // kilobytes
        }
        else if(item.name == "task")
        {
            auto typeAttribute = item.attributes.find("nal_unit_type");
            if(typeAttribute == item.attributes.end())
            {
                std::cerr << "Missing nal_unit_type attribute" << std::endl;
                std::exit(1);
            }
            int type = std::stoi(typeAttribute->second);
            if(type >= sleepRanges.size() || type < 0)
            {
                std::cerr << "Invalid nal_unit_type = " << type << std::endl;
                std::exit(1);
            }

            auto minAttribute = item.attributes.find("min_sleep");
            if(minAttribute == item.attributes.end())
            {
                std::cerr << "Missing min_sleep attribute" << std::endl;
                std::exit(1);
            }
            int min = std::stoi(minAttribute->second);

            auto maxAttribute = item.attributes.find("max_sleep");
            if(maxAttribute == item.attributes.end())
            {
                std::cerr << "Missing max_sleep attribute" << std::endl;
                std::exit(1);
            }
            int max = std::stoi(maxAttribute->second);

            sleepRanges[type] = {min, max};
        }
    }
}

const Configuration::SleepRange * Configuration::getSleepRange(int nalUnitType) const
{
    if(nalUnitType < sleepRanges.size())
    {
        return sleepRanges.data() + nalUnitType;
    }
    else
    {
        return nullptr;
    }
}

int Configuration::getRandomSleepTime(int nalUnitType) const
{
    const SleepRange * sleepRange = getSleepRange(nalUnitType);
    if(!sleepRange)
    {
        return 0;
    }
    return std::rand() % (sleepRange->max - sleepRange->min) + sleepRange->min;
}

int Configuration::getChunkSize() const
{
    return chunkSize;
}

int Configuration::getQueueLength() const
{
    return queueLength;
}
