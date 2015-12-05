#define TEST_SYSCALL_INVOKE(sample, pst) \
	syscall(TEST_SYSCALL_NR, 0, pst)
#define PRINT_SYSCALL_HEADER(sample) \
	printf("%s(0, ", TEST_SYSCALL_STR)
#define PRINT_SYSCALL_FOOTER \
	puts(") = 0")

#define TEST_SYSCALL_NR nrify(TEST_SYSCALL_NAME)
#define nrify(arg) nrify_(arg)
#define nrify_(arg) __NR_ ## arg

#define USE_ASM_STAT

#include "xstatx.c"
