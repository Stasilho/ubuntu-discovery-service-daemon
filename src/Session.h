#pragma once

#include <vector>
#include <map>
#include <string>
#include <memory>

#include "EthInterface.h"
#include "EthernetConnection.h"

namespace discoveryservice::daemon
{

class Config;

namespace io
{
class FrameSender;
class FrameReceiver;
}



class Session
{
public:
    Session();
    virtual ~Session();

    int run();


private:
    Config* m_config {};

    discoveryservice::daemon::io::FrameSender* m_frameSender {};
    discoveryservice::daemon::io::FrameReceiver* m_frameReceiver {};

    std::vector<std::string> m_ethInterfaceNames;

    std::vector<discoveryservice::daemon::io::EthInterface> m_outputSockets;
    std::vector<discoveryservice::daemon::io::EthInterface> m_inputSockets;

    std::map<std::string, std::vector<EthernetConnection>> m_connections;

    // private methods
    bool init();

    void updateConnections(const std::vector<EthernetConnection>& newConnections);

    void removeExpiredConnections();

    void emitNotifications();

    std::string getInterfaceName(int socketFd) const;
};

} // namespace discoveryservice::daemon