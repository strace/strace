#define FD0_PATH "</dev/null<char 1:3>>"
#define FD7_PATH "</dev/full<char 1:7>>"
#define SKIP_IF_PROC_IS_UNAVAILABLE skip_if_unavailable("/proc/self/fd/")

#include "dup3.c"
