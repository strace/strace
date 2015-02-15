#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>

int main(void)
{
	const unsigned long size = sysconf(_SC_PAGESIZE);
	const unsigned long mask = ~(size - 1);

	/* write instruction pointer length to the log */
	if (write(-1, NULL, 2 * sizeof(void *)) >= 0)
		return 77;
	/* just a noticeable line in the log */
	if (munmap(&munmap, 0) >= 0)
		return 77;

	int pid = fork();
	if (pid < 0)
		return 77;

	if (!pid) {
		munmap((void *) ((unsigned long) &munmap & mask), size);
		/* SIGSEGV is expected */
		munmap((void *) (((unsigned long) &munmap & mask) + size), size);
		return 77;
	}

	int status;
	if (wait(&status) != pid ||
	    !WIFSIGNALED(status) ||
	    WTERMSIG(status) != SIGSEGV)
		return 77;

	return 0;
}
