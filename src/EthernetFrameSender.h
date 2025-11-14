#ifndef ETHERNET_FRAME_SENDER_H
#define ETHERNET_FRAME_SENDER_H

namespace discoveryservice::daemon::io
{

class FrameSender
{
public:
    virtual ~FrameSender();

    int sendFrame();

private:
    int m_socket {-1};
    unsigned char* m_frame {};
};

} // namespace discoveryservice::daemon::io

#endif // ETHERNET_FRAME_SENDER_H