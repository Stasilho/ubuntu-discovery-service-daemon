#include "EthernetUtils.h"

#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <cstring>

namespace discoveryservice::daemon::io
{

std::vector<std::string> getEthernetInterfaceNames()
{
    ifaddrs* ifs {};
    if (getifaddrs(&ifs) != 0) {
        perror("getifaddrs");
        return {};
    }

    std::vector<std::string> ifNames;

    for (auto* it {ifs}; it; it = it->ifa_next) {
        if (it->ifa_addr 
            && it->ifa_addr->sa_family == AF_PACKET
            && !(it->ifa_flags & IFF_LOOPBACK)) 
        {
            ifNames.push_back({it->ifa_name});
        }
    }

    freeifaddrs(ifs);

    return ifNames;
}

uint32_t getIpAddress(const std::string& ifName)
{
    int socketFd {socket(AF_INET, SOCK_DGRAM, IPPROTO_IP)};

    ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    strcpy(ifr.ifr_name, ifName.c_str());
    ioctl(socketFd, SIOCGIFADDR, &ifr);
    close(socketFd);

    char ip[INET_ADDRSTRLEN];
    //strcpy(ip, inet_ntoa(((sockaddr_in*) &ifr.ifr_addr)->sin_addr));
    return ntohl(((sockaddr_in*) &ifr.ifr_addr)->sin_addr.s_addr);
}

void closeSockets(std::vector<EthInterface>& ethInterfaces)
{
    for (const auto& ethInterface : ethInterfaces) {
        if (ethInterface.socketFd >= 0) {
            close(ethInterface.socketFd);
        }
    }
}

} // discoveryservice::daemon::io