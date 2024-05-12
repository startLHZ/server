#include <iostream>
#include "../include/thread_pool.h"
#include "server.h"
#include <sys/socket.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <arpa/inet.h>
#include "logger.h"


my_server::my_server() {
    this->stop_server = false;
    log_init();
    this->server_thread_pool = new m_thread_pool<m_user*>();
    this->epoll_init();
    INFOLOG("server init complete!");
}

my_server::~my_server() {
    INFOLOG("server close");
    delete server_thread_pool;
}

void my_server::mainLoop() {
    while (!stop_server) {
        int number = epoll_wait(epfd, events, size, -1);
        for (int i = 0; i < number; i ++) {
            int sockfd = events[i].data.fd;
            if(sockfd == lfd) {
                INFOLOG("new connect");
                // 建立新连接
                int cfd = accept(sockfd, NULL, NULL);
                // 将文件描述符设置为非阻塞
                // 得到文件描述符的属性
                int flag = fcntl(cfd, F_GETFL);
                flag |= O_NONBLOCK;
                fcntl(cfd, F_SETFL, flag);
                // 新得到的文件描述符添加到epoll模型中, 下一轮循环的时候就可以被检测了
                // 通信的文件描述符检测读缓冲区数据的时候设置为边沿模式
                ev.events = EPOLLIN | EPOLLET;    // 读缓冲区是否有数据
                ev.data.fd = cfd;
                ret = epoll_ctl(epfd, EPOLL_CTL_ADD, cfd, &ev);
                if(ret == -1)
                {
                    perror("epoll_ctl-accept");
                    exit(0);
                }
            } else if (events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
                epoll_ctl(epfd, EPOLL_CTL_DEL, sockfd, NULL);
                close(sockfd);
            } else if (events[i].events & EPOLLIN) {
                dealwithread(sockfd);
                INFOLOG("EPOLLIN");
            }
            else if (events[i].events & EPOLLOUT) {
                dealwithwrite(sockfd);
                INFOLOG("EPOLLOUT");
            }
        }
    }
}

void my_server::epoll_init() {
    // 创建监听的套接字
    lfd = socket(AF_INET, SOCK_STREAM, 0);
    if(lfd == -1)
    {
        perror("socket error");
        ERRORLOG("socket error");
        exit(1);
    }
    int flag = fcntl(lfd, F_GETFL);
    flag |= O_NONBLOCK;
    fcntl(lfd, F_SETFL, flag);
    

    // 绑定
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(10001);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);  // 本地多有的ＩＰ
    // 127.0.0.1
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr.s_addr);
    
    // 设置端口复用
    int opt = 1;
    setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // 绑定端口
    ret = bind(lfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    if(ret == -1)
    {
        perror("bind error");
        ERRORLOG("bind error");
        exit(1);
    }

    // 监听
    ret = listen(lfd, 64);
    if(ret == -1)
    {
        perror("listen error");
        ERRORLOG("listen error");
        exit(1);
    }

    // 现在只有监听的文件描述符
    // 所有的文件描述符对应读写缓冲区状态都是委托内核进行检测的epoll
    // 创建一个epoll模型
    epfd = epoll_create(100);
    if(epfd == -1)
    {
        perror("epoll_create");
        ERRORLOG("epoll create error");
        exit(0);
    }
    this->server_thread_pool->epfd = this->epfd;

    // 往epoll实例中添加需要检测的节点, 现在只有监听的文件描述符
    ev.events = EPOLLIN;    // 检测lfd读读缓冲区是否有数据
    ev.data.fd = lfd;
    ret = epoll_ctl(epfd, EPOLL_CTL_ADD, lfd, &ev);
    if(ret == -1)
    {
        perror("epoll_ctl");
        exit(0);
    }

    struct epoll_event evs[10000];
    size = sizeof(evs) / sizeof(struct epoll_event);
}

void my_server::log_init() {
    	// 定义日志配置项
    LogConfig conf = {
        .level = "trace",
        .path  = "logger.log",
        .size  = 5 * 1024 * 1024,
        .count = 10,
    };
    INITLOG(conf);
}

void my_server::dealwithread(int sockfd) {
    m_user *request = new m_user(sockfd, 0);
    server_thread_pool->manager(request);
}

void my_server::dealwithwrite(int sockfd) {
    m_user *request = new m_user(sockfd, 1);
    server_thread_pool->manager(request);
}