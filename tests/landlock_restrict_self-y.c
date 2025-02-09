#define RULESET_FD 7
#define RULESET_FD_STR "7</dev/full>"
#define SKIP_IF_PROC_IS_UNAVAILABLE skip_if_unavailable("/proc/self/fd/")

#include "landlock_restrict_self.c"
