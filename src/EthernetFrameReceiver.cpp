#include "EthernetFrameReceiver.h"

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/epoll.h>
#include <linux/if_packet.h>
#include <net/if.h>
#include <linux/if_ether.h>
#include <arpa/inet.h>
#include <chrono>

#include "Config.h"

#define FRAME_LEN 46
#define LAST_ERROR_STATUS Status::error(std::strerror(errno))

namespace discoveryservice::daemon::io
{

FrameReceiver::~FrameReceiver()
{
    if (m_epollFd >= 0) {
        close(m_epollFd);
    }
}

Status FrameReceiver::initInputSockets(
    const std::vector<std::string>& ifNames, 
    std::vector<EthInterface>& ethInterfaces)
{
    m_epollFd = epoll_create1(0);

    for (const auto& ifName : ifNames) { 
        EthInterface ethInterface {.name = ifName};

        if ((ethInterface.socketFd = socket(AF_PACKET, SOCK_RAW | SOCK_NONBLOCK, htons(ETH_P_ALL))) < 0) {
            return LAST_ERROR_STATUS;
        }

        if ((ethInterface.index = lookupInterfaceIndex(ethInterface.socketFd, ifName)) < 0) {
            return LAST_ERROR_STATUS;
        }

        // bind to receive frames with specific protocol
        sockaddr_ll sll;
        std::memset(&sll, 0, sizeof(sll));
        sll.sll_family = AF_PACKET;
        sll.sll_ifindex = ethInterface.index;
        sll.sll_protocol = htons(0x88B5);
        if (bind(ethInterface.socketFd, (sockaddr*)&sll, sizeof(sll)) < 0) {
            return LAST_ERROR_STATUS;
        }

        epoll_event event;
        std::memset(&event, 0, sizeof(event));
        event.data.fd = ethInterface.socketFd;
        event.events = EPOLLIN;

        epoll_ctl(m_epollFd, EPOLL_CTL_ADD, ethInterface.socketFd, &event);
        m_epollEvents.push_back(std::move(event));

        ethInterfaces.push_back(std::move(ethInterface));
    }

    return Status::success();
}

std::vector<EthernetConnection> FrameReceiver::pollInputFrames(Config* config)
{
    std::vector<epoll_event> repollFds {m_epollEvents};
    std::vector<EthernetConnection> newConnections;

    int evCount = epoll_wait(m_epollFd, repollFds.data(), repollFds.size(), 0);
    for (int i {}; i < evCount; ++i) {
        if (repollFds[i].events & EPOLLIN) {
            std::cout << "got the event for our socket" << std::endl; // remove!

            ssize_t numBytes {recvfrom(repollFds[i].data.fd, m_frame, INPUT_FRAME_SIZE, 0, nullptr, nullptr)};
            if (numBytes < 0) {
                if (errno != EWOULDBLOCK && errno != EAGAIN) {
                    perror("recvfrom");
                }
            }
            else if (numBytes >= FRAME_LEN) {
                EthernetConnection connection {.socketFd = repollFds[i].data.fd};
                std::memcpy(connection.deviceMac.data(), m_frame + 6, 6);

                const std::string& token {config->getServiceId()};
                if (std::memcmp(m_frame + 14, token.c_str(), 32) != 0) {
                    std::cout << "wrong service token received, skipping the frame" << std::endl;
                    continue;
                }

                connection.lastTimeSeenActiveMsec = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now().time_since_epoch())
                    .count();

                newConnections.push_back(std::move(connection));
            }
        }
    }

    return newConnections;
}

} // namespace discoveryservice::daemon::io