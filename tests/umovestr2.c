#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>

int
main(void)
{
	const size_t page_len = sysconf(_SC_PAGESIZE);
	const size_t work_len = page_len * 2;
	const size_t tail_len = work_len - 1;

	void *p = mmap(NULL, page_len * 3, PROT_READ | PROT_WRITE,
		       MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (p == MAP_FAILED || mprotect(p + work_len, page_len, PROT_NONE))
		return 77;

	memset(p, 0, work_len);
	char *addr = p + work_len - tail_len;
	memset(addr, '0', tail_len - 1);

	char *argv[] = { NULL };
	char *envp[] = { addr, NULL };
	execve("", argv, envp);

	printf("execve(\"\", [], [\"%0*u\"]) = -1 ENOENT (No such file or directory)\n",
	       (int) tail_len - 1, 0);
	puts("+++ exited with 0 +++");

	return 0;
}
