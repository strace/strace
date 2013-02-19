/* Should strace show byte or page offsets in mmap syscalls
 * which take page offset parameters?
 *
 * At the time of writing, sys_mmap() converts page to byte offsets,
 * but only for SH64! But this routine is used on i386 too - by mmap2 syscall,
 * which uses page offsets too. As it stands now, SH64 and i386 are inconsistent.
 *
 * sys_old_mmap() is used for old mmap syscall, which uses byte offset -
 * should be ok.
 * sys_mmap64() is currently buggy (should print bogus offset, but I can't
 * test it right now. What arch/bitness invokes sys_mmap64?)
 *
 * This program is intended for testing what strace actually shows. Usage:
 * $ gcc test/mmap_offset_decode.c -o mmap_offset_decode -static
 * $ strace ./mmap_offset_decode
 *
 * As of today (2011-08), on i386 strace prints page offset.
 * Fixed 2013-02-19. Now all mmaps on all arches should show byte offsets.
 */
#define _LARGEFILE_SOURCE
#define _LARGEFILE64_SOURCE
#define _FILE_OFFSET_BITS 64
#include <sys/mman.h>
#include <errno.h>
int main()
{
	/* 0x1000 is meant to be page size multiplier */
	mmap(0, 4096, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1,
			0x7fff0000LL * 0x1000);
	return errno != 0;
}
