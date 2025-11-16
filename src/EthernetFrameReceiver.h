#pragma once

#include <vector>
#include <sys/epoll.h>

#include "EthernetFrameIO.h"
#include "Status.h"
#include "EthInterface.h"
#include "EthernetConnection.h"

#define INPUT_FRAME_SIZE 65536

namespace discoveryservice::daemon
{
class Config;
}



namespace discoveryservice::daemon::io
{

class FrameReceiver : public EthernetFrameIO
{
public:
    virtual ~FrameReceiver();

    Status initInputSockets(
        const std::vector<std::string>& ifNames,
        std::vector<EthInterface>& ethInterfaces);

    std::vector<EthernetConnection> pollInputFrames(Config* config);

private:
    unsigned char m_frame [INPUT_FRAME_SIZE];

    int m_epollFd {-1};
    std::vector<epoll_event> m_epollEvents;
};

} // namespace discoveryservice::daemon::io