
#include <iostream>
#include <memory>


#include "Session.h"

using discoveryservice::daemon::Session;

int main()
{
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

    std::unique_ptr<Session> session {new Session()};
    return session->run();
}