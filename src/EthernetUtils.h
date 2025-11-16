#pragma once

#include <vector>
#include <string>

#include "EthInterface.h"

namespace discoveryservice::daemon::io
{

std::vector<std::string> getEthernetInterfaceNames();

void closeSockets(std::vector<EthInterface>& ethInterfaces);

} // namespace discoveryservice::daemon::io