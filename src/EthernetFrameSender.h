#pragma once

#include <vector>
#include <array>
#include <map>
#include <string>

#include "Status.h"
#include "EthInterface.h"

namespace discoveryservice::daemon
{
class Config;
}

namespace discoveryservice::daemon::io
{

class FrameSender
{
public:
    virtual ~FrameSender() = default;

    Status initSockets(
        const std::vector<std::string>& ifNames,
        std::vector<EthInterface>& ethInterfaces);

    /**
     * Notify the others on the network about self presence.
     */
    Status sendNotificationFrame(
        Config* config, 
        const EthInterface& ethInterface);

private:
    std::map<std::string, EthInterface> m_ethInterfaces;
};

} // namespace discoveryservice::daemon::io