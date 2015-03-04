#include <stdlib.h>
#include <signal.h>

static void handler(int sig)
{
}

#define RT_0 32

int main(void) {
	sigset_t set;
	sigemptyset(&set);
	sigaddset(&set, SIGUSR2);
	sigaddset(&set, SIGCHLD);
	sigaddset(&set, RT_0 +  2);
	sigaddset(&set, RT_0 +  3);
	sigaddset(&set, RT_0 +  4);
	sigaddset(&set, RT_0 + 31);
	sigaddset(&set, RT_0 + 32);
	sigprocmask(SIG_SETMASK, &set, NULL);
	signal(SIGUSR1, handler);
	raise(SIGUSR1);
	return 0;
}
