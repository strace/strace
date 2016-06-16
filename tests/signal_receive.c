#include "tests.h"
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

void sig_print(const char *signame, const int pid, const int uid)
{
	printf("kill(%d, %s) = 0\n"
	       "--- %s {si_signo=%s, si_code=SI_USER, si_pid=%d"
	       ", si_uid=%d} ---\n",
	       pid, signame, signame, signame, pid, uid);
}

static void
handler(int sig)
{
}

int
main(void)
{
	int sig, pid = getpid(), uid = getuid();
	const struct sigaction act = { .sa_handler = handler };
	sigset_t mask;
	sigemptyset(&mask);

	for (sig = 1; sig <= 31; sig++) {
		if( sig != SIGKILL && sig != SIGSTOP) {
			sigaction(sig, &act, NULL);
			sigaddset(&mask, sig);
		}
	}
	sigprocmask(SIG_UNBLOCK, &mask, NULL);

	for (sig = 1; sig <= 31; sig++) {
		if(sig != SIGKILL && sig != SIGSTOP) {
			if (kill(pid, sig) != 0)
				perror_msg_and_fail("kill: %d", sig);
			sig_print(signal2name(sig), pid, uid);
		}
	}

	puts("+++ exited with 0 +++");
	return 0;
}
