#include "ConnectionHandler.h"

#include <iostream>
#include <algorithm>
#include <chrono>

#include "Config.h"
#include "EthernetUtils.h"
#include "EthernetFrameSender.h"
#include "EthernetFrameReceiver.h"

using discoveryservice::daemon::io::Status;
using discoveryservice::daemon::io::EthInterface;
using discoveryservice::daemon::io::FrameSender;
using discoveryservice::daemon::io::FrameReceiver;
using discoveryservice::daemon::EthernetConnection;

#define LAST_ERROR_STATUS Status::error(std::strerror(errno))

namespace discoveryservice::daemon
{

ConnectionHandler::~ConnectionHandler()
{
    delete m_frameSender;
    m_frameSender = nullptr;

    delete m_frameReceiver;
    m_frameReceiver = nullptr;

    io::closeSockets(m_outputSockets);
}

const std::map<std::string, std::vector<EthernetConnection>>& ConnectionHandler::getConnections() const
{
    return m_connections;
}

Status ConnectionHandler::init()
{
    m_ethInterfaceNames = io::getEthernetInterfaceNames();
    if (m_ethInterfaceNames.empty()) {
        return Status::error("No Ethernet interfaces found. Exiting...");
    }

    for (const auto& ifName : m_ethInterfaceNames) {
        m_interfaceIpAddresses[ifName] = io::getIpAddress(ifName);
    }

    m_frameSender = new FrameSender{};
    Status status {m_frameSender->initOutputSockets(m_ethInterfaceNames, m_outputSockets)};
    if (!status.isSuccess()) {
        return status;
    }

    m_frameReceiver = new FrameReceiver{};
    status = m_frameReceiver->initInputSockets(m_ethInterfaceNames, m_inputSockets);
    if (!status.isSuccess()) {
        return status;
    }

    return Status::success();
}

void ConnectionHandler::updateState(Config* config)
{
    std::vector<EthernetConnection> newConnections {m_frameReceiver->pollInputFrames(config)};
    updateConnections(newConnections);
    removeExpiredConnections(config);
}

void ConnectionHandler::updateConnections(
    const std::vector<EthernetConnection>& newConnections)
{
    for (const auto& ethConnection : newConnections) {
        if (std::string ifName {getInterfaceName(ethConnection.socketFd)}; !ifName.empty()) {
            if (!m_connections.contains(ifName)) {
                m_connections[ifName] = {};
            }

            auto& connections {m_connections.at(ifName)};
            auto connectionPtr {std::find_if(connections.begin(), connections.end(), 
                [&ethConnection] (const EthernetConnection& ec) { 
                    return ec.deviceMac == ethConnection.deviceMac;
                }
            )};

            if (connectionPtr != connections.end()) {
                connectionPtr->lastTimeSeenActiveMsec = ethConnection.lastTimeSeenActiveMsec;
            }
            else {
                EthernetConnection newConnection {ethConnection};
                newConnection.interfaceName = ifName;
                newConnection.ipAddress = m_interfaceIpAddresses.at(ifName);
                connections.push_back(std::move(newConnection));
            }
        }
    }
}

void ConnectionHandler::removeExpiredConnections(Config* config)
{
    const size_t connectionExpirePeriodMsec {config->getConnectionExpirePeriodSec() * 1000};
    int64_t nowMsec = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch())
        .count();

    for (auto& [ifName, connections] : m_connections) {
        connections.erase(std::remove_if(connections.begin(), connections.end(), 
            [connectionExpirePeriodMsec, nowMsec] (const EthernetConnection& ec) { 
                return (nowMsec - ec.lastTimeSeenActiveMsec) > connectionExpirePeriodMsec; 
            }
        ), connections.end());
    }
}

void ConnectionHandler::emitNotifications(Config* config)
{
    for (const auto& ethInterface : m_outputSockets) {
        m_frameSender->sendNotificationFrame(config, ethInterface);
    }
}

std::string ConnectionHandler::getInterfaceName(int socketFd) const
{
    auto ptr {std::find_if(m_inputSockets.begin(), m_inputSockets.end(), 
        [socketFd] (const EthInterface& eth) { return eth.socketFd == socketFd; })};
    return ptr != m_inputSockets.end() ? ptr->name : std::string{};
}

void ConnectionHandler::printMacAddress(const std::array<unsigned char, 6>& mac) const
{
    std::printf("%.02x:%.02x:%.02x:%.02x:%.02x:%.02x\n", 
        mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

} // namespace discoveryservice::daemon