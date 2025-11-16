#pragma once

#include <string>

namespace discoveryservice::daemon::io
{

class EthernetFrameIO
{
public:
    int lookupInterfaceIndex(
        int socketFd,
        const std::string& ifName);

    int lookupInterfaceMac(
        int socketFd,
        const std::string& ifName,
        std::array<unsigned char, 6>& mac);
};

} // namespace discoveryservice::daemon::io