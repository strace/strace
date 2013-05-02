#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>

/* Expected order is:
 * Child signals parent
 * Parent got signal
 * Child will exit now
 *
 * The bug we test for: under strace -f, last two lines are swapped
 * because wait syscall is suspended by strace and thus can't be interrupted.
 */

static const char msg1[] = "Child signals parent\n";
static const char msg2[] = "Parent got signal\n";
static const char msg3[] = "Child will exit now\n";

static void handler(int s)
{
	write(1, msg2, sizeof(msg2)-1);
}

static void test()
{
	/* Note: in Linux, signal() installs handler with SA_RESTART flag,
	 * therefore wait will be restarted.
	 */
	signal(SIGALRM, handler);

	if (fork() == 0) {
		/* child */
		sleep(1);
		write(1, msg1, sizeof(msg1)-1);
		kill(getppid(), SIGALRM);
		sleep(1);
		write(1, msg3, sizeof(msg3)-1);
		_exit(0);
	}

	/* parent */
	wait(NULL);
	_exit(0);
}

int main()
{
	char buf1[80];
	char buf2[80];
	char buf3[80];
	int pipefd[2];

	printf("Please run me under 'strace -f'\n");

	pipe(pipefd);

	if (fork() == 0) {
		if (pipefd[1] != 1) {
			dup2(pipefd[1], 1);
			close(pipefd[1]);
		}
		test();
	}

	if (pipefd[0] != 0) {
		dup2(pipefd[0], 0);
		close(pipefd[0]);
	}
	fgets(buf1, 80, stdin);	printf("%s", buf1);
	fgets(buf2, 80, stdin);	printf("%s", buf2);
	fgets(buf3, 80, stdin);	printf("%s", buf3);

	if (strcmp(buf1, msg1) != 0
	 || strcmp(buf2, msg2) != 0
	 || strcmp(buf3, msg3) != 0
	) {
		printf("ERROR! Expected order:\n%s%s%s", msg1, msg2, msg3);
		return 1;
	}
	printf("Good: wait seems to be correctly interrupted by signals\n");
	return 0;
}
