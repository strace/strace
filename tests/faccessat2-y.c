#define FD_PATH "</dev/full>"
#define SKIP_IF_PROC_IS_UNAVAILABLE skip_if_unavailable("/proc/self/fd/")

#include "faccessat2.c"
