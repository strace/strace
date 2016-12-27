/*
 * This file is part of ioctl_loop strace test.
 *
 * Copyright (c) 2016 JingPiao Chen <chenjingpiao@gmail.com>
 * Copyright (c) 2016 Eugene Syromyatnikov <evgsyr@gmail.com>
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
#include <stdbool.h>
#include <string.h>
#include <inttypes.h>
#include <sys/ioctl.h>
#include <sys/sysmacros.h>
#include <linux/loop.h>
#include "xlat/loop_cmds.h"

static void
print_loop_info(struct loop_info * const info, bool print_encrypt,
		const char *encrypt_type, const char *encrypt_key,
		const char *flags)
{
	printf("{lo_number=%d", info->lo_number);
# if VERBOSE
	printf(", lo_device=makedev(%u, %u), lo_inode=%lu, "
	       "lo_rdevice=makedev(%u, %u)",
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

	printf(", lo_name=\"%.*s\"",
	       (int) sizeof(info->lo_name) - 1, info->lo_name);

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
}

static void
print_loop_info64(struct loop_info64 * const info64, bool print_encrypt,
		  const char *encrypt_type, const char *encrypt_key,
		  const char *flags)
{
# if VERBOSE
	printf("{lo_device=makedev(%u, %u), lo_inode=%" PRIu64
	       ", lo_rdevice=makedev(%u, %u), lo_offset=%#" PRIx64
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
	printf(", lo_file_name=\"%.*s\"",
	       (int) sizeof(info64->lo_file_name) - 1, info64->lo_file_name);

	if (VERBOSE || print_encrypt)
		printf(", lo_crypt_name=\"%.*s\", lo_encrypt_key=\"%.*s\"",
		       (int) sizeof(info64->lo_crypt_name) - 1,
		       info64->lo_crypt_name,
		       encrypt_key ? (int) strlen(encrypt_key) :
		       (int) sizeof(info64->lo_encrypt_key),
		       encrypt_key ? encrypt_key :
		       (char *) info64->lo_encrypt_key);

# if VERBOSE
	printf(", lo_init=[%#" PRIx64 ", %#" PRIx64 "]}",
	       (uint64_t) info64->lo_init[0],
	       (uint64_t) info64->lo_init[1]);
# else /* !VERBOSE */
	printf(", ...}");
# endif /* VERBOSE */
}

static const unsigned int magic = 0xdeadbeef;
static const unsigned long lmagic = (unsigned long) 0xdeadbeefbadc0dedULL;

int
main(void)
{
	ioctl(-1, LOOP_SET_FD, magic);
	printf("ioctl(-1, LOOP_SET_FD, %d) = -1 EBADF (%m)\n", magic);

	ioctl(-1, LOOP_CLR_FD, 0);
	printf("ioctl(-1, LOOP_CLR_FD) = -1 EBADF (%m)\n");

	struct loop_info *const info = tail_alloc(sizeof(*info));

	fill_memory((void *) info, sizeof(*info));
	info->lo_encrypt_type = LO_CRYPT_NONE;
	info->lo_flags = LO_FLAGS_READ_ONLY;
	memset(info->lo_name, 'A', sizeof(info->lo_name));
	memset(info->lo_encrypt_key, 'B', sizeof(info->lo_encrypt_key));
	printf("ioctl(-1, LOOP_SET_STATUS, ");
	print_loop_info(info, false, "LO_CRYPT_NONE", NULL,
			"LO_FLAGS_READ_ONLY");
	ioctl(-1, LOOP_SET_STATUS, info);
	printf(") = -1 EBADF (%m)\n");

	ioctl(-1, LOOP_GET_STATUS, info);
	printf("ioctl(-1, LOOP_GET_STATUS, %p) = -1 EBADF (%m)\n", info);

	struct loop_info64 *const info64 = tail_alloc(sizeof(*info64));

	fill_memory((void *) info64, sizeof(*info64));
	info64->lo_flags = LO_FLAGS_READ_ONLY;
	info64->lo_encrypt_type = LO_CRYPT_NONE;
	memset(info64->lo_file_name, 'C', sizeof(info64->lo_file_name));
	memset(info64->lo_crypt_name, 'D', sizeof(info64->lo_crypt_name));
	memset(info64->lo_encrypt_key, 'E', sizeof(info64->lo_encrypt_key));

	printf("ioctl(-1, LOOP_SET_STATUS64, ");
	print_loop_info64(info64, false, "LO_CRYPT_NONE", NULL,
			  "LO_FLAGS_READ_ONLY");
	ioctl(-1, LOOP_SET_STATUS64, info64);
	printf(") = -1 EBADF (%m)\n");

	ioctl(-1, LOOP_GET_STATUS64, info64);
	printf("ioctl(-1, LOOP_GET_STATUS64, %p) = -1 EBADF (%m)\n", info64);

	ioctl(-1, LOOP_CHANGE_FD, magic);
	printf("ioctl(-1, LOOP_CHANGE_FD, %d) = -1 EBADF (%m)\n", magic);

	ioctl(-1, LOOP_SET_CAPACITY, 0);
	printf("ioctl(-1, LOOP_SET_CAPACITY) = -1 EBADF (%m)\n");

	ioctl(-1, LOOP_SET_DIRECT_IO, lmagic);
	printf("ioctl(-1, LOOP_SET_DIRECT_IO, %lu) = -1 EBADF (%m)\n", lmagic);

	ioctl(-1, LOOP_CTL_ADD, magic);
	printf("ioctl(-1, LOOP_CTL_ADD, %d) = -1 EBADF (%m)\n", magic);

	ioctl(-1, LOOP_CTL_REMOVE, magic);
	printf("ioctl(-1, LOOP_CTL_REMOVE, %d) = -1 EBADF (%m)\n", magic);

	ioctl(-1, LOOP_CTL_GET_FREE, 0);
	printf("ioctl(-1, LOOP_CTL_GET_FREE) = -1 EBADF (%m)\n");

	puts("+++ exited with 0 +++");
	return 0;
}
