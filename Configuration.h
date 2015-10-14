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

    const SleepRange * getSleepRange(int nalUnitType) const;

    /*
     * @return random sleep time in milliseconds, 0 if not configured
     */
    int getRandomSleepTime(int nalUnitType) const;

    /*
     * @return chunk size in bytes
     */
    int getChunkSize() const;

    int getQueueLength() const;

private:
    std::vector<SleepRange> sleepRanges;
    int chunkSize;
    int queueLength;
};
