#pragma once

#include <string>

namespace discoveryservice::daemon
{

class Config
{
public:
    virtual ~Config() = default;

    virtual std::string getServiceId() const = 0;
};

}; // namespace discoveryservice::daemon