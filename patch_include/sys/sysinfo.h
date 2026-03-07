#ifndef SYSINFO_SHIM_H
#define SYSINFO_SHIM_H

#include <stdint.h>

struct sysinfo {
    long uptime;
    unsigned long loads[3];
    unsigned long totalram;
    unsigned long freeram;
    unsigned long sharedram;
    unsigned long bufferram;
    unsigned long totalswap;
    unsigned long freeswap;
    unsigned short procs;
    char _f[22];
    char _ss[64];
    unsigned int mem_unit;
};

static inline int sysinfo(struct sysinfo *info) {
    if (!info) {
        return -1;
    }

    info->uptime = 0;
    info->totalram = 4ULL * 1024 * 1024;  // 8 gb total
    info->freeram = 2ULL * 1024 * 1024;   // 4 gb free
    info->loads[0] = 10;            // fake load avg
    info->loads[1] = 5;             // fake load avg
    info->loads[2] = 2;             // fake load avg
    info->procs = 100;              // fake proc #
    info->mem_unit = 1024;

    info->sharedram = 0;
    info->bufferram = 0;
    info->totalswap = 0;
    info->freeswap = 0;
    info->_f[0] = '\0';
    info->_ss[0] = '\0';

    return 0; // success
}

#endif // SYSINFO_SHIM_H