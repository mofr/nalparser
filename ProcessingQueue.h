#pragma once

#include "BlockingQueue.h"
#include <thread>
#include <list>

template<typename In, typename Out>
class ProcessingQueue
{
    typedef int Index;

    struct Task
    {
        Index index;
        In in;
    };

public:
    typedef std::function<bool(In&)> InputFunction;
    typedef std::function<void(const In&, Out&)> ProcessFunction;
    typedef std::function<void(const Out&)> OutputFunction;

    ProcessingQueue()
    {}

    void start(int threadCount, InputFunction input, ProcessFunction process, OutputFunction output)
    {
        inputQueue.setMaxSize(threadCount * 2);
        outputIndex = 0;

        for(int i = 0; i < threadCount; ++i)
        {
            workers.push_back(std::thread(&ProcessingQueue::worker, this));
        }

        this->process = process;
        this->output = output;

        Index index = 0;
        In in;
        while(input(in))
        {
            inputQueue.push({index, in});
            ++index;
        }
        inputQueue.close();
    }

    void waitForFinished()
    {
        for(auto & worker : workers)
        {
            worker.join();
        }
    }

private:
    void worker()
    {
        Task task;
        while(inputQueue.pop(task))
        {
            Out out;
            process(task.in, out);
            collect(task.index, out);
        }
    }

    void collect(Index index, Out & out)
    {
        std::unique_lock<std::mutex> lock(outputMutex);
        if(outputIndex == index)
        {
            output(out);
            ++outputIndex;
            bool found;
            do
            {
                found = false;
                for(auto out2 = std::begin(outputList); out2 != std::end(outputList); ++out2)
                {
                    if(outputIndex == out2->first)
                    {
                        output(out2->second);
                        outputList.erase(out2);
                        ++outputIndex;
                        found = true;
                        break;
                    }
                }
            }
            while(found);
        }
        else
        {
            outputList.push_back(std::make_pair(index, out));
        }
    }

private:
    BlockingQueue<Task> inputQueue;
    std::vector<std::thread> workers;

    std::mutex outputMutex;
    Index outputIndex = 0;
    std::list<std::pair<Index, Out>> outputList;

    ProcessFunction process;
    OutputFunction output;
};