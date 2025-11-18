#pragma once

#include <vector>

namespace discoveryservice::daemon
{

class ConnectionHandler;



class TcpRequestProcessor
{
public:
    TcpRequestProcessor(ConnectionHandler* connectionHandler);
    virtual ~TcpRequestProcessor() = default;

    void process(int code, std::vector<char>& bufferOut);

private:
    ConnectionHandler* m_connectionHandler;

    // private methods
    void serializeConnections(std::vector<char>& bufferOut);
};

} // namespace discoveryservice::daemon