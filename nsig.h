#ifndef STRACE_NSIG_H
#define STRACE_NSIG_H

#include <signal.h>

#ifndef NSIG
# warning NSIG is not defined, using 32
# define NSIG  32
#elif NSIG < 32
# error NSIG < 32
#endif

#endif /* !STRACE_NSIG_H */
