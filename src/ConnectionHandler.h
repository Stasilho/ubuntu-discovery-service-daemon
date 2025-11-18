#pragma once

#include <map>
#include <vector>
#include <string>

#include "Status.h"
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



class ConnectionHandler
{
public:
    virtual ~ConnectionHandler();

    const std::map<std::string, std::vector<EthernetConnection>>& getConnections() const;

    discoveryservice::daemon::io::Status init();

    void updateState(Config* config);

    void updateConnections(const std::vector<EthernetConnection>& newConnections);

    void removeExpiredConnections(Config* config);

    void emitNotifications(Config* config);

private:
    discoveryservice::daemon::io::FrameSender* m_frameSender {};
    discoveryservice::daemon::io::FrameReceiver* m_frameReceiver {};

    std::vector<discoveryservice::daemon::io::EthInterface> m_outputSockets;
    std::vector<discoveryservice::daemon::io::EthInterface> m_inputSockets;

    std::vector<std::string> m_ethInterfaceNames;
    std::map<std::string, uint32_t> m_interfaceIpAddresses;
    std::map<std::string, std::vector<EthernetConnection>> m_connections;

    // private methods
    std::string getInterfaceName(int socketFd) const;

    void printMacAddress(const std::array<unsigned char, 6>& mac) const; // for testing
};

} // namespace discoveryservice::daemon