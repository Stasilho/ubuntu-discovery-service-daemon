#include "Session.h"

#include <iostream>
#include <string>
#include <unistd.h>

#include "Status.h"
#include "Config.h"
#include "DefaultConfig.h"
#include "EthernetFrameSender.h"
#include "EthernetFrameReceiver.h"
#include "EthernetUtils.h"

using discoveryservice::daemon::io::Status;
using discoveryservice::daemon::io::FrameSender;
using discoveryservice::daemon::io::FrameReceiver;

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

    io::closeSockets(m_outputSockets);
}

int Session::run()
{
    std::vector<std::string> ethInterfaceNames {io::getEthernetInterfaceNames()};
    if (ethInterfaceNames.empty()) {
        std::cout << "No Ethernet interfaces found. Exiting...";
        return 1;
    }

    FrameSender frameSender;
    Status status {frameSender.initSockets(ethInterfaceNames, m_outputSockets)};
    if (!status.isSuccess()) {
        std::cout << status.getMessage() << std::endl;
        return 1;
    }

    status = frameSender.sendNotificationFrame(m_config, m_outputSockets[0]);
    if (!status.isSuccess()) {
        std::cout << status.getMessage() << std::endl;
        return 1;
    }

    FrameReceiver frameReceiver;
    int result {frameReceiver.initSocket()};
    if (result != 0) {
        return result;
    }

    size_t iter {};
    size_t maxIter {600};
    while (iter++ < maxIter) {
        frameReceiver.pollFrame();
        sleep(100);
    }



    return 0;
}

} // namespace discoveryservice::daemon