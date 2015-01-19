/*
 * Copyright (c) 2015 Dmitry V. Levin <ldv@altlinux.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <asm/ioctl.h>

typedef unsigned short u16;

static const char *
dir2str(const char *name, u16 dir)
{
	switch (dir) {
#define CASE(x) case x: return #x
		CASE(_IOC_NONE);
		CASE(_IOC_READ);
		CASE(_IOC_WRITE);
		CASE(_IOC_READ|_IOC_WRITE);
	}

	static char buf[3 + sizeof(dir) * 2];
	fprintf(stderr,
		"print_ioctlents: WARNING: invalid dir 0x%02x in %s\n",
		dir, name);
	snprintf(buf, sizeof(buf), "0x%02x", dir);
	return buf;
}

void
print_ioctlent(const char *info, const char *name,
	       u16 dir, u16 type, u16 nr, u16 size)
{
	unsigned int type_nr =
		((unsigned) type << _IOC_TYPESHIFT) |
		((unsigned) nr << _IOC_NRSHIFT);

	if (dir & ~_IOC_DIRMASK)
		fprintf(stderr,
			"print_ioctlents: WARNING: dir 0x%02x is out of mask 0x%02x in %s\n",
			dir, _IOC_DIRMASK, name);
	if (type & ~_IOC_TYPEMASK)
		fprintf(stderr,
			"print_ioctlents: WARNING: type 0x%02x is out of mask 0x%02x in %s\n",
			type, _IOC_TYPEMASK, name);
	if (nr & ~_IOC_NRMASK)
		fprintf(stderr,
			"print_ioctlents: WARNING: nr 0x%02x is out of mask 0x%02x in %s\n",
			nr, _IOC_NRMASK, name);
	if (size & ~_IOC_SIZEMASK)
		fprintf(stderr,
			"print_ioctlents: WARNING: size 0x%02x is out of mask 0x%02x in %s\n",
			size, _IOC_SIZEMASK, name);

	printf("{ \"%s\", \"%s\", %s, 0x%04x, 0x%02x },\n",
		info, name, dir2str(name, dir), type_nr, size);
}
