#include <stdio.h>
#include "ref_asinfo_output.h"

int
main(int argc, char *argv[])
{
	//--get-sname write
	puts("1;write;1;\n"
	//--get-sname /write
	     "1;process_vm_writev;311;\n"
	     "2;pwrite64;18;\n"
	     "3;pwritev;296;\n"
	     "4;pwritev2;328;\n"
	     "5;write;1;\n"
	     "6;writev;20;\n"
	//--get-sname write,read
	     "1;read;0;\n"
	     "2;write;1;\n"
	//--get-sname 1
	     "1;write;1;\n"
	//--get-sname 1000
	     "../../asinfo: invalid system call \'1000\'\n"
	//--get-sname helloworld
	     "../../asinfo: invalid system call \'helloworld\'");
	return 0;
}
