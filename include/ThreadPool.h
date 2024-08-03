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

// 添加新任务到任务队列
template<class F, class... Args>
auto ThreadPool::enqueue(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type> // 推导函数返回类型
{
    using return_type = typename std::result_of<F(Args...)>::type;

    auto task = std::make_shared<std::packaged_task<return_type()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));

    std::future<return_type> res = task->get_future();

    {
        std::unique_lock<std::mutex> lock(queue_mutex);

        if(stop)
        {
            throw std::runtime_error("enqueue on stopped ThreadPool");
        }

        tasks.emplace([task](){(*task)();});
    }
    condition.notify_one(); //触发之后，直到有等待线程
    return res;
}