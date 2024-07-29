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

inline ThreadPool::ThreadPool(size_t threads) : stop(false)
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
    condition.notify_one();
    return res;
}

// 析构函数
inline ThreadPool::~ThreadPool()
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
