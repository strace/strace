#define FD_PATH "</dev/full<char 1:7>>"
#define SKIP_IF_PROC_IS_UNAVAILABLE skip_if_unavailable("/proc/self/fd/")

#include "faccessat.c"
