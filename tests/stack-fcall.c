#include <unistd.h>
#include <sys/types.h>

/* Use "volatile" to avoid compiler optimization. */

int f3(int i)
{
	static pid_t (* volatile g)(void) = getpid;
	return g() + i;
}

int f2(volatile int i)
{
	static int (* volatile g)(int) = f3;
	return g(i) - i;
}

int f1(volatile int i)
{
	static int (* volatile g)(int) = f2;
	return g(i) + i;
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
