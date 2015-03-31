#include <string.h>
#include <unistd.h>
#include <sys/mman.h>

int
main(void)
{
	const size_t page_len = sysconf(_SC_PAGESIZE);
	const size_t tail_len = 257;

	if (tail_len >= page_len)
		return 77;

	void *p = mmap(NULL, page_len * 2, PROT_READ | PROT_WRITE,
		       MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (p == MAP_FAILED || mprotect(p + page_len, page_len, PROT_NONE))
		return 77;

	memset(p, 0, page_len);
	char *addr = p + page_len - tail_len;
	memset(addr, '/', tail_len - 1);
	if (chdir(addr))
		return 77;

	return 0;
}
