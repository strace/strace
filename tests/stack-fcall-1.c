#include "stack-fcall.h"

int f1(int i)
{
	return f2(i) + i;
}
