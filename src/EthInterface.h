#pragma once

#include <string>
#include <array>

namespace discoveryservice::daemon::io
{

struct EthInterface
{
    std::string name {};
    int socketFd {-1};
    int index {-1};
    std::array<unsigned char, 6> deviceMac;
};

} // namespace discoveryservice::daemon::io