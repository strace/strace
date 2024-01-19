#define SKIP_IF_PROC_IS_UNAVAILABLE skip_if_unavailable("/proc/self/fd/")
#define PRINT_SIGNALFD

#include "signalfd4.c"
