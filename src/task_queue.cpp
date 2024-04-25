#include "../include/task_queue.h"


void task_queue::add_task_Q(Task task) {
    std::unique_lock<std::mutex> locker(this->m_mtx);
    this->my_queue.push(task);
    return ;
}

Task task_queue::takeTask() {
    Task getTask;
    std::unique_lock<std::mutex> locker(this->m_mtx);
    if (my_queue.size() > 0) {
        getTask = my_queue.front();
        my_queue.pop();
    }
    return getTask;
}