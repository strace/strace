#define FD_PATH "</dev/null>"
#define SKIP_IF_PROC_IS_UNAVAILABLE skip_if_unavailable("/proc/self/fd/")

#include "memfd_secret-success.c"
