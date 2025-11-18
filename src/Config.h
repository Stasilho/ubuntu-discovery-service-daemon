#pragma once

#include <string>

namespace discoveryservice::daemon
{

class Config
{
public:
    virtual ~Config() = default;

    virtual const std::string& getServiceId() const = 0;

    virtual size_t getNotificationPeriodUsec() const = 0;

    virtual size_t getConnectionExpirePeriodSec() const = 0;

    virtual int ipcPort() const = 0;
};

}; // namespace discoveryservice::daemon