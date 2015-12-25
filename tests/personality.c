#include <stdio.h>
#include <sys/personality.h>

int main(void)
{
	const unsigned int test_pers =
		SHORT_INODE | WHOLE_SECONDS | STICKY_TIMEOUTS;
	const unsigned int saved_pers = personality(0);

	printf("personality\\(PER_LINUX\\) = %#x \\([^)]*\\)\n", saved_pers);
	personality(test_pers);
	puts("personality\\(SHORT_INODE\\|WHOLE_SECONDS\\|STICKY_TIMEOUTS\\)"
	     " = 0 \\(PER_LINUX\\)");
	personality(saved_pers);
	printf("personality\\([^)]*\\) = %#x"
	       " \\(SHORT_INODE\\|WHOLE_SECONDS\\|STICKY_TIMEOUTS\\)\n",
	       test_pers);

	return 0;
}
