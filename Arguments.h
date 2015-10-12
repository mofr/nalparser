#pragma once

#include <iostream>
#include <thread>

struct Arguments
{
    const char * filename;
    int threadCount = std::thread::hardware_concurrency();
    const char * configFilename = "config.xml";

    Arguments(int argc, char ** argv)
    {
        if(argc < 2)
        {
            std::cerr << "Usage: executable FILENAME [THREADS] [CONFIG_FILENAME]" << std::endl;
            std::exit(1);
        }

        filename = argv[1];

        if(argc > 2)
        {
            threadCount = strtol(argv[2], nullptr, 10);
            if(threadCount <= 0)
            {
                std::cerr << "Invalid thread count: " << argv[2] << std::endl;
                std::exit(1);
            }
        }

        if(argc > 3)
        {
            configFilename = argv[3];
        }
    }
};