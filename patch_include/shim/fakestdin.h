#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include <emscripten.h>
#include <emscripten/atomic.h>
#include <emscripten/threading.h>

#define FAKE_STDIN_CAPACITY 2048

#define DATA_CAP (FAKE_STDIN_CAPACITY - 12)

static uint8_t* FAKE_STDIN_BUFFER = NULL;

static inline uint32_t* fake_mutex_ptr(void) {
    return (uint32_t*)(FAKE_STDIN_BUFFER + DATA_CAP);
}
static inline uint32_t* fake_len_ptr(void) {
    return (uint32_t*)(FAKE_STDIN_BUFFER + DATA_CAP + 4);
}
static inline uint32_t* fake_cond_ptr(void) {
    return (uint32_t*)(FAKE_STDIN_BUFFER + DATA_CAP + 8);
}

static void futex_mutex_lock(uint32_t *m) {
    uint32_t c;
    if ((c = emscripten_atomic_cas_u32(m, 0, 1)) == 0) return;
    
    if (c != 2) c = emscripten_atomic_cas_u32(m, 1, 2); 
    
    while (c != 0) {
        emscripten_futex_wait((volatile void*)m, 2, INFINITY);
        c = emscripten_atomic_cas_u32(m, 0, 2);
    }
}

static void futex_mutex_unlock(uint32_t *m) {
    if (emscripten_atomic_sub_u32(m, 1) != 1) {
        emscripten_atomic_store_u32(m, 0);
        emscripten_futex_wake((volatile void*)m, 1);
    }
}

EMSCRIPTEN_KEEPALIVE uint8_t* alloc_fake_stdin(void) {
    if (FAKE_STDIN_BUFFER != NULL) {
        free(FAKE_STDIN_BUFFER);
    }
    FAKE_STDIN_BUFFER = (uint8_t*)malloc(FAKE_STDIN_CAPACITY);
    
    if (!FAKE_STDIN_BUFFER) return NULL; 

    memset((void*)FAKE_STDIN_BUFFER, 0, (size_t)FAKE_STDIN_CAPACITY);

    emscripten_atomic_store_u32(fake_mutex_ptr(), 0);
    emscripten_atomic_store_u32(fake_len_ptr(), 0);
    emscripten_atomic_store_u32(fake_cond_ptr(), 0);
    
    return FAKE_STDIN_BUFFER;
}

EMSCRIPTEN_KEEPALIVE int fake_stdin_push_char(char c) {
    if (!FAKE_STDIN_BUFFER) return 0;

    futex_mutex_lock(fake_mutex_ptr());

    uint32_t len = emscripten_atomic_load_u32(fake_len_ptr());
    
    if (c == 8) {
        if (len > 0) {
            emscripten_atomic_store_u32(fake_len_ptr(), len - 1);
        }
    } else {
        if (len >= DATA_CAP) {
            futex_mutex_unlock(fake_mutex_ptr());
            return 0;
        }

        FAKE_STDIN_BUFFER[len] = (uint8_t)c;
        emscripten_atomic_store_u32(fake_len_ptr(), len + 1);
    }

    futex_mutex_unlock(fake_mutex_ptr());

    emscripten_atomic_add_u32(fake_cond_ptr(), 1);
    emscripten_futex_wake((volatile void*)fake_cond_ptr(), INT_MAX);

    return 1;
}

EMSCRIPTEN_KEEPALIVE int fake_stdin_available(void) {
    if (!FAKE_STDIN_BUFFER) return 0;
    return (int)emscripten_atomic_load_u32(fake_len_ptr());
}

EMSCRIPTEN_KEEPALIVE int fake_stdin_read(void* dest, int len) {
    if (!FAKE_STDIN_BUFFER || len <= 0) return 0;

    futex_mutex_lock(fake_mutex_ptr());

    while (1) {
        uint32_t avail = emscripten_atomic_load_u32(fake_len_ptr());
        void* newline_pos = memchr(FAKE_STDIN_BUFFER, '\n', avail);

        if (newline_pos != NULL || avail == DATA_CAP) {
            
            uint32_t copy_limit = (newline_pos != NULL) ? 
                (uint32_t)((uint8_t*)newline_pos - FAKE_STDIN_BUFFER + 1) : avail;
            
            uint32_t tocopy = (copy_limit < (uint32_t)len) ? copy_limit : (uint32_t)len;

            memcpy(dest, FAKE_STDIN_BUFFER, tocopy);

            uint32_t newlen = avail - tocopy;
            if (newlen > 0) {
                memmove(FAKE_STDIN_BUFFER, FAKE_STDIN_BUFFER + tocopy, newlen);
            }
            emscripten_atomic_store_u32(fake_len_ptr(), newlen);

            futex_mutex_unlock(fake_mutex_ptr());
            return (int)tocopy;
        }

        uint32_t cond_val = emscripten_atomic_load_u32(fake_cond_ptr());
        
        futex_mutex_unlock(fake_mutex_ptr());

        emscripten_futex_wait((volatile void*)fake_cond_ptr(), cond_val, INFINITY);
        futex_mutex_lock(fake_mutex_ptr());
    }
}