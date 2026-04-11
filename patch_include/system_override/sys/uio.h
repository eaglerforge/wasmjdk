#ifndef LWJGL_EMCC_PATCH_UIO
#define LWJGL_EMCC_PATCH_UIO

#include_next <sys/uio.h> // Include the actual system header first
#include <errno.h>
#include <sys/types.h>

#if _LWJGL_GLX_==1

#else
static inline ssize_t process_vm_writev(pid_t pid, 
                                        const struct iovec *local_iov,
                                        unsigned long liovcnt,
                                        const struct iovec *remote_uio,
                                        unsigned long riovcnt,
                                        unsigned long flags) {
    errno = ENOSYS;
    return -1;
}
#endif


#endif