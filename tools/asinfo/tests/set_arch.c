#include <stdio.h>
#include "ref_asinfo_output.h"

int
main(int argc, char *argv[])
{
	puts("1" X86_64_64bit_STR "\n"
	     "2" X86_64_X32_STR "\n"
	     "3" X86_64_32bit_STR);
	return 0;
}
