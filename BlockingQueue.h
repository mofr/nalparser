#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>

template<typename T>
class BlockingQueue
{
public:
    void push(const T & item)
    {
        std::unique_lock<std::mutex> lock(mutex);
        queue.push(item);
        condition.notify_one();
    }

    bool pop(T & item)
    {
        std::unique_lock<std::mutex> lock(mutex);
        condition.wait(lock, [this](){return !queue.empty() || closed;});
        if(closed && queue.empty())
        {
            return false;
        }
        else
        {
            item = queue.front();
            queue.pop();
            return true;
        }
    }

    void close()
    {
        std::unique_lock<std::mutex> lock(mutex);
        closed = true;
        condition.notify_all();
    }

private:
    std::queue<T> queue;
    mutable std::mutex mutex;
    std::condition_variable condition;
    bool closed = false;
};
