#pragma once

#include "Config.h"

namespace discoveryservice::daemon
{

class DefaultConfig : public Config
{
public:
    // Override Config
    const std::string& getServiceId() const override;
    size_t getNotificationPeriodUsec() const override;
    size_t getConnectionExpirePeriodSec() const override;
    int ipcPort() const override;

private:
    /**
     * Daemon UUID to authenticate frames sent by this daemon
     */
    std::string m_serviceId {"58a9a24d970f446b8d589711eea1fef0"};

    size_t m_notificationPeriodUsec {1000000};  // 1 sec

    size_t m_connectionExpirePeriodSec {30};

    int m_ipcPort {8090};
};

}; // namespace discoveryservice::daemon