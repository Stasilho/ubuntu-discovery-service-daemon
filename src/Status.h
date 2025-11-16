#pragma once

#include <string>

namespace discoveryservice::daemon::io
{

class Status
{
public:
    Status() = default;
    Status(bool error, const std::string& msg);
    virtual ~Status() = default;

    Status(const Status& other) = default;
    Status& operator= (const Status& other) = default;
    Status(Status&& other) = default;
    Status& operator= (Status&& other) = default;

    bool isSuccess() const;
    std::string getMessage() const;

    static Status success();
    static Status error(const char* msg);

private:
    bool m_error{};
    std::string m_message {};
};

} // namespace discoveryservice::daemon::io