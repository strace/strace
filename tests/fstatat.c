#ifndef HAVE_FSTATAT
# undef TEST_SYSCALL_NAME
#endif

#define TEST_SYSCALL_INVOKE(sample, pst) \
	fstatat(AT_FDCWD, sample, pst, AT_SYMLINK_NOFOLLOW)
#define PRINT_SYSCALL_HEADER(sample) \
	printf("%s(AT_FDCWD, \"%s\", ", TEST_SYSCALL_STR, sample)
#define PRINT_SYSCALL_FOOTER \
	puts(", AT_SYMLINK_NOFOLLOW) = 0")

#include "xstatx.c"
