#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/sendfile.h>

int main(void)
{
	const unsigned long size = sysconf(_SC_PAGESIZE);

	/* write instruction pointer length to the log */
	if (write(-1, NULL, 2 * sizeof(void *)) >= 0)
		return 77;

	/* just a noticeable line in the log */
	if (munmap(&main, 0) >= 0)
		return 77;

	int pid = fork();
	if (pid < 0)
		return 77;

	if (!pid) {
		const unsigned long mask = ~(size - 1);
		const unsigned long addr = (unsigned long) &main;

		/* SIGSEGV is expected */
		(void) munmap((void *) ((addr & mask) - size * 2), size * 4);
		(void) munmap((void *) ((addr & mask) - size * 2), size * 4);
		return 77;
	}

	int status;
	if (wait(&status) != pid ||
	    !WIFSIGNALED(status) ||
	    WTERMSIG(status) != SIGSEGV)
		return 77;

	/* dump process map for debug purposes */
	close(0);
	if (!open("/proc/self/maps", O_RDONLY))
		(void) sendfile(1, 0, NULL, size);

	return 0;
}
