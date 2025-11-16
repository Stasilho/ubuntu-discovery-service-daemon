#pragma once

#include "Config.h"

namespace discoveryservice::daemon
{

class DefaultConfig : public Config
{
public:
    // Override Config
    std::string getServiceId() const override;

private:
    /**
     * Daemon UUID to authenticate frames sent by this daemon
     */
    std::string m_serviceId {"58a9a24d970f446b8d589711eea1fef0"};
};

}; // namespace discoveryservice::daemon