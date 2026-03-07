#ifndef EPOLL_SHIM_H
#define EPOLL_SHIM_H

#include <poll.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif

/* epoll operations */
#define EPOLL_CTL_ADD 1
#define EPOLL_CTL_DEL 2
#define EPOLL_CTL_MOD 3
#define EPOLL_CLOEXEC 02000000

/* Map epoll events to poll events */
#define EPOLLIN     POLLIN
#define EPOLLOUT    POLLOUT
#define EPOLLPRI    POLLPRI
#define EPOLLERR    POLLERR
#define EPOLLHUP    POLLHUP
#define EPOLLRDNORM POLLRDNORM
#define EPOLLRDBAND POLLRDBAND
#define EPOLLWRNORM POLLWRNORM
#define EPOLLWRBAND POLLWRBAND
#define EPOLLMSG    POLLMSG
/* Edge-triggered behavior is not supported by poll(); provided as a dummy */
#define EPOLLET     (1U << 31) 

typedef union epoll_data {
    void    *ptr;
    int      fd;
    uint32_t u32;
    uint64_t u64;
} epoll_data_t;

struct epoll_event {
    uint32_t     events;
    epoll_data_t data;
};

#ifdef EPOLL_SHIM_IMPLEMENTATION

typedef struct {
    int fd;
    struct epoll_event event;
} shim_epoll_entry_t;

typedef struct {
    int epfd_id;
    shim_epoll_entry_t *entries;
    int count;
    int capacity;
} shim_epoll_ctx_t;

#define MAX_EPOLL_INSTANCES 64
static shim_epoll_ctx_t g_epoll_contexts[MAX_EPOLL_INSTANCES] = {0};
static int g_epoll_counter = 10000;

static shim_epoll_ctx_t* get_epoll_ctx(int epfd) {
    for (int i = 0; i < MAX_EPOLL_INSTANCES; ++i) {
        if (g_epoll_contexts[i].epfd_id == epfd) {
            return &g_epoll_contexts[i];
        }
    }
    return NULL;
}

int epoll_create(int size) {
    if (size <= 0) {
        errno = EINVAL;
        return -1;
    }
    for (int i = 0; i < MAX_EPOLL_INSTANCES; ++i) {
        if (g_epoll_contexts[i].epfd_id == 0) {
            g_epoll_contexts[i].epfd_id = g_epoll_counter++;
            g_epoll_contexts[i].capacity = 16;
            g_epoll_contexts[i].count = 0;
            g_epoll_contexts[i].entries = (shim_epoll_entry_t*)malloc(sizeof(shim_epoll_entry_t) * 16);
            return g_epoll_contexts[i].epfd_id;
        }
    }
    errno = ENOMEM;
    return -1;
}

int epoll_create1(int flags) {
    return epoll_create(1);
}

int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event) {
    shim_epoll_ctx_t *ctx = get_epoll_ctx(epfd);
    if (!ctx) {
        errno = EBADF;
        return -1;
    }

    int idx = -1;
    for (int i = 0; i < ctx->count; ++i) {
        if (ctx->entries[i].fd == fd) {
            idx = i;
            break;
        }
    }

    if (op == EPOLL_CTL_ADD) {
        if (idx != -1) {
            errno = EEXIST;
            return -1;
        }
        if (ctx->count == ctx->capacity) {
            ctx->capacity *= 2;
            ctx->entries = (shim_epoll_entry_t*)realloc(ctx->entries, sizeof(shim_epoll_entry_t) * ctx->capacity);
        }
        ctx->entries[ctx->count].fd = fd;
        ctx->entries[ctx->count].event = *event;
        ctx->count++;
        return 0;
    } 
    else if (op == EPOLL_CTL_MOD) {
        if (idx == -1) {
            errno = ENOENT;
            return -1;
        }
        ctx->entries[idx].event = *event;
        return 0;
    } 
    else if (op == EPOLL_CTL_DEL) {
        if (idx == -1) {
            errno = ENOENT;
            return -1;
        }
        ctx->entries[idx] = ctx->entries[ctx->count - 1];
        ctx->count--;
        return 0;
    }

    errno = EINVAL;
    return -1;
}

int epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout) {
    shim_epoll_ctx_t *ctx = get_epoll_ctx(epfd);
    if (!ctx || maxevents <= 0) {
        errno = EINVAL;
        return -1;
    }

    if (ctx->count == 0) {
        if (timeout > 0) usleep(timeout * 1000);
        return 0;
    }

    struct pollfd *pfds = (struct pollfd*)malloc(sizeof(struct pollfd) * ctx->count);
    for (int i = 0; i < ctx->count; ++i) {
        pfds[i].fd = ctx->entries[i].fd;
        pfds[i].events = ctx->entries[i].event.events;
        pfds[i].revents = 0;
    }

    int n = poll(pfds, ctx->count, timeout);
    if (n > 0) {
        int ev_idx = 0;
        for (int i = 0; i < ctx->count && ev_idx < maxevents; ++i) {
            if (pfds[i].revents) {
                events[ev_idx].events = pfds[i].revents;
                events[ev_idx].data = ctx->entries[i].event.data;
                ev_idx++;
            }
        }
        n = ev_idx;
    }

    free(pfds);
    return n;
}

#endif // EPOLL_SHIM_IMPLEMENTATION

#ifdef __cplusplus
}
#endif

#endif // EPOLL_SHIM_H