#define PRINT_PIDFD_PID 1
#define FD0_PATH "</dev/full<char 1:7>>"
#define SKIP_IF_PROC_IS_UNAVAILABLE skip_if_unavailable("/proc/self/fd/")

#include "pidfd_getfd.c"
