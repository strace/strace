#include <time.h>
#include <utime.h>
#include <errno.h>
#include <stdio.h>

static void
print_tm(struct tm *p)
{
	printf("%02d/%02d/%02d-%02d:%02d:%02d",
	       p->tm_year + 1900, p->tm_mon + 1, p->tm_mday,
	       p->tm_hour, p->tm_min, p->tm_sec);
}

int
main(void)
{
	time_t t = time(NULL);
	struct utimbuf u = { .actime = t, .modtime = t };
	struct tm *p = localtime(&t);

	printf("utime(\"utime\\nfilename\", [");
	print_tm(p);
	printf(", ");
	print_tm(p);
	puts("]) = -1 ENOENT (No such file or directory)");
	puts("+++ exited with 0 +++");

	return utime("utime\nfilename", &u) == -1 && errno == ENOENT ? 0 : 77;
}
