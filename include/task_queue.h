# pragma once
#include <iostream>
#include <queue>
#include <mutex>
#include <condition_variable>
// #include "thread_pool.h"

using callback = void (*)(void* arg, void* pool);

class Task{
public:
    Task(callback func = nullptr, void* arg = nullptr) : function(func), arg(arg){};

    callback function;
    void* arg;
};

class task_queue
{
public:
    task_queue() {

    };
    // 添加任务
    void add_task_Q(Task task);
    // void add_task(callback func, void* arg);

    // 取出任务
    Task takeTask();

    // 获取当前任务队列个数
    inline int get_task_number() {
        int n = my_queue.size();
        return n;
    }
private:
    /* data */
    std::mutex m_mtx;
    std::queue<Task> my_queue;
};

