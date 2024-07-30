#pragma once

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <stdexcept>

class ThreadPool
{
public:
    ThreadPool(size_t);

    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type>;
   
    ~ThreadPool();

private:
    std::vector<std::thread> workers; //工作线程

    std::queue<std::function<void()>> tasks; // 任务队列

    // 同步
    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop;
};

