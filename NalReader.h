#pragma once

struct BinaryNalUnit
{
    int fakeData;
};

/**
 * @todo Implement file reading
 */
class NalReader
{
public:
    NalReader(const char * filename) { }

    bool readNext(BinaryNalUnit & unit)
    {
        unit.fakeData = i;
        ++i;
        return i <= 10;
    }

private:
    int i = 0;
};