#include <signal.h>
#include <unistd.h>

#include "stack-fcall.h"

int f3(int i)
{
	static int pid;

	switch (i) {
	case 1:
		return kill(pid, SIGURG);

	default:
		return (pid = getpid()) + i;
	}

}
