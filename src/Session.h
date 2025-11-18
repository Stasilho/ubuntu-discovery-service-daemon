#pragma once

namespace discoveryservice::daemon
{

class Config;
class ConnectionHandler;
class TcpRequestProcessor;

namespace io
{
class TcpServer;
}



class Session
{
public:
    Session();
    virtual ~Session();

    int run();

private:
    Config* m_config {};
    ConnectionHandler* m_connectionHandler {};
    discoveryservice::daemon::io::TcpServer* m_tcpServer {};
    TcpRequestProcessor* m_tcpRequestProcessor {};

    // private methods
    bool init();
};

} // namespace discoveryservice::daemon