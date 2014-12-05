#include <errno.h>

extern int capget(int *, int *);
extern int capset(int *, const int *);

int
main(void)
{
	int unused[6];
	const int data[] = { 2, 4, 0, 8, 16, 0 };
	const int v3 = 0x20080522;
	int head[] = { v3, 0 };

	if (capget(head, unused) || head[0] != v3 ||
	    capset(head, data) == 0 || errno != EPERM)
		return 77;

	return 0;
}
