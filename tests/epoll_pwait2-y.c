#define DECODE_FDS 1
#define SKIP_IF_PROC_IS_UNAVAILABLE skip_if_unavailable("/proc/self/fd/")

#include "epoll_pwait2.c"
