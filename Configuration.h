#pragma once

#include <vector>

class Configuration
{
public:
    struct SleepRange
    {
        int min = 0;
        int max = 0;

        SleepRange(int min = 0, int max = 0) : min(min), max(max)
        { }
    };

    Configuration(const char * filename);

    SleepRange * getSleepRange(int nalUnitType);
    int getRandomSleepTime(int nalUnitType);

private:
    std::vector<SleepRange> sleepRanges;
};
