#include "Status.h"

namespace discoveryservice::daemon::io
{

//

Status::Status(bool error, const std::string& msg)
    : m_error {error}
    , m_message {msg}
{
}

bool Status::isSuccess() const
{
    return !m_error;
}

std::string Status::getMessage() const
{
    return m_message;
}

Status Status::success()
{
    return Status(false, {});
}

Status Status::error(const char *msg)
{
    return Status(true, {msg});
}

} // namespace discoveryservice::daemon::io