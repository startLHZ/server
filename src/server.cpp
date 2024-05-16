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
#include <signal.h>
#include "logger.h"
#include "lst_timer.h"


my_server::my_server(int argc, char*argv[]) {
    m_port = 9006;
    m_thread_num = 8;
    init_config(argc, argv);
    this->stop_server = false;
    timer_init();
    log_init();

    this->server_thread_pool = m_thread_pool<m_user*>::getinstance_thradPool();

    this->epoll_init();
    INFOLOG("server init complete!");
}

my_server::~my_server() {
    INFOLOG("server close");
    delete server_thread_pool;
}

void my_server::init_config(int argc, char*argv[]){
    int opt;
    const char *str = "p:t:";
    while ((opt = getopt(argc, argv, str)) != -1) {
        switch (opt) {
        case 'p': {
            this->m_port = atoi(optarg);
            break;
        }
        case 't': {
            this->m_thread_num = atoi(optarg);
            break;
        }
        default:
            break;
        }
    }
}

void my_server::mainLoop() {
    while (!stop_server) {
        int number = epoll_wait(epfd, events, size, -1);
        if (number < 0 && errno != EINTR) {
            ERRORLOG("epoll failure");
            break;
        }

        for (int i = 0; i < number; i ++) {
            int sockfd = events[i].data.fd;
            if(sockfd == lfd) {
                // 建立新连接
                int cfd = accept(sockfd, NULL, NULL);
                addfd(epfd, cfd, true);
                INFOLOG("new connect success");
            } else if (events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
                //服务器端关闭连接，移除对应的定时器
                util_timer *timer = users_timer[sockfd].timer;
                timer->cb_func(&users_timer[sockfd], this->epfd);

                if (timer) {
                    timer_lst.del_timer(timer);
                }
                INFOLOG("client close and timer delete");
            } else if (events[i].events & EPOLLIN) {
                util_timer *timer = users_timer[sockfd].timer;
                dealwithread(sockfd);
                if (timer) {
                    time_t cur = time(NULL);
                    timer->expire = cur + 3 * 5;
                    INFOLOG("%s", "adjust timer once");
                    timer_lst.adjust_timer(timer);
                }
                INFOLOG("EPOLLIN");
            }
            else if (events[i].events & EPOLLOUT) {
                util_timer *timer = users_timer[sockfd].timer;
                dealwithwrite(sockfd);
                //若有数据传输，则将定时器往后延迟3个单位
                //并对新的定时器在链表上的位置进行调整
                if (timer) {
                    time_t cur = time(NULL);
                    timer->expire = cur + 3 * 5;
                    INFOLOG("%s", "adjust timer once");
                    timer_lst.adjust_timer(timer);
                }
                INFOLOG("EPOLLOUT");
            }
        } if (timeout) {
            timer_handler();
            timeout = false;
        }
    }
}

void my_server::epoll_init() {
    // 创建监听的套接字
    lfd = socket(AF_INET, SOCK_STREAM, 0);
    if(lfd == -1) {
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
    serv_addr.sin_port = htons(m_port);
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

//定时器回调函数，删除非活动连接在socket上的注册事件，并关闭
void cb_func(client_data *user_data, int epfd)
{
    epoll_ctl(epfd, EPOLL_CTL_DEL, user_data->sockfd, 0);
    assert(user_data);
    close(user_data->sockfd);
    INFOLOG("close fd %d", user_data->sockfd);
}

//将内核事件表注册读事件，ET模式，选择开启EPOLLONESHOT
void my_server::addfd(int epollfd, int cfd, bool one_shot) {
    // 将文件描述符设置为非阻塞
    // 得到文件描述符的属性
    int flag = fcntl(cfd, F_GETFL);
    flag |= O_NONBLOCK;
    fcntl(cfd, F_SETFL, flag);
    epoll_event event;
    event.data.fd = cfd;
    event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;

    if (one_shot)
        event.events |= EPOLLONESHOT;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, cfd, &event);
    if(ret == -1) {
        perror("epoll_ctl-accept");
        exit(0);
    }
    //初始化client_data数据
    //创建定时器，设置回调函数和超时时间，绑定用户数据，将定时器添加到链表中
    users_timer[cfd].sockfd = cfd;
    util_timer *timer = new util_timer;
    timer->user_data = &users_timer[cfd];
    timer->cb_func = cb_func;
    time_t cur = time(NULL);
    timer->expire = cur + 3 * 5;
    users_timer[cfd].timer = timer;
    timer_lst.add_timer(timer);
    // setnonblocking(fd);
}

//从内核时间表删除描述符
void my_server::removefd(int epollfd, int fd)
{
    epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, 0);
    close(fd);
}

//将事件重置为EPOLLONESHOT
void my_server::modfd(int epollfd, int fd, int ev)
{
    epoll_event event;
    event.data.fd = fd;
    event.events = ev | EPOLLET | EPOLLONESHOT | EPOLLRDHUP;
    epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &event);
}

//设置信号函数
void my_server::addsig(int sig, bool restart)
{
    struct sigaction sa;
    memset(&sa, '\0', sizeof(sa));
    //为保证函数的可重入性，保留原来的errno
    int save_errno = errno;
    int msg = sig;
    send(pipefd[1], (char *)&msg, 1, 0);
    errno = save_errno;
    if (restart)
        sa.sa_flags |= SA_RESTART;
    sigfillset(&sa.sa_mask);
    assert(sigaction(sig, &sa, NULL) != -1);
}

//定时处理任务，重新定时以不断触发SIGALRM信号
void my_server::timer_handler()
{
    timer_lst.tick();
}

void my_server::timer_init()
{
    users_timer = new client_data[65535];
    this->timer_lst.init_timer_epfd(this->epfd);
    //创建管道
    ret = socketpair(PF_UNIX, SOCK_STREAM, 0, pipefd);
    assert(ret != -1);
    // setnonblocking(pipefd[1]);
    addfd(epfd, pipefd[0], false);

    addsig(SIGALRM, false);
    addsig(SIGTERM, false);
    bool stop_server = false;

    

    bool timeout = false;
    // alarm(TIMESLOT);
}