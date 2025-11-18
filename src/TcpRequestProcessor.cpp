#include "TcpRequestProcessor.h"

#include <iostream>
#include <cstring>
#include <cstdint>

#include "ConnectionHandler.h"

namespace discoveryservice::daemon
{

TcpRequestProcessor::TcpRequestProcessor(ConnectionHandler* connectionHandler)
    : m_connectionHandler {connectionHandler}
{
}

void TcpRequestProcessor::process(int code, std::vector<char>& bufferOut)
{
    switch (code)
    {
    case 1:
        serializeConnections(bufferOut);
        break;
    default:
        std::cout << "Unknown tcp request code" << std::endl;
        break;
    }
}

/**
 * memory layout:
 * | int num interfaces | for each ifname: char[16] ifname |.
 * for each interface name: | int num connections per interface | 
 *                          |for each connection: char[6] device mac, uint32_t IP |
 */
void TcpRequestProcessor::serializeConnections(std::vector<char>& bufferOut)
{
    bufferOut.clear();

    const auto& connectionMap {m_connectionHandler->getConnections()};

    std::vector<std::string> ifNames;
    for (auto& [key, value] : connectionMap) {
        ifNames.push_back(key);
    }

    // put num interfaces
    char intBytes[4];
    const int numInterfaces {static_cast<int>(ifNames.size())};
    std::memcpy(intBytes, &numInterfaces, sizeof(int));
    bufferOut.insert(bufferOut.end(), intBytes, intBytes + 4);

    // put interface names
    for (size_t i {}; i < numInterfaces; ++i) {
        char ifNameBuffer[16] {};
        strncpy(ifNameBuffer, ifNames[i].c_str(), 15);
        bufferOut.insert(bufferOut.end(), ifNameBuffer, ifNameBuffer + 16);
    }

    // put all connections
    for (const auto& ifName : ifNames) {
        const auto& connections {connectionMap.at(ifName)};

        // put num connections
        const int numConnections {static_cast<int>(connections.size())};
        std::memcpy(intBytes, &numConnections, sizeof(int));
        bufferOut.insert(bufferOut.end(), intBytes, intBytes + 4);

        // put connections for the given interface name
        for (const auto& connection : connections) {
            const char* mac {reinterpret_cast<const char*>(connection.deviceMac.data())};
            bufferOut.insert(bufferOut.end(), mac, mac + 6);

            std::memcpy(intBytes, &connection.ipAddress, sizeof(uint32_t));
            bufferOut.insert(bufferOut.end(), intBytes, intBytes + 4);
        }
    }
}

} // namespace discoveryservice::daemon