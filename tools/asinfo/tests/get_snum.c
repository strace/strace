#include <stdio.h>
#include "ref_asinfo_output.h"

int
main(int argc, char *argv[])
{
	//--get-snum write
	puts("1;1;write;\n"
	//--get-snum /write
	     "1;1;write;\n"
	     "2;18;pwrite64;\n"
	     "3;20;writev;\n"
	     "4;296;pwritev;\n"
	     "5;311;process_vm_writev;\n"
	     "6;328;pwritev2;\n"
	//--get-snum write,read
	     "1;0;read;\n"
	     "2;1;write;\n"
	//--get-snum 1
	     "1;1;write;\n"
	//--get-snum 1000
	     "../../asinfo: invalid system call \'1000\'\n"
	//--get-snum helloworld
	     "../../asinfo: invalid system call \'helloworld\'");
	return 0;
}
