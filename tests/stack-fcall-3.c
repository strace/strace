#include <signal.h>
#include <unistd.h>
#include <sys/types.h>

#include "stack-fcall.h"

static int pid;

int f3(int i)
{
	switch (i) {
	case 1:
		return kill(pid, SIGURG);

	default:
		return (pid = getpid()) + i;
	}

}
