#include <stdio.h>
#include "ref_asinfo_output.h"

int
main(int argc, char *argv[])
{
	//--get-snum write
	puts("1;1;write;write;-;-;\n"
	     "2;4;-;-;write;write;\n"
	//--get-snum /write
	     "1;1;write;write;-;-;\n"
	     "2;4;-;-;write;write;\n"
	     "3;18;pwrite64;pwrite64;-;-;\n"
	     "4;20;writev;writev#64;-;-;\n"
	     "5;146;-;-;writev;writev;\n"
	     "6;181;-;-;pwrite64;pwrite64;\n"
	     "7;296;pwritev;pwritev#64;-;-;\n"
	     "8;311;process_vm_writev;process_vm_writev#64;-;-;\n"
	     "9;328;pwritev2;pwritev2#64;-;-;\n"
	     "10;334;-;-;pwritev;pwritev;\n"
	     "11;348;-;-;process_vm_writev;process_vm_writev;\n"
	     "12;379;-;-;pwritev2;pwritev2;\n"
	     "13;516;-;writev;-;-;\n"
	     "14;535;-;pwritev;-;-;\n"
	     "15;540;-;process_vm_writev;-;-;\n"
	     "16;547;-;pwritev2;-;-;\n"
	//--get-snum write,read
	     "1;0;read;read;-;-;\n"
	     "2;1;write;write;-;-;\n"
	     "3;3;-;-;read;read;\n"
	     "4;4;-;-;write;write;\n"
	//--get-snum 1
	     "1;1;write;write;exit;exit;\n"
	//--get-snum 1000
	     "../../asinfo: invalid system call \'1000\'\n"
	//--get-snum helloworld
	     "../../asinfo: invalid system call \'helloworld\'");
	return 0;
}
