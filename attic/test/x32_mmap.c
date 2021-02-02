// Test program which explores whether mmap's ofs parameter
// is 64-bit, and whether it needs to be shifted << PAGE_SHIFT.
// Apparently it is 64-bit and isn't shifted.
//
// Build: x86_64-gcc -static -Wall -ox32_mmap x32_mmap.c
// Typical output:
// 7f9390696000-7f93906a6000 r--s 12345670000 08:06 2224545 /etc/passwd
//                                ^^^^^^^^^^^
#define _GNU_SOURCE
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <asm/unistd.h>
// Ensure we are compiling to 64 bits
struct bug { int t[sizeof(long) > 4 ? 1 : -1]; };
int main(int argc, char **argv)
{
	long ofs = 0x12345670000; // fails if not page-aligned
	errno = 0;
	close(0);
	if (open("/etc/passwd", O_RDONLY))
		return 1;
	long r = syscall(
		(long) (__NR_mmap | 0x40000000), // make x32 call
		(long) (0),		// start
		(long) (0x10000),	// len
		(long) (PROT_READ),	// prot
		(long) (MAP_SHARED),	// flags
		(long) (0),		// fd
		(long) (ofs)		// ofs
	);
	printf("ret:0x%lx errno:%m\n", r);

	char buf[16*1024];
	sprintf(buf, "/proc/%d/maps", getpid());
	int fd = open(buf, O_RDONLY);
	if (fd > 0) {
		int sz = read(fd, buf, sizeof(buf));
		if (sz > 0)
			write(1, buf, sz);
	}

	return 0;
}
