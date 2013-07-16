/*
 * Check that strace output contains RT_1 RT_3 RT_31 RT_32 here:
 *  rt_sigprocmask(SIG_BLOCK, [CHLD RT_1 RT_3 RT_31 RT_32], NULL, 8) = 0
 * and here:
 *  sigreturn() (mask [CHLD RT_1 RT_3 RT_31 RT_32]) = 0
 *
 * On x86, both 32-bit and 64-bit strace needs to be checked.
 */
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

void null_handler(int sig)
{
}

int main(int argc, char *argv[])
{
	sigset_t set;
	sigemptyset(&set);
	sigaddset(&set, SIGCHLD);
	sigaddset(&set, 33);
	sigaddset(&set, 35);
	sigaddset(&set, 63);
	sigaddset(&set, 64);
	sigprocmask(SIG_BLOCK, &set, NULL);
	signal(SIGWINCH, null_handler);
	raise(SIGWINCH);
	return 0;
}
