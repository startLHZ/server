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

thread_pool::thread_pool() : shutdown(false), busyNum(0), aliveNum(0), minNum(10)
{
    taskQ = new task_queue;

    std::unique_lock<std::mutex> locker(pool_mtx, std::defer_lock);
    locker.lock();

    // ***************************************************************************
    // 创建监听的套接字
    lfd = socket(AF_INET, SOCK_STREAM, 0);
    if(lfd == -1)
    {
        perror("socket error");
        exit(1);
    }

    // 绑定
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(10002);
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
        exit(1);
    }

    // 监听
    ret = listen(lfd, 64);
    if(ret == -1)
    {
        perror("listen error");
        exit(1);
    }

    // 现在只有监听的文件描述符
    // 所有的文件描述符对应读写缓冲区状态都是委托内核进行检测的epoll
    // 创建一个epoll模型
    epfd = epoll_create(100);
    if(epfd == -1)
    {
        perror("epoll_create");
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
    // *******************************************************************************


    threadIDs.push_back(std::thread(manager,this));
    aliveNum ++;
    make_thread(minNum);      
    locker.unlock();
}

thread_pool::~thread_pool() {
    shutdown = true;
    for (int i  = 0; i < threadIDs.size(); ++ i) {
        notEmpty.notify_one();
        if(threadIDs[i].joinable()){
            threadIDs[i].join(); // 等待任务结束， 前提：线程一定会执行完
        }     
    }
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


        curTask.function(curTask.arg, (void*) pool);


        locker.lock();
        pool->busyNum --;
        locker.unlock();
    }
}

void thread_pool::add_task(Task task) {
    if (shutdown) return ;
    std::unique_lock<std::mutex> locker(pool_mtx, std::defer_lock);
    // locker.lock();
    taskQ->add_task_Q(task);
    // locker.unlock();
    notEmpty.notify_one();
}

void taskFunc(void* arg, void* pool)
{
    int curfd = *(int*)arg;
    // thread_pool p = *(thread_pool*) pool;
    if(curfd == ((thread_pool*) pool)->lfd)
    {
        // 建立新的连接
        int cfd = accept(curfd, NULL, NULL);
        // 将文件描述符设置为非阻塞
        // 得到文件描述符的属性
        int flag = fcntl(cfd, F_GETFL);
        flag |= O_NONBLOCK;
        fcntl(cfd, F_SETFL, flag);
        // 新得到的文件描述符添加到epoll模型中, 下一轮循环的时候就可以被检测了
        // 通信的文件描述符检测读缓冲区数据的时候设置为边沿模式
        ((thread_pool*) pool)->ev.events = EPOLLIN | EPOLLET;    // 读缓冲区是否有数据
        ((thread_pool*) pool)->ev.data.fd = cfd;
        ((thread_pool*) pool)->ret = epoll_ctl(((thread_pool*) pool)->epfd, EPOLL_CTL_ADD, cfd, &((thread_pool*) pool)->ev);
        if(((thread_pool*) pool)->ret == -1)
        {
            perror("epoll_ctl-accept");
            exit(0);
        }
    }
    else
    {
        // 处理通信的文件描述符
        // 接收数据
        char buf[5];
        memset(buf, 0, sizeof(buf));
        // 循环读数据
        while(1)
        {
            int len = recv(curfd, buf, sizeof(buf), 0);
            if(len == 0)
            {
                // 非阻塞模式下和阻塞模式是一样的 => 判断对方是否断开连接
                printf("客户端断开了连接...\n");
                // 将这个文件描述符从epoll模型中删除
                epoll_ctl(((thread_pool*) pool)->epfd, EPOLL_CTL_DEL, curfd, NULL);
                close(curfd);
                break;
            }
            else if(len > 0)
            {
                // 通信
                // 接收的数据打印到终端
                write(STDOUT_FILENO, buf, len);
                // 发送数据
                send(curfd, buf, len, 0);
            }
            else
            {
                // len == -1
                if(errno == EAGAIN)
                {
                    printf("数据读完了...\n");
                    break;
                }
                else
                {
                    perror("recv");
                    exit(0);
                }
            }
        }
    }



}

void* thread_pool::manager(void* arg) {
//     // 创建监听的套接字
//     lfd = socket(AF_INET, SOCK_STREAM, 0);
//     if(lfd == -1)
//     {
//         perror("socket error");
//         exit(1);
//     }

//     // 绑定
//     struct sockaddr_in serv_addr;
//     memset(&serv_addr, 0, sizeof(serv_addr));
//     serv_addr.sin_family = AF_INET;
//     serv_addr.sin_port = htons(10002);
//     serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);  // 本地多有的ＩＰ
//     // 127.0.0.1
//     inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr.s_addr);
    
//     // 设置端口复用
//     int opt = 1;
//     setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

//     // 绑定端口
//     int ret = bind(lfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
//     if(ret == -1)
//     {
//         perror("bind error");
//         exit(1);
//     }

//     // 监听
//     ret = listen(lfd, 64);
//     if(ret == -1)
//     {
//         perror("listen error");
//         exit(1);
//     }

//     // 现在只有监听的文件描述符
//     // 所有的文件描述符对应读写缓冲区状态都是委托内核进行检测的epoll
//     // 创建一个epoll模型
//     epfd = epoll_create(100);
//     if(epfd == -1)
//     {
//         perror("epoll_create");
//         exit(0);
//     }

//     // 往epoll实例中添加需要检测的节点, 现在只有监听的文件描述符
    
//     ev.events = EPOLLIN;    // 检测lfd读读缓冲区是否有数据
//     ev.data.fd = lfd;
//     ret = epoll_ctl(epfd, EPOLL_CTL_ADD, lfd, &ev);
//     if(ret == -1)
//     {
//         perror("epoll_ctl");
//         exit(0);
//     }

//     struct epoll_event evs[1024];
//     int size = sizeof(evs) / sizeof(struct epoll_event);

    thread_pool* pool = static_cast<thread_pool*>(arg);
    while (1) {
        std::unique_lock<std::mutex> locker(pool->pool_mtx, std::defer_lock);
        locker.lock();
        if (pool->if_shutdown()) {
            pool->aliveNum --;
            locker.unlock();
            pool->notEmpty.notify_one();
            return nullptr;
        }
        locker.unlock();
        sleep(1);
        int num = epoll_wait(pool->epfd, pool->evs, pool->size, 0);
        while (num == 0) {
            sleep(3);
            num = epoll_wait(pool->epfd, pool->evs, pool->size, 0);
        }
        printf("==== num: %d\n", num);
        locker.lock();
        for(int i=0; i<num; ++i)
        {
            // 取出当前的文件描述符
            int curfd = pool->evs[i].data.fd;
            
            Task task;
            task.function = taskFunc;
            task.arg = &curfd;

            pool->add_task(task);
        
        }
        locker.unlock();
    }
}

void thread_pool::make_thread(int n) {
    // 不需要加锁
    aliveNum += n;
    for (int i = 0; i < n; ++ i) {
        threadIDs.push_back(std::thread(&worker, this));
    }  
}


