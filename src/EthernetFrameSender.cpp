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

#include "Config.h"

#define HEADER_LEN 14
#define PAYLOAD_LEN 32
#define FRAME_LEN 46
#define LAST_ERROR_STATUS Status::error(std::strerror(errno))

using discoveryservice::daemon::Config;

namespace discoveryservice::daemon::io
{

Status FrameSender::initOutputSockets(
    const std::vector<std::string>& ifNames,
    std::vector<EthInterface>& ethInterfaces)
{
    for (const auto& ifName : ifNames) {        
        EthInterface ethInterface {.name = ifName};

        if ((ethInterface.socketFd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) < 0) {
            return LAST_ERROR_STATUS;
        }

        if ((ethInterface.index = lookupInterfaceIndex(ethInterface.socketFd, ifName)) < 0) {
            return LAST_ERROR_STATUS;
        }

        if (lookupInterfaceMac(ethInterface.socketFd, ifName, ethInterface.deviceMac) != 0) {
            return LAST_ERROR_STATUS;
        }

        ethInterfaces.push_back(std::move(ethInterface));
    }

    return Status::success();
}

Status FrameSender::sendNotificationFrame(
    Config* config, 
    const EthInterface& ethInterface)
{
    // build Ethernet frame
    const unsigned char dst_mac[6] {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    const uint16_t ethertype {0x88B5};

    unsigned char frame[FRAME_LEN];

    std::memcpy(frame + 0, dst_mac, 6);
    std::memcpy(frame + 6, ethInterface.deviceMac.data(), 6);
    uint16_t ethertypeNetwork {htons(ethertype)};
    std::memcpy(frame + 12, &ethertypeNetwork, 2);

    std::memcpy(frame + 14, config->getServiceId().c_str(), PAYLOAD_LEN);

    // prepare for sending
    sockaddr_ll sll;
    std::memset(&sll, 0, sizeof(sll));
    sll.sll_family = AF_PACKET;
    sll.sll_ifindex = ethInterface.index;
    sll.sll_halen = ETH_ALEN;
    std::memcpy(sll.sll_addr, dst_mac, 6);

    // optionally set socket to allow broadcast
    int on {1};
    if (setsockopt(ethInterface.socketFd, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on)) < 0) {
        std::cout << "failed to set socket to broadcast";
    }

    // send frame
    ssize_t sent {sendto(ethInterface.socketFd, frame, FRAME_LEN, 0, (sockaddr*)&sll, sizeof(sll))};
    if (sent < 0 || sent < FRAME_LEN) {
        return LAST_ERROR_STATUS;
    }

    return Status::success();
}

} // namespace discoveryservice::daemon::io