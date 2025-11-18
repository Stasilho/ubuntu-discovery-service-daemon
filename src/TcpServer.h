#pragma once

#include <sys/epoll.h>

namespace discoveryservice::daemon
{
class Config;
class TcpRequestProcessor;
}



namespace discoveryservice::daemon::io
{

class Status;

/**
 * Intended for IPC with a client CLI
 */
class TcpServer
{
public:
    virtual ~TcpServer();

    Status initConnection(Config* config);

    void handleClientConnection(TcpRequestProcessor& requestProcessor);

private:
    int m_epollFd {-1};
    int m_socketFd {-1};
};

} // namespace discoveryservice::daemon::io