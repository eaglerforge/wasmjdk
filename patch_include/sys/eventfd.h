#ifndef EVENTFD_SHIM_H
#define EVENTFD_SHIM_H

#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <errno.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/* eventfd flags */
#define EFD_SEMAPHORE (1 << 0)
#define EFD_CLOEXEC   O_CLOEXEC
#define EFD_NONBLOCK  O_NONBLOCK

typedef uint64_t eventfd_t;

#include <sys/ioctl.h>

static int eventfd(unsigned int initval, int flags) {
    int pipefds[2];
    
    if (pipe(pipefds) != 0) {
        return -1;
    }

    int read_fd = pipefds[0];
    int write_fd = pipefds[1];

    if (flags & EFD_NONBLOCK) {
        fcntl(read_fd, F_SETFL, O_NONBLOCK);
        fcntl(write_fd, F_SETFL, O_NONBLOCK);
    }

    if (initval > 0) {
        uint64_t val = initval;
        write(write_fd, &val, sizeof(val));
    }
    return read_fd; 
}

#ifdef __cplusplus
}
#endif

#endif // EVENTFD_SHIM_H