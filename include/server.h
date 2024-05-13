#pragma once
#include "thread_pool.h"
#include "logger.h"
#include "user.h"
#include "lst_timer.h"

class my_server {
public:
    my_server();
    ~my_server();
    void epoll_init();
    void mainLoop();
    void log_init();
    void addfd(int epollfd, int fd, bool one_shot);
    void removefd(int epollfd, int fd);
    void modfd(int epollfd, int fd, int ev);

    void timer_init();
    void addsig(int sig, bool restart = true);
    void timer_handler();
    // void cb_func(client_data *user_data);
private:
    // basic
    bool stop_server;
    void dealwithread(int sockfd);
    void dealwithwrite(int sockfd);

    // pool
    m_thread_pool<m_user*>* server_thread_pool;
    
    // epoll相关
    int lfd = -1;
    int epfd = -1;
    struct epoll_event ev;
    struct epoll_event events[1024];
    int size;
    int ret;

    // timer
    int pipefd[2];
    
    client_data *users_timer;
    bool timeout = false;
    sort_timer_lst timer_lst;
};