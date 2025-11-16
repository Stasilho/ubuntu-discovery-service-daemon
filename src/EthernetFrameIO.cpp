#include "EthernetFrameIO.h"

#include <cstring>
#include <array>
#include <net/if.h>
#include <sys/ioctl.h>

namespace
{

void prepareRequest(ifreq& ifr, const char* ifName)
{
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, ifName, IFNAMSIZ - 1);
}

} // epmty namespace



namespace discoveryservice::daemon::io
{

int EthernetFrameIO::lookupInterfaceIndex(
    int socketFd,
    const std::string& ifName)
{
    ifreq ifr;
    prepareRequest(ifr, ifName.c_str());

    if (ioctl(socketFd, SIOCGIFINDEX, &ifr) < 0) {
        return -1;
    }

    return ifr.ifr_ifindex;
}

int EthernetFrameIO::lookupInterfaceMac(
    int socketFd, 
    const std::string& ifName,
    std::array<unsigned char, 6>& mac)
{
    ifreq ifr;
    prepareRequest(ifr, ifName.c_str());

    if (int result = ioctl(socketFd, SIOCGIFHWADDR, &ifr); result < 0) {
        return result;
    }
    std::memcpy(mac.data(), ifr.ifr_hwaddr.sa_data, 6);

    return 0;
}

} // namespace discoveryservice::daemon::io