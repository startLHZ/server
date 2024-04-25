#include "../include/task_queue.h"


void task_queue::add_task_Q(Task task) {
    std::unique_lock<std::mutex> locker(this->m_mtx, std::defer_lock);
    locker.lock();
    this->my_queue.push(task);
    locker.unlock();
}

Task task_queue::takeTask() {
    Task getTask;
    std::unique_lock<std::mutex> locker(this->m_mtx, std::defer_lock);
    locker.lock();
    if (my_queue.size() > 0) {
        getTask = my_queue.front();
        my_queue.pop();
    }
    locker.unlock();
    return getTask;
}