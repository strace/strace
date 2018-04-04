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

#if XLAT_RAW
# define str_unknown "0x300"
# define str_submount_200 "0x4000200"
# define str_mgc_val "0xc0ed0000"
# define str_remount "0x20"
# define str_bind "0x1000"
# define str_ro_nosuid_nodev_noexec "0xf"
# define str_ro_nosuid_nodev_noexec_relatime "0x20000f"
#elif XLAT_VERBOSE
# define str_unknown "0x300 /* MS_??? */"
# define str_submount_200 "0x4000200 /* MS_SUBMOUNT|0x200 */"
# define str_mgc_val "0xc0ed0000 /* MS_MGC_VAL */"
# define str_remount "0x20 /* MS_REMOUNT */"
# define str_bind "0x1000 /* MS_BIND */"
# define str_ro_nosuid_nodev_noexec \
	"0xf /* MS_RDONLY|MS_NOSUID|MS_NODEV|MS_NOEXEC */"
# define str_ro_nosuid_nodev_noexec_relatime \
	"0x20000f /* MS_RDONLY|MS_NOSUID|MS_NODEV|MS_NOEXEC|MS_RELATIME */"
#else /* !XLAT_RAW && !XLAT_VERBOSE */
# define str_unknown "0x300 /* MS_??? */"
# define str_submount_200 "MS_SUBMOUNT|0x200"
# define str_mgc_val "MS_MGC_VAL"
# define str_remount "MS_REMOUNT"
# define str_bind "MS_BIND"
# define str_ro_nosuid_nodev_noexec "MS_RDONLY|MS_NOSUID|MS_NODEV|MS_NOEXEC"
# define str_ro_nosuid_nodev_noexec_relatime \
	"MS_RDONLY|MS_NOSUID|MS_NODEV|MS_NOEXEC|MS_RELATIME"
#endif /* XLAT_RAW, XLAT_VERBOSE */

int
main(void)
{
	static const char source[] = "mount_source";
	static const char target[] = "mount_target";
	static const char fstype[] = "mount_fstype";
	static const char data[] = "mount_data";
	TAIL_ALLOC_OBJECT_CONST_PTR(char, bogus);

	bogus[0] = 'a';

	int rc = mount(NULL, NULL, NULL, 0, NULL);
	printf("mount(NULL, NULL, NULL, 0, NULL) = %s\n",
	       sprintrc(rc));

	rc = mount(bogus, bogus, bogus, 768, bogus);
	printf("mount(%p, %p, %p, %s, %p) = %s\n",
	       bogus, bogus, bogus, str_unknown, bogus, sprintrc(rc));

	rc = mount(bogus + 1, bogus + 1, bogus + 1, 0x4000200, bogus + 1);
	printf("mount(%p, %p, %p, %s, %p) = %s\n",
	       bogus + 1, bogus + 1, bogus + 1, str_submount_200,
	       bogus + 1, sprintrc(rc));

	rc = mount(source, target, fstype, 15, data);
	printf("mount(\"%s\", \"%s\", \"%s\", %s, \"%s\") = %s\n",
	       source, target, fstype, str_ro_nosuid_nodev_noexec,
	       data, sprintrc(rc));

	rc = mount(source, target, fstype, MS_RELATIME | 15, data);
	printf("mount(\"%s\", \"%s\", \"%s\", %s, \"%s\") = %s\n",
	       source, target, fstype,
	       str_ro_nosuid_nodev_noexec_relatime,
	       data, sprintrc(rc));

	rc = mount(source, target, fstype, MS_MGC_VAL, data);
	printf("mount(\"%s\", \"%s\", \"%s\", %s, \"%s\") = %s\n",
	       source, target, fstype, str_mgc_val, data, sprintrc(rc));

	rc = mount(source, target, fstype, MS_MGC_VAL | 15, data);
	printf("mount(\"%s\", \"%s\", \"%s\", %s, \"%s\") = %s\n",
	       source, target, fstype,
	       str_mgc_val "|" str_ro_nosuid_nodev_noexec,
	       data, sprintrc(rc));

	rc = mount(source, target, NULL, MS_REMOUNT, data);
	printf("mount(\"%s\", \"%s\", NULL, %s, \"%s\") = %s\n",
	       source, target, str_remount, data, sprintrc(rc));

	rc = mount(source, target, fstype, MS_REMOUNT, data);
	printf("mount(\"%s\", \"%s\", %p, %s, \"%s\") = %s\n",
	       source, target, fstype, str_remount, data, sprintrc(rc));

	rc = mount(source, target, NULL, MS_BIND, data);
	printf("mount(\"%s\", \"%s\", NULL, %s, %p) = %s\n",
	       source, target, str_bind, data, sprintrc(rc));

	rc = mount(source, target, fstype, MS_BIND, NULL);
	printf("mount(\"%s\", \"%s\", %p, %s, NULL) = %s\n",
	       source, target, fstype, str_bind, sprintrc(rc));

	puts("+++ exited with 0 +++");
	return 0;
}
