#include "EthernetUtils.h"

#include <unistd.h>
#include <net/if.h>
#include <ifaddrs.h>

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

void closeSockets(std::vector<EthInterface>& ethInterfaces)
{
    for (const auto& ethInterface : ethInterfaces) {
        if (ethInterface.socketFd >= 0) {
            close(ethInterface.socketFd);
        }
    }
}

} // discoveryservice::daemon::io