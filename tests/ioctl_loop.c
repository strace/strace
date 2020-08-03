/*
 * This file is part of ioctl_loop strace test.
 *
 * Copyright (c) 2016 JingPiao Chen <chenjingpiao@gmail.com>
 * Copyright (c) 2016 Eugene Syromyatnikov <evgsyr@gmail.com>
 * Copyright (c) 2016-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */


#include "tests.h"
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/sysmacros.h>
#include "scno.h"
#include <linux/ioctl.h>
#include <linux/loop.h>
#include "print_fields.h"

#ifdef HAVE_STRUCT_LOOP_CONFIG
typedef struct loop_config struct_loop_config;
#else
# include "types/loop.h"
#endif

#define XLAT_MACROS_ONLY
#include "xlat/loop_cmds.h"
#undef XLAT_MACROS_ONLY

#ifndef ABBREV
# define ABBREV 0
#endif

static long
sys_ioctl(kernel_long_t fd, kernel_ulong_t cmd, kernel_ulong_t arg)
{
	return syscall(__NR_ioctl, fd, cmd, arg);
}

static void
print_loop_info(struct loop_info * const info, bool print_encrypt,
		const char *encrypt_type, const char *encrypt_key,
		const char *flags)
{
#if ABBREV
	printf("%p", info);
#else
	printf("{lo_number=%d", info->lo_number);
# if VERBOSE
	printf(", lo_device=makedev(%#x, %#x), lo_inode=%lu, "
	       "lo_rdevice=makedev(%#x, %#x)",
	       major(info->lo_device), minor(info->lo_device),
	       info->lo_inode,
	       major(info->lo_rdevice), minor(info->lo_rdevice));
# endif /* VERBOSE */

	printf(", lo_offset=%#x", info->lo_offset);

	if (VERBOSE || print_encrypt) {
		printf(", lo_encrypt_type=");
		if (encrypt_type)
			printf("%s", encrypt_type);
		else
			printf("%#x /* LO_CRYPT_??? */", info->lo_encrypt_type);

		printf(", lo_encrypt_key_size=%" PRIu32,
		       (uint32_t) info->lo_encrypt_key_size);
	}

	printf(", lo_flags=");
	if (flags)
		printf("%s", flags);
	else
		printf("%#x /* LO_FLAGS_??? */", info->lo_flags);

	PRINT_FIELD_CSTRING(", ", *info, lo_name);

	if (VERBOSE || print_encrypt)
		printf(", lo_encrypt_key=\"%.*s\"",
		       encrypt_key ? (int) strlen(encrypt_key) :
		       (int) sizeof(info->lo_encrypt_key),
		       encrypt_key ? encrypt_key :
		       (char *) info->lo_encrypt_key);

# if VERBOSE
	printf(", lo_init=[%#lx, %#lx]"
	       ", reserved=[%#hhx, %#hhx, %#hhx, %#hhx]}",
	       info->lo_init[0], info->lo_init[1],
	       info->reserved[0], info->reserved[1],
	       info->reserved[2], info->reserved[3]);
# else /* !VERBOSE */
	printf(", ...}");
# endif /* VERBOSE */
#endif /* !ABBREV */
}

static void
print_loop_info64(struct loop_info64 * const info64, bool print_encrypt,
		  const char *encrypt_type, const char *encrypt_key,
		  const char *flags)
{
#if ABBREV
	printf("%p", info64);
#else
# if VERBOSE
	printf("{lo_device=makedev(%#x, %#x), lo_inode=%" PRIu64
	       ", lo_rdevice=makedev(%#x, %#x), lo_offset=%#" PRIx64
	       ", lo_sizelimit=%" PRIu64 ", lo_number=%" PRIu32,
	       major(info64->lo_device), minor(info64->lo_device),
	       (uint64_t) info64->lo_inode,
	       major(info64->lo_rdevice), minor(info64->lo_rdevice),
	       (uint64_t) info64->lo_offset,
	       (uint64_t) info64->lo_sizelimit,
	       (uint32_t) info64->lo_number);
# else /* !VERBOSE */
	printf("{lo_offset=%#" PRIx64 ", lo_number=%" PRIu32,
	       (uint64_t) info64->lo_offset,
	       (uint32_t) info64->lo_number);
# endif /* VERBOSE */

	if (VERBOSE || print_encrypt) {
		printf(", lo_encrypt_type=");
		if (encrypt_type)
			printf("%s", encrypt_type);
		else
			printf("%#x /* LO_CRYPT_??? */",
			       info64->lo_encrypt_type);

		printf(", lo_encrypt_key_size=%" PRIu32,
		       info64->lo_encrypt_key_size);
	}

	printf(", lo_flags=");
	if (flags)
		printf("%s", flags);
	else
		printf("%#x /* LO_FLAGS_??? */", info64->lo_flags);
	PRINT_FIELD_CSTRING(", ", *info64, lo_file_name);

	if (VERBOSE || print_encrypt) {
		PRINT_FIELD_CSTRING(", ", *info64, lo_crypt_name);
		printf(", lo_encrypt_key=\"%.*s\"",
		       encrypt_key ? (int) strlen(encrypt_key) :
		       (int) sizeof(info64->lo_encrypt_key),
		       encrypt_key ? encrypt_key :
		       (char *) info64->lo_encrypt_key);
	}

# if VERBOSE
	printf(", lo_init=[%#" PRIx64 ", %#" PRIx64 "]}",
	       (uint64_t) info64->lo_init[0],
	       (uint64_t) info64->lo_init[1]);
# else /* !VERBOSE */
	printf(", ...}");
# endif /* VERBOSE */
#endif /* !ABBREV */
}

static void
print_loop_config(struct_loop_config *config, bool print_reserved)
{
#if ABBREV
	printf("%p", config);
#else
	printf("{fd=%d, block_size=%u, info=",
	       (int) config->fd, config->block_size);
	print_loop_info64(&config->info, false, "LO_CRYPT_NONE", NULL,
			  "LO_FLAGS_READ_ONLY");
	if (print_reserved) {
		printf(", __reserved=");
		for (size_t i = 0; i < ARRAY_SIZE(config->__reserved); ++i)
			printf("%s%#llx", (i ? ", " : "["),
			       (unsigned long long) config->__reserved[i]);
		printf("]");
	}
	printf("}");
#endif /* !ABBREV */
}

int
main(void)
{
	static const kernel_ulong_t unknown_loop_cmd =
		(kernel_ulong_t) 0xbadc0dedfeed4cedULL;
	static const kernel_ulong_t magic =
		(kernel_ulong_t) 0xdeadbeefbadc0dedULL;
	static const kernel_ulong_t kernel_mask =
		((kernel_ulong_t) -1) - ((unsigned long) -1L);

	TAIL_ALLOC_OBJECT_CONST_PTR(struct loop_info, info);
	TAIL_ALLOC_OBJECT_CONST_PTR(struct loop_info64, info64);
	TAIL_ALLOC_OBJECT_CONST_PTR(struct_loop_config, config);

	/* Unknown loop commands */
	sys_ioctl(-1, unknown_loop_cmd, magic);
	printf("ioctl(-1, _IOC(%s_IOC_READ|_IOC_WRITE, 0x4c, %#x, %#x), "
	       "%#lx) = -1 EBADF (%m)\n",
	       _IOC_DIR((unsigned int) unknown_loop_cmd) & _IOC_NONE ?
	       "_IOC_NONE|" : "",
	       _IOC_NR((unsigned int) unknown_loop_cmd),
	       _IOC_SIZE((unsigned int) unknown_loop_cmd),
	       (unsigned long) magic);

	sys_ioctl(-1, LOOP_CONFIGURE + 1, magic);
	printf("ioctl(-1, _IOC(%s, 0x4c, %#x, %#x), %#lx) = "
	       "-1 EBADF (%m)\n",
	       _IOC_NONE ? "0" : "_IOC_NONE",
	       _IOC_NR(LOOP_CONFIGURE + 1),
	       _IOC_SIZE(LOOP_CONFIGURE + 1),
	       (unsigned long) magic);

	sys_ioctl(-1, LOOP_CTL_GET_FREE + 1, magic);
	printf("ioctl(-1, _IOC(%s, 0x4c, %#x, %#x), %#lx) = "
	       "-1 EBADF (%m)\n",
	       _IOC_NONE ? "0" : "_IOC_NONE",
	       _IOC_NR(LOOP_CTL_GET_FREE + 1),
	       _IOC_SIZE(LOOP_CTL_GET_FREE + 1),
	       (unsigned long) magic);

	/* LOOP_SET_FD */
	sys_ioctl(-1, LOOP_SET_FD, magic);
	printf("ioctl(-1, LOOP_SET_FD, %d) = -1 EBADF (%m)\n",
	       (unsigned int) magic);

	/* LOOP_CLR_FD */
	ioctl(-1, LOOP_CLR_FD);
	printf("ioctl(-1, LOOP_CLR_FD) = -1 EBADF (%m)\n");

	/* LOOP_SET_STATUS */
	ioctl(-1, LOOP_SET_STATUS, NULL);
	printf("ioctl(-1, LOOP_SET_STATUS, NULL) = -1 EBADF (%m)\n");

	fill_memory(info, sizeof(*info));
	info->lo_flags = 0xdeface00;
	info->lo_name[0] = '\0';
	info->lo_encrypt_key[0] = '\0';
	info->lo_encrypt_key_size = 1;

	printf("ioctl(-1, LOOP_SET_STATUS, ");
	print_loop_info(info, true, NULL, "\\0", NULL);
	ioctl(-1, LOOP_SET_STATUS, info);
	printf(") = -1 EBADF (%m)\n");

	fill_memory(info, sizeof(*info));
	info->lo_encrypt_type = LO_CRYPT_NONE;
	info->lo_flags = LO_FLAGS_READ_ONLY;
	memset(info->lo_name, 'A', sizeof(info->lo_name));
	memset(info->lo_encrypt_key, 'B', sizeof(info->lo_encrypt_key));

	ioctl(-1, LOOP_SET_STATUS, (void *) info + ALIGNOF(info));
	printf("ioctl(-1, LOOP_SET_STATUS, %p) = -1 EBADF (%m)\n",
	       (void *) info + ALIGNOF(info));

	printf("ioctl(-1, LOOP_SET_STATUS, ");
	print_loop_info(info, false, "LO_CRYPT_NONE", NULL,
			"LO_FLAGS_READ_ONLY");
	ioctl(-1, LOOP_SET_STATUS, info);
	printf(") = -1 EBADF (%m)\n");

	/* LOOP_GET_STATUS */
	ioctl(-1, LOOP_GET_STATUS, NULL);
	printf("ioctl(-1, LOOP_GET_STATUS, NULL) = -1 EBADF (%m)\n");

	ioctl(-1, LOOP_GET_STATUS, (unsigned long) info | kernel_mask);
	printf("ioctl(-1, LOOP_GET_STATUS, %p) = -1 EBADF (%m)\n", info);

	/* LOOP_SET_STATUS64 */
	ioctl(-1, LOOP_SET_STATUS64, NULL);
	printf("ioctl(-1, LOOP_SET_STATUS64, NULL) = -1 EBADF (%m)\n");

	fill_memory(info64, sizeof(*info64));
	info64->lo_flags = 0xdec0de00;
	info64->lo_file_name[0] = '\0';
	info64->lo_crypt_name[0] = '\0';
	info64->lo_encrypt_key[0] = '\0';
	info64->lo_encrypt_key_size = 1;

	printf("ioctl(-1, LOOP_SET_STATUS64, ");
	print_loop_info64(info64, true, NULL, "\\0", NULL);
	ioctl(-1, LOOP_SET_STATUS64, info64);
	printf(") = -1 EBADF (%m)\n");

	fill_memory(info64, sizeof(*info64));
	info64->lo_flags = LO_FLAGS_READ_ONLY;
	info64->lo_encrypt_type = LO_CRYPT_NONE;
	memset(info64->lo_file_name, 'C', sizeof(info64->lo_file_name));
	memset(info64->lo_crypt_name, 'D', sizeof(info64->lo_crypt_name));
	memset(info64->lo_encrypt_key, 'E', sizeof(info64->lo_encrypt_key));

	ioctl(-1, LOOP_SET_STATUS64, (void *) info64 + ALIGNOF(info64));
	printf("ioctl(-1, LOOP_SET_STATUS64, %p) = -1 EBADF (%m)\n",
	       (void *) info64 + ALIGNOF(info64));

	printf("ioctl(-1, LOOP_SET_STATUS64, ");
	print_loop_info64(info64, false, "LO_CRYPT_NONE", NULL,
			  "LO_FLAGS_READ_ONLY");
	ioctl(-1, LOOP_SET_STATUS64, info64);
	printf(") = -1 EBADF (%m)\n");

	/* LOOP_GET_STATUS64 */
	ioctl(-1, LOOP_GET_STATUS64, NULL);
	printf("ioctl(-1, LOOP_GET_STATUS64, NULL) = -1 EBADF (%m)\n");

	ioctl(-1, LOOP_GET_STATUS64, (unsigned long) info64 | kernel_mask);
	printf("ioctl(-1, LOOP_GET_STATUS64, %p) = -1 EBADF (%m)\n", info64);

	/* LOOP_CONFIGURE */
	ioctl(-1, LOOP_CONFIGURE, NULL);
	printf("ioctl(-1, LOOP_CONFIGURE, NULL) = -1 EBADF (%m)\n");

	fill_memory(config, sizeof(*config));
	config->info.lo_flags = LO_FLAGS_READ_ONLY;
	config->info.lo_encrypt_type = LO_CRYPT_NONE;
	memset(config->info.lo_file_name, 'C', sizeof(config->info.lo_file_name));
	memset(config->info.lo_crypt_name, 'D', sizeof(config->info.lo_crypt_name));
	memset(config->info.lo_encrypt_key, 'E', sizeof(config->info.lo_encrypt_key));

	ioctl(-1, LOOP_CONFIGURE, (void *) config + ALIGNOF(config));
	printf("ioctl(-1, LOOP_CONFIGURE, %p) = -1 EBADF (%m)\n",
	       (void *) config + ALIGNOF(config));

	printf("ioctl(-1, LOOP_CONFIGURE, ");
	print_loop_config(config, true);
	ioctl(-1, LOOP_CONFIGURE, config);
	printf(") = -1 EBADF (%m)\n");

	memset(config->__reserved, 0, sizeof(config->__reserved));
	printf("ioctl(-1, LOOP_CONFIGURE, ");
	print_loop_config(config, false);
	ioctl(-1, LOOP_CONFIGURE, config);
	printf(") = -1 EBADF (%m)\n");

	/* LOOP_CHANGE_FD */
	sys_ioctl(-1, LOOP_CHANGE_FD, magic);
	printf("ioctl(-1, LOOP_CHANGE_FD, %d) = -1 EBADF (%m)\n",
	       (unsigned int) magic);

	/* LOOP_SET_CAPACITY */
	ioctl(-1, LOOP_SET_CAPACITY);
	printf("ioctl(-1, LOOP_SET_CAPACITY) = -1 EBADF (%m)\n");

	/* LOOP_SET_DIRECT_IO */
	sys_ioctl(-1, LOOP_SET_DIRECT_IO, magic);
	printf("ioctl(-1, LOOP_SET_DIRECT_IO, %lu) = -1 EBADF (%m)\n",
	       (unsigned long) magic);

	/* LOOP_SET_BLOCK_SIZE */
	sys_ioctl(-1, LOOP_SET_BLOCK_SIZE, magic);
	printf("ioctl(-1, LOOP_SET_BLOCK_SIZE, %lu) = -1 EBADF (%m)\n",
	       (unsigned long) magic);

	/* LOOP_CTL_ADD */
	sys_ioctl(-1, LOOP_CTL_ADD, magic);
	printf("ioctl(-1, LOOP_CTL_ADD, %d) = -1 EBADF (%m)\n",
	       (unsigned int) magic);

	/* LOOP_CTL_REMOVE */
	sys_ioctl(-1, LOOP_CTL_REMOVE, magic);
	printf("ioctl(-1, LOOP_CTL_REMOVE, %d) = -1 EBADF (%m)\n",
	       (unsigned int) magic);

	/* LOOP_CTL_GET_FREE */
	ioctl(-1, LOOP_CTL_GET_FREE);
	printf("ioctl(-1, LOOP_CTL_GET_FREE) = -1 EBADF (%m)\n");

	puts("+++ exited with 0 +++");
	return 0;
}
