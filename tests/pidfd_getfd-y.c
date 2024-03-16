#define PRINT_PIDFD_PATH 1
#define FD0_PATH "</dev/full>"
#define SKIP_IF_PROC_IS_UNAVAILABLE skip_if_unavailable("/proc/self/fd/")

#include "pidfd_getfd.c"
