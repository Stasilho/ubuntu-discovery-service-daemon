#include "TcpServer.h"

#include <iostream>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>

#include "Config.h"
#include "Status.h"
#include "TcpRequestProcessor.h"

constexpr size_t maxPollEvents {10};
constexpr size_t inputBufferSize {4};

#define LAST_ERROR_STATUS Status::error(std::strerror(errno))

namespace
{
int setNonBlocking(int socketFd)
{
    int flags {fcntl(socketFd, F_GETFL, 0)};
    if (flags == -1) return -1;
    return fcntl(socketFd, F_SETFL, flags | O_NONBLOCK);
}

} // empty namespace



namespace discoveryservice::daemon::io
{

TcpServer::~TcpServer()
{
    if (m_socketFd != -1) {
        close(m_socketFd);
    }

    if (m_epollFd >= 0) {
        close(m_epollFd);
    }
}

Status TcpServer::initConnection(Config* config)
{
    m_epollFd = epoll_create1(0);

    if ((m_socketFd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        return LAST_ERROR_STATUS;
    }

    setNonBlocking(m_socketFd);

    int opt {1};
    sockaddr_in address;

    // bind to PORT
    if (setsockopt(m_socketFd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
        return LAST_ERROR_STATUS;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(config->ipcPort());

    if (bind(m_socketFd, (sockaddr*) &address, sizeof(address)) < 0) {
        return LAST_ERROR_STATUS;
    }

    if (listen(m_socketFd, 4) < 0) {
        return LAST_ERROR_STATUS;
    }

    // register event
    epoll_event event;
    std::memset(&event, 0, sizeof(event));
    event.data.fd = m_socketFd;
    event.events = EPOLLIN;

    epoll_ctl(m_epollFd, EPOLL_CTL_ADD, m_socketFd, &event);

    return Status::success();
}

void TcpServer::handleClientConnection(TcpRequestProcessor& requestProcessor)
{
    epoll_event event;
    std::memset(&event, 0, sizeof(event));

    epoll_event revents [maxPollEvents];

    int evCount {epoll_wait(m_epollFd, revents, maxPollEvents, 0)};
    for (size_t i {}; i < evCount; ++i) {
        if (revents[i].data.fd == m_socketFd) {
            // accept new connection
            std::cout << "got TCP connection..." << std::endl;
            int clientSocketId {accept(m_socketFd, nullptr, nullptr)};
            setNonBlocking(clientSocketId);
            event.events = EPOLLIN | EPOLLET;
            event.data.fd = clientSocketId;
            epoll_ctl(m_epollFd, EPOLL_CTL_ADD, clientSocketId, &event);
        }
        else {
            // handle client
            int clientSocketFd {revents[i].data.fd};
            if (revents[i].events & EPOLLIN) {
                char buffer[inputBufferSize];
                ssize_t count {read(clientSocketFd, buffer, sizeof(buffer))};
                if (count > 0) {
                    int code {};
                    memcpy(&code, buffer, sizeof(int));
                    
                    std::vector<char> resultBuffer;
                    requestProcessor.process(code, resultBuffer);

                    if (resultBuffer.size() > 0) {
                        write(clientSocketFd, resultBuffer.data(), resultBuffer.size());
                    }
                }
                else if (count == 0 || (count == -1 && errno != EAGAIN)) {
                    std::cout << "closing client socket" << std::endl;
                    close(clientSocketFd);
                    epoll_ctl(m_epollFd, EPOLL_CTL_DEL, clientSocketFd, nullptr);
                }
            }
        }
    }
}

} // namespace discoveryservice::daemon::io