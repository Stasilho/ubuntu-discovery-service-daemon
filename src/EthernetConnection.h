#pragma once

#include <string>
#include <array>

namespace discoveryservice::daemon
{

struct EthernetConnection
{
    std::string interfaceName {};
    int socketFd {-1};
    std::array<unsigned char, 6> deviceMac {};
    int64_t lastTimeSeenActiveMsec {};
};


} // namespace discoveryservice::daemon