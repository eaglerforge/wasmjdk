#ifndef _SYS_PROCFS_H_SHIM
#define _SYS_PROCFS_H_SHIM

#include <sys/time.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Basic types required by most procfs implementations */
typedef unsigned long elf_greg_t;
#define ELF_NGREG 32 // Standard placeholder count
typedef elf_greg_t elf_gregset_t[ELF_NGREG];

typedef struct {
    // Placeholder for floating point registers
    char dummy[128];
} elf_fpregset_t;

/* prstatus_t is the most commonly used structure in JVM signal/dump code */
typedef struct prstatus {
    int pr_cursig;            /* Current signal */
    unsigned long pr_sigpend; /* Set of pending signals */
    unsigned long pr_sighold; /* Set of held signals */
    pid_t pr_pid;
    pid_t pr_ppid;
    pid_t pr_pgrp;
    pid_t pr_sid;
    struct timeval pr_utime;  /* User time */
    struct timeval pr_stime;  /* System time */
    struct timeval pr_cutime; /* Cumulative user time */
    struct timeval pr_cstime; /* Cumulative system time */
    elf_gregset_t pr_reg;     /* GP registers */
    int pr_fpvalid;           /* FP registers valid? */
} prstatus_t;

/* Identification of the process/thread */
typedef struct prpsinfo {
    char pr_state;     /* Numeric process state */
    char pr_sname;     /* Char for process state */
    char pr_zomb;      /* Zombie */
    char pr_nice;      /* Nice value */
    unsigned long pr_flag; /* Flags */
    uid_t pr_uid;
    gid_t pr_gid;
    pid_t pr_pid, pr_ppid, pr_pgrp, pr_sid;
    char pr_fname[16]; /* Filename of executable */
    char pr_psargs[80];/* Initial part of arg list */
} prpsinfo_t;

/* Address types */
typedef void* psaddr_t;

#ifdef __cplusplus
}
#endif

#endif /* _SYS_PROCFS_H_SHIM */