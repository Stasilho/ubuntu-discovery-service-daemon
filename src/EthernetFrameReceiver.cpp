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

namespace discoveryservice::daemon::io
{

FrameReceiver::~FrameReceiver()
{
    if (m_socket >= 0) {
        close(m_socket);
    }

    if (m_epollFd >= 0) {
        close(m_epollFd);
    }

    delete [] m_frame;
    m_frame = nullptr;
}

int FrameReceiver::initSocket()
{
    const char* interfaceName {"enp0s8"};
    //const char* want {"DISCOVER_FROM_DEVICE"};
    //const size_t wantLength {std::strlen(want)};

    // open AF_PACKET socket
    m_socket = socket(AF_PACKET, SOCK_RAW | SOCK_NONBLOCK, htons(ETH_P_ALL));  //  ETH_P_ALL, 0x88B5
    if (m_socket < 0) {
        std::perror("socket");
        return 1;
    }
    std::cout << "receiver socket is: " << m_socket << std::endl;

    // Lookup interface index
    ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, interfaceName, IFNAMSIZ - 1);

    if (ioctl(m_socket, SIOCGIFINDEX, &ifr) < 0) {
        perror("SIOCGIFINDEX");
        return 1;
    }
    int ifIndex {ifr.ifr_ifindex};
    std::cout << "receiver ifIndex is: " << ifIndex << std::endl;

    // bind to the interface to receive only frames for this interface
    sockaddr_ll sll;
    std::memset(&sll, 0, sizeof(sll));
    sll.sll_family = AF_PACKET;
    sll.sll_ifindex = ifIndex;
    sll.sll_protocol = htons(0x88B5);
    if (bind(m_socket, (sockaddr*)&sll, sizeof(sll)) < 0) {
        perror("bind");
        return 1;
    }

    // timer
    //int tfd {timer_f}

    //pollfd fds[1];

    m_epollFd = epoll_create1(0);
    m_epollEvent.data.fd = m_socket;
    m_epollEvent.events = EPOLLIN;

    epoll_ctl(m_epollFd, EPOLL_CTL_ADD, m_socket, &m_epollEvent);
    perror("epoll_ctl");

    //if (fds[0].revents & POLLIN) {
        //std::cout << "frame recieived!!!";
    //}


    return 0;
}

void FrameReceiver::pollFrame()
{
    epoll_event repollFds[1];

    //std::cout << "waiting for frame..." << std::endl;
    int evCount = epoll_wait(m_epollFd, repollFds, 1, 0);
    //std::cout << "received events: " << evCount << std::endl;
    for (int i {}; i < evCount; ++i) {
        if (repollFds[i].events & EPOLLIN) {
            std::cout << "got the event for our socket" << std::endl;

            ssize_t n {recvfrom(m_socket, m_frame, m_frameSize, 0, nullptr, nullptr)};
            if (n < 0) {
                if (errno != EWOULDBLOCK && errno != EAGAIN) {
                    perror("recvfrom");
                }
            }
            else {
                std::cout << "received " << n << " bytes." << std::endl;
            }
        }
    }
}

} // namespace discoveryservice::daemon::io