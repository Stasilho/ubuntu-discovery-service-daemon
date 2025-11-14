#include "EthernetFrameSender.h"

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/if_packet.h>
#include <net/if.h>
#include <linux/if_ether.h>
#include <arpa/inet.h>

namespace discoveryservice::daemon::io
{

FrameSender::~FrameSender()
{
    if (m_socket >= 0) {
        close(m_socket);
    }

    delete [] m_frame;
    m_frame = nullptr;
}

int FrameSender::sendFrame()
{
    const char* interfaceName {"enp0s8"};

    m_socket = socket(AF_PACKET, SOCK_RAW, htons(0x88B5));  // ETH_P_ALL
    if (m_socket < 0) {
        std::cout << "socket is -1" << std::endl;
        return 1;
    }
    std::cout << "sender socket is: " << m_socket << std::endl;

    ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, interfaceName, IFNAMSIZ - 1);

    if (ioctl(m_socket, SIOCGIFINDEX, &ifr) < 0) {
        perror("SIOCGIFINDEX");
        return 1;
    }
    int ifIndex {ifr.ifr_ifindex};
    std::cout << "sender ifIndex is: " << ifIndex << std::endl;

    if (ioctl(m_socket, SIOCGIFHWADDR, &ifr) < 0) {
        perror("SIOCGIFHWADDR");
        return 1;
    }
    unsigned char src_mac[6];
    std::memcpy(src_mac, ifr.ifr_hwaddr.sa_data, 6);

    // build Ethernet frame
    const unsigned char dst_mac[6] {0xff};
    const uint16_t ethertype {0x88B5};
    const unsigned char payload[] = "DISCOVER_FROM_DEVICE";
    const size_t payloadLength = sizeof(payload);

    const size_t frameLength {14 + payloadLength};

    if (frameLength > 1500 + 14) {
        // error
        std::cout << "payload to large!" << std::endl;
        return 1;
    }

    m_frame = new unsigned char[frameLength];

    std::memcpy(m_frame + 0, dst_mac, 6);
    std::memcpy(m_frame + 6, src_mac, 6);
    uint16_t ethertype_be {htons(ethertype)};
    std::memcpy(m_frame + 12, &ethertype_be, 2);

    std::memcpy(m_frame + 14, payload, payloadLength);

    // prepare for sending
    sockaddr_ll sll;
    std::memset(&sll, 0, sizeof(sll));
    sll.sll_family = AF_PACKET;
    sll.sll_ifindex = ifIndex;
    sll.sll_halen = ETH_ALEN;
    std::memcpy(sll.sll_addr, dst_mac, 6);

    // optionally set socket to allow broadcast
    int on {1};
    if (setsockopt(m_socket, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on)) < 0) {
        // warn
        std::cout << "failed to set socket to broadcast";
    }

    // send frame
    ssize_t sent {sendto(m_socket, m_frame, frameLength, 0, (sockaddr*)&sll, sizeof(sll))};
    if (sent < 0) {
        // error
        std::cout << "failed to sent frame";
        return 1;
    }
    else if ((size_t) sent != frameLength) {
        // error
        std::cout << "partial send";
        return 1;
    }

    std::cout << "frame sent successfully" << std::endl;


    return 0;
}

} // namespace discoveryservice::daemon::io