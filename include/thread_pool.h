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
#include <string>
#include <sstream>
#include "logger.h"

// 读buf的大小
#define BUF_SIZE 2048
// 写buffer的大小
#define BUFFER_SIZE 64
#define FILENAME "../root/judge.html"

template<typename task_type>
class m_thread_pool 
{
public:
    static m_thread_pool<task_type>* getinstance_thradPool() {
        return instance;
    }

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

    int extractAction(const char* buffer, size_t length);

    // 创建线程池，构造函数
    m_thread_pool(int n = 5);

private:
    std::condition_variable notEmpty;    //条件变量，队列不为空
    std::vector<std::thread> threadIDs;  //工作的线程ID
    task_queue<task_type>* taskQ;        //任务队列
    int minNum;                          //最小线程数量
    int aliveNum;                        //存活线程数
    bool shutdown;                       //是不是要销毁线程池，销毁为1，不销毁为0
    int thread_nums;
    static m_thread_pool<task_type>* instance;
    
public:
    std::mutex pool_mtx;                 //线程池的锁
    int epfd;                            //关闭单个用户连接时使用
};

// 饿汉式初始化静态成员变量instance
template<typename task_type>
m_thread_pool<task_type>* m_thread_pool<task_type>::instance = new m_thread_pool<task_type>;

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
        notEmpty.notify_all();
        if(threadIDs[i].joinable()) {
            threadIDs[i].join(); // 等待任务结束， 前提：线程一定会执行完
        } 
    }
}

template<typename task_type>
void* m_thread_pool<task_type>::worker(void* arg) {
    m_thread_pool<task_type>* pool = static_cast<m_thread_pool<task_type>*>(arg);
    while (1) {
        std::unique_lock<std::mutex> locker(pool->pool_mtx);
        while (pool->get_taskQ_size() == 0) {
            pool->notEmpty.wait(locker);
            // 是否销毁线程
        }
        task_type task = pool->taskQ->takeTask();
        locker.unlock();

        if (task->state == 0) {
            pool->taskRead(task->sockfd, -1);
        } else {
            pool->taskWrite(task->sockfd, -1);
        }
    }
}

template<typename task_type>
void m_thread_pool<task_type>::taskRead(int arg, int epfd) {
    int sockfd = arg;
    char buf[BUF_SIZE];
    memset(buf, 0, sizeof(buf));
    int action;
    // 循环读数据
    while(1) { 
        int len = recv(sockfd, buf, sizeof(buf), 0);
        if(len == 0) {
            // 非阻塞模式下和阻塞模式是一样的 => 判断对方是否断开连接
            // 将这个文件描述符从epoll模型中删除
            epoll_ctl(epfd, EPOLL_CTL_DEL, sockfd, NULL);
            close(sockfd);
            INFOLOG("client close");
            return;
        } else if (len > 0) {
            // 通信
            // 接收的数据打印到终端
            // write(STDOUT_FILENO, buf, len);
            INFOLOG(buf);
            action = -1;
            action = extractAction(buf, BUF_SIZE);
        } else {
            // len == -1
            if(errno == EAGAIN)
                break;
            else {
                perror("recv");
                ERRORLOG("recv error!");
                epoll_ctl(epfd, EPOLL_CTL_DEL, sockfd, NULL);
                close(sockfd);
                INFOLOG("client close");
                break;
            }
        }
    }
    int n;
    FILE *file;
    char buffer[BUFFER_SIZE];

    // 打开文件
    switch (action)
    {
    case -1:
        file = fopen("../root/judge.html", "r");
        break;
    case 0:
        file = fopen("../root/test.html", "r");
        break;
    case 1:
        file = fopen("../root/index.html", "r");
        break;
    default:
        break;
    }
    
    if (file == NULL) {
        perror("Error opening file");
        exit(1);
    }

    // 构建HTTP响应头
    char header[BUFFER_SIZE];
    sprintf(header, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n");

    // 发送HTTP响应头
    if (send(sockfd, header, strlen(header), 0) < 0) {
        ERRORLOG("ERROR writing to socket");
    }

    // 读取文件内容并发送
    while ((n = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
        if (send(sockfd, buffer, n, 0) < 0) {
            perror("ERROR writing to socket");
            ERRORLOG("ERROR writing to socket");
            // exit(1);
            break;
        }
        // INFOLOG(buffer);
    }
    // 关闭文件
    fclose(file);

    // 关闭连接，不然webbench会fail
    epoll_ctl(epfd, EPOLL_CTL_DEL, sockfd, NULL);
    close(sockfd);
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

template<typename task_type>
int m_thread_pool<task_type>::extractAction(const char* buffer, size_t length) {
    std::istringstream ss(std::string(buffer, length)); // 将字符数组转换为字符串流
    std::string line;
    int action = -1; // 默认值为 -1，表示未找到 action

    // 读取请求行
    if (!std::getline(ss, line)) {
        std::cerr << "Error: Empty request" << std::endl;
        return action;
    }

    // 读取请求头
    while (std::getline(ss, line) && !line.empty()) {
        // 检查是否包含 action 属性
        if (line.find("9006/0") != std::string::npos) {
            action = 0;
            break;
        } else if (line.find("9006/1") != std::string::npos) {
            action = 1;
            break;
        }
    }

    return action;
}