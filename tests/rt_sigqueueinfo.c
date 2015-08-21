#include <stdio.h>
#include <signal.h>
#include <unistd.h>

int
main (void)
{
	struct sigaction sa = {
		.sa_handler = SIG_IGN
	};
	union sigval value = {
		.sival_ptr = (void *) (unsigned long) 0xdeadbeefbadc0ded
	};
	pid_t pid = getpid();

	if (sigaction(SIGUSR1, &sa, NULL) == -1)
		return 77;
	if (sigqueue(pid, SIGUSR1, value) == -1)
		return 77;
	printf("rt_sigqueueinfo(%u, SIGUSR1, {si_signo=SIGUSR1, "
		"si_code=SI_QUEUE, si_pid=%u, si_uid=%u, "
		"si_value={int=%d, ptr=%p}}) = 0\n",
		pid, pid, getuid(), value.sival_int, value.sival_ptr);
	printf("+++ exited with 0 +++\n");

	return 0;
}
