#include "DefaultConfig.h"

namespace discoveryservice::daemon
{

const std::string& DefaultConfig::getServiceId() const
{
    return m_serviceId;
}

size_t DefaultConfig::getNotificationPeriodUsec() const
{
    return m_notificationPeriodUsec;
}

size_t DefaultConfig::getConnectionExpirePeriodSec() const
{
    return m_connectionExpirePeriodSec;
}

}  // namespace discoveryservice::daemon