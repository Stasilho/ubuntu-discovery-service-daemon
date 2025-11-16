#pragma once

#include <sys/epoll.h>

namespace discoveryservice::daemon::io
{

class FrameReceiver
{
public:
    virtual ~FrameReceiver();

    int initSocket();
    void pollFrame();

private:
    int m_socket {-1};
    const size_t m_frameSize {65536};
    unsigned char* m_frame {new unsigned char[m_frameSize]};

    int m_epollFd {-1};
    epoll_event m_epollEvent;
};

} // namespace discoveryservice::daemon::io