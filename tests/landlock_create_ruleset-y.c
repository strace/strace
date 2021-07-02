#define FD_PATH "<anon_inode:landlock-ruleset>"
#define SKIP_IF_PROC_IS_UNAVAILABLE skip_if_unavailable("/proc/self/fd/")

#include "landlock_create_ruleset.c"
