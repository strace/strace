#define SKIP_IF_PROC_IS_UNAVAILABLE skip_if_unavailable("/proc/self/fd/")
#define FD0_PATH "</dev/null>"
#define FD7_PATH "</dev/zero>"
#include "sockaddr_xlat-Xverbose.c"
