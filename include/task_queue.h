# pragma once
#include <iostream>
#include <queue>
#include <mutex>
#include <condition_variable>
// #include "thread_pool.h"

template<typename task_type>
class task_queue
{
public:
    // 添加任务
    void add_task_Q(task_type cfd);

    // 取出任务
    task_type takeTask();

    // 获取当前任务队列个数
    inline int get_task_number() {
        int n = my_queue.size();
        return n;
    }
private:
    /* data */
    std::queue<task_type> my_queue;
};

template<typename task_type>
void task_queue<task_type>::add_task_Q(task_type cfd) {
    this->my_queue.push(cfd);
    return ;
}

template<typename task_type>
task_type task_queue<task_type>::takeTask() {
    task_type getTask;
    if (my_queue.size() > 0) {
        getTask = my_queue.front();
        my_queue.pop();
    }
    return getTask;
}