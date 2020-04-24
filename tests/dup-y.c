#define FD0_PATH "</dev/null>"
#define FD9_PATH "</dev/full>"
#define SKIP_IF_PROC_IS_UNAVAILABLE skip_if_unavailable("/proc/self/fd/")

#include "dup.c"
