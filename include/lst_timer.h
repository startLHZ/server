#pragma once
#include <time.h>
#include "logger.h"

class util_timer;
struct client_data
{
    int sockfd;
    util_timer *timer;
};

class util_timer
{
public:
    util_timer() : prev(NULL), next(NULL) {}

public:
    time_t expire;
    void (*cb_func)(client_data *, int);
    client_data *user_data;
    util_timer *prev;
    util_timer *next;
};

class sort_timer_lst
{
public:
    sort_timer_lst() : head(NULL), tail(NULL), epfd(-1) {}
    ~sort_timer_lst();

    inline void init_timer_epfd(int fd) {
        this->epfd = fd;
    }

    void add_timer(util_timer *timer);
    void adjust_timer(util_timer *timer);

    void del_timer(util_timer *timer);
    void tick();

private:
    void add_timer(util_timer *timer, util_timer *lst_head);

private:
    util_timer *head;
    util_timer *tail;
    int epfd;
};

