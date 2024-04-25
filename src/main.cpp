#include "../include/task_queue.h"
#include "../include/thread_pool.h"
#include <unistd.h>
#include <iostream>
#include <string>



int main()
{
    // 创建线程池
    thread_pool* pool = new thread_pool;
    std::unique_lock<std::mutex> locker(pool->mtx);
    pool->main_condition.wait(locker, [&]()->bool{ return true;});
    while (1)
    {
        sleep(10);
    }
    

    return 0;
}