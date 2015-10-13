#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>
#include <iostream>

template<typename T>
class BlockingQueue
{
public:
    BlockingQueue() :
            sizeLimited(false)
    {
    }

    BlockingQueue(int maxSize) :
            maxSize(maxSize),
            sizeLimited(true)
    {
    }

    void push(const T & item)
    {
        std::unique_lock<std::mutex> lock(mutex);
        std::cout << "push to queue " << queue.size() << std::endl;
        pushCondition.wait(lock, [this](){return queue.size() < maxSize || !sizeLimited;});
        queue.push(item);
        popCondition.notify_one();
        std::cout << "pushed " << queue.size() << std::endl;
    }

    bool pop(T & item)
    {
        std::unique_lock<std::mutex> lock(mutex);
        std::cout << "wait pop condition";
        popCondition.wait(lock, [this](){return !queue.empty() || closed;});
        std::cout << "pop" << queue.empty() << " " << queue.size() << std::endl;
        if(closed && queue.empty())
        {
            return false;
        }
        else
        {
            item = queue.front();
            queue.pop();
            pushCondition.notify_one();
            return true;
        }
    }

    void close()
    {
        std::unique_lock<std::mutex> lock(mutex);
        closed = true;
        popCondition.notify_all();
    }

    int size() const
    {
        std::unique_lock<std::mutex> lock(mutex);
        return queue.size();
    }

    void setMaxSize(int size)
    {
        std::unique_lock<std::mutex> lock(mutex);
        maxSize = size;
        pushCondition.notify_all();
    }

private:
    std::queue<T> queue;
    mutable std::mutex mutex;
    std::condition_variable popCondition;
    std::condition_variable pushCondition;
    bool closed = false;
    int maxSize;
    bool sizeLimited;
};
