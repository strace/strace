#include <signal.h>
main()
{
	char buf[1024];
	void interrupt();

	signal(SIGINT, interrupt);
	read(0, buf, 1024);
	write(2, "qwerty\n", 7);
	exit(0);
}

interrupt()
{
	write(2, "xyzzy\n", 6);
}
