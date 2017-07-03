#ifndef __SOCKET_GUARD_H__
#define __SOCKET_GUARD_H__

#include <unistd.h>

class CloseSocket
{
    int socket_;
public:
    CloseSocket(int s) : socket_(s){}
    ~CloseSocket()
    {
        if (socket_>=0)
        {
            close(socket_);
        }
    }
};


#endif

