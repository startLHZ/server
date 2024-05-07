#include "../include/thread_pool.h"
#include <vector>
#include <thread>
#include <mutex>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>
#include "logger.h"

thread_pool::thread_pool() : shutdown(false), busyNum(0), aliveNum(0), minNum(10)
{
    taskQ = new task_queue;
    mtx.lock();
    std::unique_lock<std::mutex> locker(pool_mtx, std::defer_lock);
    locker.lock();

    // threadIDs.push_back(std::thread(manager,this));
    // aliveNum ++;
    aliveNum += 10;
    for (int i = 0; i < 10; ++ i) {
        threadIDs.push_back(std::thread(&worker, this));
    }       
    locker.unlock();
}

thread_pool::~thread_pool() {
    shutdown = true;
    for (int i  = 0; i < threadIDs.size(); ++ i) {
        notEmpty.notify_one();
        if(threadIDs[i].joinable()) {
            threadIDs[i].join(); // 等待任务结束， 前提：线程一定会执行完
        } 
    }
    mtx.unlock();
    main_condition.notify_one();
    return;
}

void* thread_pool::worker(void* arg) {
    thread_pool* pool = static_cast<thread_pool*>(arg);
    while (1)
    {
        std::unique_lock<std::mutex> locker(pool->pool_mtx);
        pool->notEmpty.wait(locker, [&]()->bool{ return pool->get_taskQ_size() || pool->if_shutdown();});
        if (pool->if_shutdown()) {
            pool->aliveNum --;
            locker.unlock();
            pool->notEmpty.notify_one();
            return nullptr;
        }

        auto curTask = pool->taskQ->takeTask();
        pool->busyNum ++;

        locker.unlock();
        
        curTask.function(curTask.fd, curTask.epfd);
        INFOLOG("worker get task");

        locker.lock();
        pool->busyNum --;
        locker.unlock();
    }
}

void thread_pool::add_task(Task task) {
    if (shutdown) return ;
    // std::unique_lock<std::mutex> locker(pool_mtx, std::defer_lock);
    // locker.lock();
    taskQ->add_task_Q(task);
    INFOLOG("taskQueue add task");
    // locker.unlock();
    notEmpty.notify_one();
}

void taskRead(int arg, int epfd) {
    int sockfd = arg;
    char buf[64];
    memset(buf, 0, sizeof(buf));
    // 循环读数据
    while(1)
    {
        int len = recv(sockfd, buf, sizeof(buf), 0);
        if(len == 0) {
            // 非阻塞模式下和阻塞模式是一样的 => 判断对方是否断开连接
            INFOLOG("client close");
            // 将这个文件描述符从epoll模型中删除
            epoll_ctl(epfd, EPOLL_CTL_DEL, sockfd, NULL);
            close(sockfd);
            break;
        } else if (len > 0) {
            // 通信
            // 接收的数据打印到终端
            write(STDOUT_FILENO, buf, len);
            std::cout << std::endl;
            INFOLOG("client write");
            // // 发送数据
            // send(curfd, buf, len, 0);
        } else {
            // len == -1
            if(errno == EAGAIN)
                break;
            else {
                perror("recv");
                ERRORLOG("recv error!");
                exit(0);
            }
        }
    }
}

void taskWrite(int arg, int epfd) {
    int sockfd = arg;
    char buf[64] = "server send message";
    // memset(buf, 0, sizeof(buf));
    // char msg[64] = "server send message";
    // 发送数据
    send(sockfd, buf, sizeof(buf), 0);
    INFOLOG("server send message");
}

void thread_pool::manager(int sockfd, int flag, int epfd) {
   
    std::unique_lock<std::mutex> locker(pool_mtx, std::defer_lock);
    locker.lock();
    if (shutdown) {
        aliveNum --;
        locker.unlock();
        notEmpty.notify_one();
        return ;
    }

        
    Task task;
    if (flag == 0){
        task.function = taskWrite;
        task.epfd = epfd;
    } else {
        task.function = taskRead;
        task.epfd = epfd;
    }
    task.fd = sockfd;
    
    add_task(task);
    locker.unlock();
}


