// Test program which explores whether lseek syscall (not llseek!)
// on x32 uses 64-bit offset argument.
// IOW: does _kernel_ truncate it on entry?
// The answer appears to be "no, full 64-bit offset is used".
// strace must show it correctly too - tricky if strace itself is x32 one!
//
// Build: x86_64-gcc -static -Wall -ox32_lseek x32_lseek.c
// Run:   $ strace ./x32_lseek 2>&1 | grep lseek | grep 1250999896321
//        lseek(0, 1250999896321, SEEK_SET) = 1250999896321
#define _GNU_SOURCE
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <asm/unistd.h>
// Ensure we are compiling to 64 bits
struct bug { int t[sizeof(long) > 4 ? 1 : -1]; };
int main(int argc, char **argv)
{
	long ofs = 0x12345678901;
	errno = 0;
	close(0);
	if (open("/etc/passwd", O_RDONLY))
		return 1;
	long r = syscall(
		(long) (__NR_lseek | 0x40000000), // make x32 call
		(long) (0),
		(long) (ofs),
		(long) (SEEK_SET)
	);
	printf("pos:%ld(0x%lx) errno:%m\n", r, r);
	if (!errno)
		printf((r == ofs) ? "64-bit offset used\n" : "Kernel truncated offset\n");
	return 0;
}
