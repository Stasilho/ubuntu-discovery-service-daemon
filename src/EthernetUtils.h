#pragma once

#include <vector>
#include <string>
#include <cstdint>

#include "EthInterface.h"

namespace discoveryservice::daemon::io
{

std::vector<std::string> getEthernetInterfaceNames();

uint32_t getIpAddress(const std::string& ifName);

void closeSockets(std::vector<EthInterface>& ethInterfaces);

} // namespace discoveryservice::daemon::io