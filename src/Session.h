#pragma once

#include <vector>

namespace discoveryservice::daemon
{

class Config;



class Session
{
public:
    Session();
    virtual ~Session();

    int run();

private:
    Config* m_config {};

    std::vector<discoveryservice::daemon::io::EthInterface> m_outputSockets;
};

} // namespace discoveryservice::daemon