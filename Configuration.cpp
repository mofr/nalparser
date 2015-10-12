#include "Configuration.h"
#include "XmlDocument.h"
#include <iostream>
#include <cstdlib>
#include <ctime>

Configuration::Configuration(const char * filename)
{
    std::srand(std::time(0));

    XmlDocument configFile;
    if(!configFile.load(filename))
    {
        std::cerr << "Can't load config file '" << filename << "': " << configFile.getLastError() << std::endl;
        std::exit(1);
    }

    sleepRanges.resize(64);

    for(auto & task : configFile.getRoot().children)
    {
        if(task.name != "task")
        {
            continue;
        }

        auto typeAttribute = task.attributes.find("nal_unit_type");
        if(typeAttribute == task.attributes.end())
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

        auto minAttribute = task.attributes.find("min_sleep");
        if(minAttribute == task.attributes.end())
        {
            std::cerr << "Missing min_sleep attribute" << std::endl;
            std::exit(1);
        }
        int min = std::stoi(minAttribute->second);

        auto maxAttribute = task.attributes.find("max_sleep");
        if(maxAttribute == task.attributes.end())
        {
            std::cerr << "Missing max_sleep attribute" << std::endl;
            std::exit(1);
        }
        int max = std::stoi(maxAttribute->second);

        sleepRanges[type] = {min, max};
    }
}

Configuration::SleepRange * Configuration::getSleepRange(int nalUnitType)
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

int Configuration::getRandomSleepTime(int nalUnitType)
{
    SleepRange * sleepRange = getSleepRange(nalUnitType);
    if(!sleepRange)
    {
        return 0;
    }
    return std::rand() % (sleepRange->max - sleepRange->min) + sleepRange->min;
}