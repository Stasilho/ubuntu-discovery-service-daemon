#include "DefaultConfig.h"

namespace discoveryservice::daemon
{

std::string DefaultConfig::getServiceId() const
{
    return m_serviceId;
}

}  // namespace discoveryservice::daemon