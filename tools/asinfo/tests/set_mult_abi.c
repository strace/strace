#include <stdio.h>
#include "ref_asinfo_output.h"

int
main(int argc, char *argv[])
{
	puts("1" X86_64_64bit_STR "\n"
	     "2" AARCH64_64bit_STR "\n"
	     "3" AARCH64_eabi_STR );
	return 0;
}
