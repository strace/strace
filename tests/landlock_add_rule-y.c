#define FD0_STR "</dev/null>"
#define RULESET_FD 7
#define RULESET_FD_STR "7</dev/zero>"
#define PARENT_FD 8
#define PARENT_FD_STR "8</dev/full>"
#define SKIP_IF_PROC_IS_UNAVAILABLE skip_if_unavailable("/proc/self/fd/")

#include "landlock_add_rule.c"
