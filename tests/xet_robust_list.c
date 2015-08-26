#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/syscall.h>

#if defined __NR_get_robust_list && defined __NR_set_robust_list

int
main(void)
{
	const size_t page_len = sysconf(_SC_PAGESIZE);
	const pid_t pid = getpid();
	const long long_pid = (unsigned long) (0xdeadbeef00000000LL | pid);

	void *p = mmap(NULL, page_len * 4, PROT_READ | PROT_WRITE,
		       MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (p == MAP_FAILED ||
	    mprotect(p + page_len, page_len, PROT_NONE) ||
	    mprotect(p + page_len * 3, page_len, PROT_NONE))
		return 77;

	void **p_head = p + page_len - sizeof(void *);
	size_t *p_len = p + page_len * 3 - sizeof(size_t);

	if (syscall(__NR_get_robust_list, long_pid, p_head, p_len))
		return 77;
	printf("get_robust_list(%d, [%#lx], [%lu]) = 0\n",
	       (int) pid, (unsigned long) *p_head, (unsigned long) *p_len);

	if (syscall(__NR_set_robust_list, p, *p_len))
		return 77;
	printf("set_robust_list(%#lx, %lu) = 0\n",
	       (unsigned long) p, (unsigned long) *p_len);

	if (syscall(__NR_get_robust_list, long_pid, p_head, p_len))
		return 77;
	printf("get_robust_list(%d, [%#lx], [%lu]) = 0\n",
	       (int) pid, (unsigned long) *p_head, (unsigned long) *p_len);

	puts("+++ exited with 0 +++");
	return 0;
}

#else

int
main(void)
{
	return 77;
}

#endif
