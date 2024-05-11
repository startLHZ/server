#include <iostream>

class m_user
{
public:
    m_user(int fd, int s) : sockfd(fd), state(s) {};

    int sockfd;
    int state;
private:
    
};

