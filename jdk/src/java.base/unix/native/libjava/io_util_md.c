/*
 * Copyright (c) 2001, 2024, Oracle and/or its affiliates. All rights reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.  Oracle designates this
 * particular file as subject to the "Classpath" exception as provided
 * by Oracle in the LICENSE file that accompanied this code.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *
 * You should have received a copy of the GNU General Public License version
 * 2 along with this work; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Please contact Oracle, 500 Oracle Parkway, Redwood Shores, CA 94065 USA
 * or visit www.oracle.com if you need additional information or have any
 * questions.
 */
#include "jni.h"
#include "jni_util.h"
#include "jvm.h"
#include "io_util.h"
#include "io_util_md.h"
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <shim/fakestdin.h>
#include <emscripten/atomic.h>

#if defined(__linux__) || defined(_ALLBSD_SOURCE) || defined(_AIX) || defined(__EMSCRIPTEN__)
#include <sys/ioctl.h>
#endif

#if defined(__linux__)
#include <linux/fs.h>
#endif

#if defined(__linux__) || defined(__EMSCRIPTEN__)
#include <sys/stat.h>
#endif

#ifdef MACOSX
#include <CoreFoundation/CoreFoundation.h>

__private_extern__
jstring newStringPlatform(JNIEnv *env, const char* str)
{
    jstring rv = NULL;
    CFMutableStringRef csref = CFStringCreateMutable(NULL, 0);
    if (csref == NULL) {
        JNU_ThrowOutOfMemoryError(env, "native heap");
    } else {
        CFStringAppendCString(csref, str, kCFStringEncodingUTF8);
        CFStringNormalize(csref, kCFStringNormalizationFormC);
        int clen = CFStringGetLength(csref);
        int ulen = (clen + 1) * 2; // utf16, zero padding
        char* chars = malloc(ulen);
        if (chars == NULL) {
            CFRelease(csref);
            JNU_ThrowOutOfMemoryError(env, "native heap");
        } else {
            if (CFStringGetCString(csref, chars, ulen, kCFStringEncodingUTF16)) {
                rv = (*env)->NewString(env, (jchar*)chars, clen);
            }
            free(chars);
            CFRelease(csref);
        }
    }
    return rv;
}
#endif

FD
handleOpen(const char *path, int oflag, int mode) {
    FD fd;
    RESTARTABLE(open(path, oflag, mode), fd);
    if (fd != -1) {
        struct stat buf;
        int result;
        RESTARTABLE(fstat(fd, &buf), result);
        if (result != -1) {
            if (S_ISDIR(buf.st_mode)) {
                close(fd);
                errno = EISDIR;
                fd = -1;
            }
        } else {
            close(fd);
            fd = -1;
        }
    }
    return fd;
}

FD getFD(JNIEnv *env, jobject obj, jfieldID fid) {
  jobject fdo = (*env)->GetObjectField(env, obj, fid);
  if (fdo == NULL) {
    return -1;
  }
  return (*env)->GetIntField(env, fdo, IO_fd_fdID);
}

void
fileOpen(JNIEnv *env, jobject this, jstring path, jfieldID fid, int flags)
{
    WITH_PLATFORM_STRING(env, path, ps) {
        FD fd;

#if defined(__linux__) || defined(_ALLBSD_SOURCE) || defined(__EMSCRIPTEN__)
        char *p = (char *)ps + strlen(ps) - 1;
        while ((p > ps) && (*p == '/'))
            *p-- = '\0';
#endif
        fd = handleOpen(ps, flags, 0666);
        //printf("fileOpen() in io_util_md.c, int fd = %d, path=%s\n", fd, ps);
        if (fd != -1) {
            jobject fdobj;
            jboolean append;
            fdobj = (*env)->GetObjectField(env, this, fid);
            if (fdobj != NULL) {
                (*env)->SetIntField(env, fdobj, IO_fd_fdID, fd);
                append = (flags & O_APPEND) == 0 ? JNI_FALSE : JNI_TRUE;
                (*env)->SetBooleanField(env, fdobj, IO_append_fdID, append);
            }
        } else {
            throwFileNotFoundException(env, path);
        }
    } END_PLATFORM_STRING(env, ps);
}

void
fileDescriptorClose(JNIEnv *env, jobject this)
{
    FD fd = (*env)->GetIntField(env, this, IO_fd_fdID);
    if ((*env)->ExceptionCheck(env)) {
        return;
    }

    if (fd == -1) {
        return;
    }
    (*env)->SetIntField(env, this, IO_fd_fdID, -1);
    if ((*env)->ExceptionCheck(env)) {
        return;
    }
    if (fd >= STDIN_FILENO && fd <= STDERR_FILENO) {
        int devnull = open("/dev/null", O_WRONLY);
        if (devnull < 0) {
            (*env)->SetIntField(env, this, IO_fd_fdID, fd);
            JNU_ThrowIOExceptionWithLastError(env, "open /dev/null failed");
        } else {
            dup2(devnull, fd);
            close(devnull);
        }
    } else {
        int result;
#if defined(_AIX)
        RESTARTABLE(close(fd), result);
#else
        result = close(fd);
#endif
        if (result == -1 && errno != EINTR) {
            JNU_ThrowIOExceptionWithLastError(env, "close failed");
        }
    }
}

ssize_t
handleRead(FD fd, void *buf, jint len)
{
    //EMSTDINPATCH
    if ((int)fd == 0) {
        //printf("fs read op in io_util_md.c (stdin patch!!!), fd=%d len=%s\n", fd, (int)len);
        return (ssize_t)fake_stdin_read(buf, (int)len);
    }
    
    ssize_t result;
    RESTARTABLE(read(fd, buf, len), result);
    return result;
}

ssize_t
handleWrite(FD fd, const void *buf, jint len)
{
    //EMSTDINPATCH
    if ((int)fd == 0) {
        //printf("fs write op in io_util_md.c (stdin no supported!), fd=%d\n", fd);
        return -1;
    }

    ssize_t result;
    RESTARTABLE(write(fd, buf, len), result);
    return result;
}

jint
handleAvailable(FD fd, jlong *pbytes)
{
    //EMSTDINPATCH
    if ((int)fd == 0) {
        //printf("fs available op in io_util_md.c (stdin patch!!!), fd=%d\n", fd);
        *pbytes = fake_stdin_available();
        return 1;
    }

    int mode;
    struct stat buf;
    jlong size = -1, current = -1;

    int result;
    RESTARTABLE(fstat(fd, &buf), result);
    if (result != -1) {
        mode = buf.st_mode;
        if (S_ISCHR(mode) || S_ISFIFO(mode) || S_ISSOCK(mode)) {
            int n;
            int result;
            RESTARTABLE(ioctl(fd, FIONREAD, &n), result);
            if (result >= 0) {
                *pbytes = n;
                return 1;
            }
        } else if (S_ISREG(mode)) {
            size = buf.st_size;
        }
    }

    if ((current = lseek(fd, 0, SEEK_CUR)) == -1) {
        return 0;
    }

    if (size < current) {
        if ((size = lseek(fd, 0, SEEK_END)) == -1)
            return 0;
        else if (lseek(fd, current, SEEK_SET) == -1)
            return 0;
    }

    *pbytes = size - current;
    return 1;
}

jint
handleSetLength(FD fd, jlong length)
{
    //EMSTDINPATCH
    if ((int)fd == 0) {
        //printf("fs set length op in io_util_md.c (unsupported), fd=%d, ret=%s\n", fd);
        return (jint)(-1);
    }

    int result;
    RESTARTABLE(ftruncate(fd, length), result);
    return result;
}

jlong
handleGetLength(FD fd)
{
    //EMSTDINPATCH
    if ((int)fd == 0) {
        //printf("fs get length op in io_util_md.c (stdin patch!!!), fd=%d\n", fd);
        return (jlong)fake_stdin_available();
    }

    struct stat sb;
    int result;
    RESTARTABLE(fstat(fd, &sb), result);
    if (result < 0) {
        return -1;
    }
#if defined(__linux__) && defined(BLKGETSIZE64)
    if (S_ISBLK(sb.st_mode)) {
        uint64_t size;
        if(ioctl(fd, BLKGETSIZE64, &size) < 0) {
            return -1;
        }
        return (jlong)size;
    }
#endif
    return sb.st_size;
}

jboolean
handleIsRegularFile(JNIEnv* env, FD fd)
{
    //EMSTDINPATCH
    if ((int)fd == 0) {
        //printf("shimming stdin [handleIsRegularFile] fd=%d\n", fd);
        return JNI_FALSE;
    }
    
    struct stat fbuf;
    if (fstat(fd, &fbuf) == -1)
        JNU_ThrowIOExceptionWithLastError(env, "fstat failed");

    return S_ISREG(fbuf.st_mode) ? JNI_TRUE : JNI_FALSE;
}
