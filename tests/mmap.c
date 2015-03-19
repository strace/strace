#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <limits.h>
#include <sys/mman.h>

int
main(void)
{
	const intmax_t pagesize = sysconf(_SC_PAGESIZE);
	const unsigned long length = pagesize * 3;
	const int fd = -1;
	off_t offset;
	void *addr, *p;

#if ULONG_MAX > 4294967295UL
	offset = 0xcafedeadbeef000 & -pagesize;
	addr = (void *) (uintmax_t) (0xfacefeed000 & -pagesize);
#else
	offset = 0xdeadbeef000 & -pagesize;
	addr = (void *) (unsigned int) (0xfaced000 & -pagesize);
#endif

	p = mmap(addr, length, PROT_READ | PROT_WRITE,
		 MAP_PRIVATE | MAP_ANONYMOUS, fd, offset);
	if (p == MAP_FAILED ||
	    mprotect(p, length, PROT_NONE) ||
	    munmap(p, length))
		return 77;

	if (sizeof(offset) == sizeof(int))
		printf("(mmap2?|old_mmap)\\(%p, %lu, PROT_READ\\|PROT_WRITE, "
		       "MAP_PRIVATE\\|MAP_ANONYMOUS, %d, %#x\\) = %p\n",
		       addr, length, fd, (unsigned int) offset, p);
	else
		printf("(mmap2?|old_mmap)\\(%p, %lu, PROT_READ\\|PROT_WRITE, "
		       "MAP_PRIVATE\\|MAP_ANONYMOUS, %d, %#jx\\) = %p\n",
		       addr, length, fd, (uintmax_t) offset, p);
	printf("mprotect\\(%p, %lu, PROT_NONE\\) += 0\n", p, length);
	printf("munmap\\(%p, %lu\\) += 0\n", p, length);
	return 0;
}
