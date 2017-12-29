#include <unistd.h>
#include "stack-fcall.h"

int f3(int i)
{
	return getpid() + i;
}
