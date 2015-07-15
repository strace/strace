#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <assert.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/resource.h>

int
main(void)
{
	int fds[2];
	int s;
	pid_t pid;
	struct rusage rusage = {};
	siginfo_t info = {};

	(void) close(0);
	(void) close(1);
	assert(!pipe(fds) && fds[0] == 0 && fds[1] == 1);

	pid = fork();
	assert(pid >= 0);
	if (!pid) {
		char c;
		(void) close(1);
		assert(read(0, &c, sizeof(c)) == 1);
		return 42;
	}

	(void) close(0);
	assert(wait4(pid, &s, WNOHANG | __WALL, NULL) == 0);
	assert(waitid(P_PID, pid, &info, WNOHANG | WEXITED) == 0);

	assert(write(1, "", 1) == 1);
	(void) close(1);
	assert(wait4(pid, &s, 0, &rusage) == pid);
	assert(WIFEXITED(s) && WEXITSTATUS(s) == 42);

	pid = fork();
	assert(pid >= 0);
	if (!pid) {
		(void) raise(SIGUSR1);
		return 77;
	}
	assert(wait4(pid, &s, __WALL, NULL) == pid);
	assert(WIFSIGNALED(s) && WTERMSIG(s) == SIGUSR1);

	pid = fork();
	assert(pid >= 0);
	if (!pid) {
		raise(SIGSTOP);
		return 0;
	}
	assert(wait4(pid, &s, WUNTRACED, NULL) == pid);
	assert(WIFSTOPPED(s) && WSTOPSIG(s) == SIGSTOP);

	assert(kill(pid, SIGCONT) == 0);
	assert(waitid(P_PID, pid, &info, WEXITED | WSTOPPED) == 0);
	assert(info.si_code == CLD_EXITED && info.si_status == 0);

	assert(wait4(-1, &s, WNOHANG | WUNTRACED | __WALL, &rusage) == -1);

	return 0;
}
