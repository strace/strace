#include <unistd.h>

int f3(int i)
{
	return getpid() + i;
}
