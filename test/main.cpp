#include "ThreadPool.h"
#include <iostream>
#include <vector>
#include <chrono>


void exampleTask(int n)
{
    std::cout << "Task " << n << " if starting." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "Task " << n << " is completed." << std::endl;
}

int main()
{
    ThreadPool pool(4);

    std::vector<std::future<void>> results;

    for(int i=0; i<8; ++i)
    {
        results.emplace_back(pool.enqueue(exampleTask, i));
    }

    for(auto && result : results)
    {
        result.get();
    }

    return 0;
}