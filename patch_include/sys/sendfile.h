#ifndef SENDFILE_SHIM_H
#define SENDFILE_SHIM_H

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

static ssize_t sendfile(int out_fd, int in_fd, off_t *offset, size_t count) {
    size_t total_sent = 0;
    char buffer[4096];
    ssize_t bytes_read, bytes_written;

    if (offset != NULL) {
        if (lseek(in_fd, *offset, SEEK_SET) == (off_t)-1) {
            return -1;
        }
    }

    while (total_sent < count) {
        bytes_read = read(in_fd, buffer, sizeof(buffer));
        if (bytes_read < 0) {
            return -1;
        }
        if (bytes_read == 0) {
            break;
        }

        bytes_written = write(out_fd, buffer, bytes_read);
        if (bytes_written < 0) {
            return -1;
        }

        total_sent += bytes_written;

        if (offset != NULL) {
            *offset += bytes_written;
        }
    }

    return total_sent;
}

#endif