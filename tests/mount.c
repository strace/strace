/*
 * Check decoding of mount syscall.
 *
 * Copyright (c) 2016 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2016-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
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
# define str_unknown "0x200"
# define str_submount_200 "0x4000200"
# define str_mgc_val "0xc0ed0000"
# define str_remount "0x20"
# define str_bind "0x1000"
# define str_ro_nosuid_nodev_noexec "0xf"
# define str_ro_nosuid_nodev_noexec_relatime "0x20000f"
#elif XLAT_VERBOSE
# define str_unknown "0x200 /* MS_??? */"
# define str_submount_200 "0x4000200 /* MS_SUBMOUNT|0x200 */"
# define str_mgc_val "0xc0ed0000 /* MS_MGC_VAL */"
# define str_remount "0x20 /* MS_REMOUNT */"
# define str_bind "0x1000 /* MS_BIND */"
# define str_ro_nosuid_nodev_noexec \
	"0xf /* MS_RDONLY|MS_NOSUID|MS_NODEV|MS_NOEXEC */"
# define str_ro_nosuid_nodev_noexec_relatime \
	"0x20000f /* MS_RDONLY|MS_NOSUID|MS_NODEV|MS_NOEXEC|MS_RELATIME */"
#else /* !XLAT_RAW && !XLAT_VERBOSE */
# define str_unknown "0x200 /* MS_??? */"
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

	rc = mount(bogus, bogus, bogus, 0x200, bogus);
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
