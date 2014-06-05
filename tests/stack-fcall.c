#include <unistd.h>
#include <sys/types.h>

/* Use "volatile" to avoid compiler optimization. */

int f1(int i)
{
	static pid_t (* volatile g)(void) = getpid;
	return g() + i;
}

int f0(volatile int i)
{
	static int (* volatile g)(int) = f1;
	return g(i) - i;
}

int main(int argc, char** argv)
{
	static int (* volatile g)(int) = f0;
	g(argc);
	return 0;
}
