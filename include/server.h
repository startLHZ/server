#pragma once
#include "thread_pool.h"

class my_server {
public:
    my_server();
    ~my_server();
    void epoll_init();
    void mainLoop();
private:
    // basic
    bool stop_server;

    // pool
    thread_pool* m_thread_pool;
    
    // epoll相关
    int lfd = -1;
    int epfd = -1;
    struct epoll_event ev;
    struct epoll_event events[1024];
    int size;
    int ret;


};