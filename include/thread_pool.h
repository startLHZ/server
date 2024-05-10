#pragma once
#include <iostream>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <unordered_map>
#include "task_queue.h"
#include <sys/socket.h>
#include <sys/epoll.h>
#include <vector>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include "logger.h"

template<typename task_type>
class m_thread_pool 
{
public:
    // 创建线程池，构造函数
    m_thread_pool(int n = 5);

    // 销毁线程池，析构函数
    ~m_thread_pool();

    // 获取线程池中工作的线程数
    inline int get_busyNum() {
        return this->busyNum;
    }

    // 获取线程池中活着的线程数
    inline int get_aliveNum() {
        return this->aliveNum;
    }

    inline bool if_shutdown() {
        return this->shutdown;
    }

    inline int get_taskQ_size() {
        return taskQ->get_task_number();
    }

    // 工作的线程任务函数
    static void* worker(void* arg);

    // 管理者线程任务函数
    void manager(task_type cfd);
private:
    void taskRead(int arg, int epfd);
    void taskWrite(int arg, int epfd);

private:
    std::condition_variable notEmpty;    //条件变量，队列不为空
    std::vector<std::thread> threadIDs;  //工作的线程ID
    task_queue<task_type>* taskQ;                   //任务队列
    int minNum;               //最小线程数量
    int aliveNum;             //存活线程数
    bool shutdown;    //是不是要销毁线程池，销毁为1，不销毁为0
    int thread_nums;
public:
    std::mutex pool_mtx;                 //线程池的锁
    std::condition_variable main_condition;
    int lfdt;
};

template<typename task_type>
m_thread_pool<task_type>::m_thread_pool(int n) : shutdown(false), aliveNum(n), thread_nums(n)
{
    taskQ = new task_queue<task_type>();

    std::unique_lock<std::mutex> locker(pool_mtx);

    for (int i = 0; i < thread_nums; ++ i) {
        threadIDs.push_back(std::thread(&worker, this));
    }       
    locker.unlock();
}

template<typename task_type>
m_thread_pool<task_type>::~m_thread_pool() {
    shutdown = true;
    for (int i  = 0; i < threadIDs.size(); ++ i) {
        notEmpty.notify_one();
        if(threadIDs[i].joinable()) {
            threadIDs[i].join(); // 等待任务结束， 前提：线程一定会执行完
        } 
    }

    main_condition.notify_one();
    return;
}

template<typename task_type>
void* m_thread_pool<task_type>::worker(void* arg) {
    m_thread_pool<task_type>* pool = static_cast<m_thread_pool<task_type>*>(arg);
    
    // 创建一个epoll模型
    int epfd = -1;
    struct epoll_event ev;
    struct epoll_event events[1024];
    epfd = epoll_create(100);
    if(epfd == -1)
    {
        perror("epoll_create");
        ERRORLOG("epoll create error");
        exit(0);
    }

    // 往epoll实例中添加需要检测的节点, 现在只有监听的文件描述符
    ev.events = EPOLLIN;    // 检测lfd读读缓冲区是否有数据
    ev.data.fd = pool->lfdt;
    int ret = epoll_ctl(epfd, EPOLL_CTL_ADD, pool->lfdt, &ev);
    if(ret == -1)
    {
        perror("epoll_ctl");
        exit(0);
    }

    struct epoll_event evs[1024];
    int size = sizeof(evs) / sizeof(struct epoll_event);

    while (1)
    {
        std::unique_lock<std::mutex> locker(pool->pool_mtx, std::defer_lock);
        if (locker.try_lock()) {
            int cfd = pool->taskQ->takeTask();
            ev.events = EPOLLIN | EPOLLET;
            ev.data.fd = cfd;
            ret = epoll_ctl(epfd, EPOLL_CTL_ADD, cfd, &ev);
            locker.unlock();
        }

        int number = epoll_wait(epfd, events, size, -1);
        for (int i = 0; i < number; i ++) {
            int sockfd = events[i].data.fd;

            if (sockfd == pool->lfdt) {
                // 建立新连接
            } else if (events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
                // 服务器端关闭连接
                epoll_ctl(epfd, EPOLL_CTL_DEL, sockfd, NULL);
                close(sockfd);
            } else if (events[i].events & EPOLLIN) {
                // 读事件
                void taskRead(int sockfd, int epfd);
                // m_thread_pool->manager(sockfd, 0, epfd);
            } else {
                // 写事件
                // m_thread_pool->manager(sockfd, 0);
            }
        }
    }
}

template<typename task_type>
void m_thread_pool<task_type>::taskRead(int arg, int epfd) {
    int sockfd = arg;
    char buf[64];
    memset(buf, 0, sizeof(buf));
    // 循环读数据
    while(1)
    {
        int len = recv(sockfd, buf, sizeof(buf), 0);
        if(len == 0) {
            // 非阻塞模式下和阻塞模式是一样的 => 判断对方是否断开连接
            // 将这个文件描述符从epoll模型中删除
            epoll_ctl(epfd, EPOLL_CTL_DEL, sockfd, NULL);
            close(sockfd);
            // INFOLOG("client close");
            break;
        } else if (len > 0) {
            // 通信
            // 接收的数据打印到终端
            // write(STDOUT_FILENO, buf, len);
            // INFOLOG("client write");
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

template<typename task_type>
void m_thread_pool<task_type>::taskWrite(int arg, int epfd) {
    int sockfd = arg;
    char buf[64] = "server send message";
    send(sockfd, buf, sizeof(buf), 0);
    INFOLOG("server send message");
}

template<typename task_type>
void m_thread_pool<task_type>::manager(task_type cfd) {
    std::unique_lock<std::mutex> locker(pool_mtx);
    taskQ->add_task_Q(cfd);
    notEmpty.notify_one();
}
