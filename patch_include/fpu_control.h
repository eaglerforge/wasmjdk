#ifndef FPU_CONTROL_SHIM_H
#define FPU_CONTROL_SHIM_H

#include <stdint.h>

#define _FPU_SSE         0x0001
#define _FPU_DOUBLE      0x0002
#define _FPU_SINGLE      0x0004
#define _FPU_ROUND_NEAR  0x0008

typedef uint32_t fpu_control_t;
typedef int pid_t;

static inline fpu_control_t _mm_get_fpu_control() {
    return 0;
}

static inline void _mm_set_fpu_control(fpu_control_t control) {
    return;
}

static inline void fpsetround(int round) {
    return;
}

static inline int fpgetround() {
    return _FPU_ROUND_NEAR;
}

#endif