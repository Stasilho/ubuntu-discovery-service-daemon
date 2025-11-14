#include <iostream>
#include <sys/types.h>
#include <unistd.h>

#include "EthernetFrameSender.h"
#include "EthernetFrameReceiver.h"

using discoveryservice::daemon::io::FrameSender;
using discoveryservice::daemon::io::FrameReceiver;

int main()
{
    FrameReceiver frameReceiver;
    frameReceiver.initSocket();

    FrameSender frameSender;
    int status {frameSender.sendFrame()};

    /*
    pid_t childId {fork()};

    if (childId < 0) {
        exit(EXIT_FAILURE);
    }

    if (childId > 0) {
        exit(EXIT_SUCCESS);
    }

    setsid();

    childId = fork();
    if (childId < 0) {
        exit(EXIT_FAILURE);
    }

    if (childId > 0) {
        exit(EXIT_SUCCESS);
    }

    // now we are in the daemon process
    pid_t pid {getpid()};
    */

    size_t iter {0};
    size_t maxIter {5};

    while (iter++ < maxIter) {
        frameReceiver.pollFrame();
        
        sleep(1);
        std::cout << "iter: " << iter << std::endl;
    }

    return 0;
}