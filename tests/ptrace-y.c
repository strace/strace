#define NULL_FD_STR "<" NULL_STR ">"
#define SKIP_IF_PROC_IS_UNAVAILABLE skip_if_unavailable("/proc/self/fd/")
#include "ptrace.c"
