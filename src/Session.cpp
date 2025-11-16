#include "Session.h"

#include <iostream>
#include <string>
#include <unistd.h>
#include <algorithm>
#include <chrono>

#include "Status.h"
#include "Config.h"
#include "DefaultConfig.h"
#include "EthernetFrameSender.h"
#include "EthernetFrameReceiver.h"
#include "EthernetUtils.h"

using discoveryservice::daemon::io::Status;
using discoveryservice::daemon::io::EthInterface;
using discoveryservice::daemon::io::FrameSender;
using discoveryservice::daemon::io::FrameReceiver;

/**
 * estimated out of condition for 30 second refresh rate and 10,000 active connection handling 
 * as 30/10000 minus some margin to account cpu time spent to do other actions
 */
#define POLL_DELAY_USEC 2000

namespace discoveryservice::daemon
{

Session::Session()
    : m_config {new DefaultConfig()}
{
}

Session::~Session()
{
    delete m_config;
    m_config = nullptr;

    delete m_frameSender;
    m_frameSender = nullptr;

    delete m_frameReceiver;
    m_frameReceiver = nullptr;

    io::closeSockets(m_outputSockets);
}

int Session::run()
{
    if (!init()) {
        return 1;
    }

    const size_t notificationPeriod {m_config->getNotificationPeriodUsec()};
    size_t notificationElapsedTime {};

    while (true) {
        std::vector<EthernetConnection> newConnections {m_frameReceiver->pollInputFrames(m_config)};
        updateConnections(newConnections);
        removeExpiredConnections();

        usleep(POLL_DELAY_USEC);

        notificationElapsedTime += POLL_DELAY_USEC;
        if (notificationElapsedTime >= notificationPeriod) {
            emitNotifications();
            notificationElapsedTime = 0;
        }
    }

    return 0;
}

bool Session::init()
{
    m_ethInterfaceNames = io::getEthernetInterfaceNames();
    if (m_ethInterfaceNames.empty()) {
        std::cout << "No Ethernet interfaces found. Exiting...";
        return false;
    }

    m_frameSender = new FrameSender{};
    Status status {m_frameSender->initOutputSockets(m_ethInterfaceNames, m_outputSockets)};
    if (!status.isSuccess()) {
        std::cout << status.getMessage() << std::endl;
        return false;
    }

    m_frameReceiver = new FrameReceiver{};
    status = m_frameReceiver->initInputSockets(m_ethInterfaceNames, m_inputSockets);
    if (!status.isSuccess()) {
        std::cout << status.getMessage() << std::endl;
        return false;
    }

    return true;
}

void Session::updateConnections(const std::vector<EthernetConnection>& newConnections)
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
                connections.push_back(std::move(newConnection));
            }
        }
    }
}

void Session::removeExpiredConnections()
{
    const size_t connectionExpirePeriodMsec {m_config->getConnectionExpirePeriodSec() * 1000};
    int64_t nowMsec = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch())
        .count();

    for (auto& [ifName, connections] : m_connections) {
        std::remove_if(connections.begin(), connections.end(), 
            [connectionExpirePeriodMsec, nowMsec] (const EthernetConnection& ec) { 
                return (nowMsec - ec.lastTimeSeenActiveMsec) > connectionExpirePeriodMsec; 
            }
        );
    }
}

void Session::emitNotifications()
{
    for (const auto& ethInterface : m_outputSockets) {
        m_frameSender->sendNotificationFrame(m_config, ethInterface);
    }
}

std::string Session::getInterfaceName(int socketFd) const
{
    auto ptr {std::find_if(m_inputSockets.begin(), m_inputSockets.end(), 
        [socketFd] (const EthInterface& eth) { return eth.socketFd == socketFd; })};
    return ptr != m_inputSockets.end() ? ptr->name : std::string{};
}

} // namespace discoveryservice::daemon