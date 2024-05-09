#include <iostream>
#include "thread_pool.h"
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


my_server::my_server() {
    this->stop_server = false;
    log_init();
    this->epoll_init();
    this->m_thread_pool = new thread_pool;
    INFOLOG("server init complete!");
}

my_server::~my_server() {
    INFOLOG("server close");
    delete m_thread_pool;
}

void my_server::mainLoop() {
    while (!stop_server) {
         int number = epoll_wait(epfd, events, size, -1);
        for (int i = 0; i < number; i ++) {
            int sockfd = events[i].data.fd;

            if (sockfd == lfd) {
                // INFOLOG("new connect");
                // 建立新连接
                int cfd = accept(sockfd, NULL, NULL);
                int flag = fcntl(cfd, F_GETFL);
                flag |= O_NONBLOCK;
                fcntl(cfd, F_SETFL, flag);
                // 添加到epoll模型中，读缓冲区设置为边沿模式
                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = cfd;
                ret = epoll_ctl(epfd, EPOLL_CTL_ADD, cfd, &ev);
            } else if (events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
                // 服务器端关闭连接
                printf("客户端断开了连接...\n");
                // 将这个文件描述符从epoll模型中删除
                epoll_ctl(epfd, EPOLL_CTL_DEL, sockfd, NULL);
                close(sockfd);
            } else if (events[i].events & EPOLLIN) {
                // 读事件
                m_thread_pool->manager(sockfd, 1, epfd);
                // m_thread_pool->manager(sockfd, 0, epfd);
            } else {
                // 写事件
                // m_thread_pool->manager(sockfd, 0);
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

    // 绑定
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(10000);
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

    // 往epoll实例中添加需要检测的节点, 现在只有监听的文件描述符
    
    ev.events = EPOLLIN;    // 检测lfd读读缓冲区是否有数据
    ev.data.fd = lfd;
    ret = epoll_ctl(epfd, EPOLL_CTL_ADD, lfd, &ev);
    if(ret == -1)
    {
        perror("epoll_ctl");
        exit(0);
    }

    struct epoll_event evs[1024];
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