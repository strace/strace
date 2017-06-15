#include <stdio.h>
#include "ref_asinfo_output.h"

int
main(int argc, char *argv[])
{
	puts(
"| N | Architecture name | ABI mode | IMPL syscalls | IPC IMPL | SOCKET IMPL |\n"
"| 1 |             avr32 |    32bit |           329 | external |    external |"
	);
	puts(
"|   |      | x86_64 | x86_64 | x86_64 |\n"
"| N | Snum |  64bit |    x32 |  32bit |\n"
"| 1 |    1 |  write |  write |      - |\n"
"| 2 |    4 |      - |      - |  write |");
	return 0;
}
