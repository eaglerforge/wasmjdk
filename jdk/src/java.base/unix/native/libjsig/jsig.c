#include "jni.h"
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_SIGNALS NSIG

extern int __sigaction(int, const struct sigaction *, struct sigaction *);
extern void (*__libc_signal(int, void (*)(int)))(int);

static struct sigaction sact[MAX_SIGNALS];
static bool deprecated_usage[MAX_SIGNALS];
static sigset_t jvmsigs;

/* Thread-local guard to prevent infinite recursion */
static __thread bool jsig_active = false;

static pthread_mutex_t mutex;
static pthread_once_t mutex_init_once = PTHREAD_ONCE_INIT;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
static pthread_t jvm_tid;

static bool jvm_signal_installing = false;
static bool jvm_signal_installed = false;
static bool warning_printed = false;

/* Initialize mutex as RECURSIVE to prevent deadlocks on the same thread */
static void init_mutex() {
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&mutex, &attr);
    pthread_mutexattr_destroy(&attr);
}

static void signal_lock() {
    pthread_once(&mutex_init_once, init_mutex);
    pthread_mutex_lock(&mutex);
    
    if (jvm_signal_installing) {
        if (pthread_equal(jvm_tid, pthread_self()) == 0) {
            do {
                pthread_cond_wait(&cond, &mutex);
            } while (jvm_signal_installing);
        }
    }
}

static void signal_unlock() {
    pthread_mutex_unlock(&mutex);
}

static void print_deprecation_warning() {
    if (!warning_printed) {
        warning_printed = true;
        fprintf(stderr, "VM warning: signal-chaining for signal() is deprecated. Use sigaction().\n");
    }
}


JNIEXPORT void (*signal(int sig, void (*disp)(int)))(int) {
    if (sig < 0 || sig >= MAX_SIGNALS) {
        errno = EINVAL;
        return SIG_ERR;
    }

    if (jsig_active) {
        return __libc_signal(sig, disp);
    }

    void (*old_handler)(int);
    signal_lock();
    jsig_active = true;

    deprecated_usage[sig] = true;

    if (jvm_signal_installed && sigismember(&jvmsigs, sig)) {
        print_deprecation_warning();
        old_handler = sact[sig].sa_handler;
        sact[sig].sa_handler = disp;
        
        jsig_active = false;
        signal_unlock();
        return old_handler;
    } else {
        old_handler = __libc_signal(sig, disp);
        if (!jvm_signal_installed && jvm_signal_installing) {
            sact[sig].sa_handler = old_handler;
            sigaddset(&jvmsigs, sig);
        }
        
        jsig_active = false;
        signal_unlock();
        return old_handler;
    }
}

JNIEXPORT int sigaction(int sig, const struct sigaction *act, struct sigaction *oact) {
    if (sig < 0 || sig >= MAX_SIGNALS) {
        errno = EINVAL;
        return -1;
    }

    /* If recursing, call the internal musl sigaction immediately */
    if (jsig_active) {
        return __sigaction(sig, act, oact);
    }

    int res;
    struct sigaction oldAct;
    signal_lock();
    jsig_active = true;

    if (jvm_signal_installed && sigismember(&jvmsigs, sig)) {
        if (oact != NULL) *oact = sact[sig];
        if (act != NULL) sact[sig] = *act;
        res = 0;
    } else {
        res = __sigaction(sig, act, &oldAct);
        if (res == 0) {
            if (jvm_signal_installing && act != NULL) {
                sact[sig] = oldAct;
                sigaddset(&jvmsigs, sig);
            }
            if (oact != NULL) *oact = oldAct;
        }
    }

    jsig_active = false;
    signal_unlock();
    return res;
}


JNIEXPORT void JVM_begin_signal_setting() {
    signal_lock();
    sigemptyset(&jvmsigs);
    jvm_signal_installing = true;
    jvm_tid = pthread_self();
    signal_unlock();
}

JNIEXPORT void JVM_end_signal_setting() {
    signal_lock();
    jvm_signal_installed = true;
    jvm_signal_installing = false;
    pthread_cond_broadcast(&cond);
    signal_unlock();
}

JNIEXPORT struct sigaction *JVM_get_signal_action(int sig) {
    if (sigismember(&jvmsigs, sig)) {
        return &sact[sig];
    }
    return NULL;
}