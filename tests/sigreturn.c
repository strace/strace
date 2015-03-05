#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#include <stdlib.h>
#include <signal.h>

#ifdef ASM_SIGRTMIN
# define RT_0 ASM_SIGRTMIN
#else
/* Linux kernel >= 3.18 defines SIGRTMIN to 32 on all architectures. */
# define RT_0 32
#endif

static void handler(int sig)
{
}

int main(void) {
	sigset_t set;
	sigemptyset(&set);
	sigaddset(&set, SIGUSR2);
	sigaddset(&set, SIGCHLD);
	sigaddset(&set, RT_0 +  2);
	sigaddset(&set, RT_0 +  3);
	sigaddset(&set, RT_0 +  4);
	sigaddset(&set, RT_0 + 26);
	sigaddset(&set, RT_0 + 27);
	sigprocmask(SIG_SETMASK, &set, NULL);
	signal(SIGUSR1, handler);
	raise(SIGUSR1);
	return 0;
}
