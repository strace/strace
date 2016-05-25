/*
 * Check decoding of mount syscall.
 *
 * Copyright (c) 2016 Dmitry V. Levin <ldv@altlinux.org>
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

#include "tests.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/mount.h>

#ifndef MS_MGC_VAL
# define MS_MGC_VAL 0xC0ED0000
#endif

#ifndef MS_RELATIME
# define MS_RELATIME (1ul << 21)
#endif

#define str_ro_nosuid_nodev_noexec "MS_RDONLY|MS_NOSUID|MS_NODEV|MS_NOEXEC"

int
main(void)
{
	static const char source[] = "mount_source";
	static const char target[] = "mount_target";
	static const char fstype[] = "mount_fstype";
	static const char data[] = "mount_data";

	int rc = mount(source, target, fstype, 15, data);
	printf("mount(\"%s\", \"%s\", \"%s\", %s, \"%s\") = %d %s (%m)\n",
	       source, target, fstype, str_ro_nosuid_nodev_noexec,
	       data, rc, errno2name());

	rc = mount(source, target, fstype, MS_RELATIME | 15, data);
	printf("mount(\"%s\", \"%s\", \"%s\", %s, \"%s\") = %d %s (%m)\n",
	       source, target, fstype,
	       str_ro_nosuid_nodev_noexec "|MS_RELATIME",
	       data, rc, errno2name());

	rc = mount(source, target, fstype, MS_MGC_VAL, data);
	printf("mount(\"%s\", \"%s\", \"%s\", %s, \"%s\") = %d %s (%m)\n",
	       source, target, fstype, "MS_MGC_VAL", data, rc, errno2name());

	rc = mount(source, target, fstype, MS_MGC_VAL | 15, data);
	printf("mount(\"%s\", \"%s\", \"%s\", %s, \"%s\") = %d %s (%m)\n",
	       source, target, fstype,
	       "MS_MGC_VAL|" str_ro_nosuid_nodev_noexec,
	       data, rc, errno2name());

	rc = mount(source, target, fstype, MS_REMOUNT, data);
	printf("mount(\"%s\", \"%s\", %p, %s, \"%s\") = %d %s (%m)\n",
	       source, target, fstype, "MS_REMOUNT", data, rc, errno2name());

	rc = mount(source, target, fstype, MS_BIND, data);
	printf("mount(\"%s\", \"%s\", %p, %s, %p) = %d %s (%m)\n",
	       source, target, fstype, "MS_BIND", data, rc, errno2name());

	puts("+++ exited with 0 +++");
	return 0;
}
