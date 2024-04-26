#include "../include/task_queue.h"
#include "../include/thread_pool.h"
#include <unistd.h>
#include <iostream>
#include <string>
#include "server.h"


int main()
{
    
    my_server m_server;
    m_server.mainLoop();
    return 0;
}