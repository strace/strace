#include <stdio.h>
#include "ref_asinfo_output.h"

int
main(int argc, char *argv[])
{
	//--get-sname write
	puts("1;write;1;1;4;4;\n"
	//--get-sname /write
	     "1;process_vm_writev;311;540;348;348;\n"
	     "2;process_vm_writev#64;-;311;-;-;\n"
	     "3;pwrite64;18;18;181;181;\n"
	     "4;pwritev;296;535;334;334;\n"
	     "5;pwritev#64;-;296;-;-;\n"
	     "6;pwritev2;328;547;379;379;\n"
	     "7;pwritev2#64;-;328;-;-;\n"
	     "8;write;1;1;4;4;\n"
	     "9;writev;20;516;146;146;\n"
	     "10;writev#64;-;20;-;-;\n"
	//--get-sname write,read
	     "1;read;0;0;3;3;\n"
	     "2;write;1;1;4;4;\n"
	//--get-sname 1
	     "1;exit;-;-;1;1;\n"
	     "2;write;1;1;-;-;\n"
	//--get-sname 1000
	     "../../asinfo: invalid system call \'1000\'\n"
	//--get-sname helloworld
	     "../../asinfo: invalid system call \'helloworld\'");
	return 0;
}
