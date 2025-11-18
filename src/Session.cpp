#include "Session.h"

#include <iostream>
#include <unistd.h>

#include "Status.h"
#include "Config.h"
#include "DefaultConfig.h"
#include "ConnectionHandler.h"
#include "TcpRequestProcessor.h"
#include "TcpServer.h"

using discoveryservice::daemon::io::Status;
using discoveryservice::daemon::io::TcpServer;

/**
 * estimated out of condition for 30 second refresh rate and 10,000 active connection handling 
 * as 30/10000 minus some margin to account cpu time spent to do other actions
 */
#define POLL_DELAY_USEC 2000

namespace discoveryservice::daemon
{

Session::Session()
    : m_config {new DefaultConfig()}
    , m_connectionHandler {new ConnectionHandler{}}
{
}

Session::~Session()
{
    delete m_config;
    m_config = nullptr;

    delete m_connectionHandler;
    m_connectionHandler = nullptr;

    delete m_tcpServer;
    m_tcpServer = nullptr;

    delete m_tcpRequestProcessor;
    m_tcpRequestProcessor = nullptr;
}

int Session::run()
{
    if (!init()) {
        return 1;
    }

    const size_t notificationPeriod {m_config->getNotificationPeriodUsec()};
    size_t notificationElapsedTime {};

    while (true) {
        m_connectionHandler->updateState(m_config);
        m_tcpServer->handleClientConnection(*m_tcpRequestProcessor);

        usleep(POLL_DELAY_USEC);

        notificationElapsedTime += POLL_DELAY_USEC;
        if (notificationElapsedTime >= notificationPeriod) {
            m_connectionHandler->emitNotifications(m_config);
            notificationElapsedTime = 0;
        }
    }

    return 0;
}

bool Session::init()
{
    Status status {m_connectionHandler->init()};
    if (!status.isSuccess()) {
        std::cout << status.getMessage() << std::endl;
        return false;
    }
    
    m_tcpServer = new TcpServer{};
    status = m_tcpServer->initConnection(m_config);
    if (!status.isSuccess()) {
        std::cout << status.getMessage() << std::endl;
        return false;
    }

    m_tcpRequestProcessor = new TcpRequestProcessor{m_connectionHandler};

    return true;
}

} // namespace discoveryservice::daemon