#include "ThreadPool.h"

ThreadPool::ThreadPool(size_t threads) : stop(false)
{
    for (size_t i=0; i<threads; ++i)
    {
        workers.emplace_back(
            [this]
            {
                for( ; ;) // emplace_back 被调用时，它会立即返回，不会等待线程完成, 所以这里不会影响线程的创建的
                {
                    std::function<void()>task;

                    {
                        std::unique_lock<std::mutex> lock(this->queue_mutex);
                        this->condition.wait(lock, [this]{return this->stop || !this->tasks.empty();});
                        if(this->stop && this->tasks.empty())
                        {
                            return;
                        }
                        task = std::move(this->tasks.front());
                        this->tasks.pop();
                    }
                    task();
                }
            });
    }
}



// 析构函数
ThreadPool::~ThreadPool()
{
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        stop = true;
    }

    condition.notify_all();
    for(std::thread &worker : workers)
    {
        worker.join();
    }
}
