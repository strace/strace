#include <stdio.h>
#include <stdlib.h>

#include <asm/ioctl.h>
#include <linux/types.h>

#include "ioctldefs.h"
#include <linux/atmioc.h>

struct ioctlent {
	const char*	header;
	const char*	name;
	unsigned long	code;
};

struct ioctlent ioctls[] = {
#include "ioctls.h"
};

int nioctls = sizeof(ioctls) / sizeof(ioctls[0]);


int compare(const void* a, const void* b) {
	unsigned long code1 = ((struct ioctlent *) a)->code;
	unsigned long code2 = ((struct ioctlent *) b)->code;
	return (code1 > code2) ? 1 : (code1 < code2) ? -1 : 0;
}


int main(int argc, char** argv) {
	int i;

	qsort(ioctls, nioctls, sizeof(ioctls[0]), compare);
	for (i = 0; i < nioctls; i++)
		printf("\t{\"%s\",\t\"%s\",\t%#lx},\n",
			ioctls[i].header, ioctls[i].name, ioctls[i].code);
	
	return 0;
}


