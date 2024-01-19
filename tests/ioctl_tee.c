/*
 * Copyright (c) 2020-2023 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include <fcntl.h>
#include <stdio.h>
#include <string.h>

#include <linux/tee.h>

#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "xlat.h"

#define NUM_PARAMS 8
#define UUID_SIZE 16

#ifndef TEE_IOCTL_SHM_MAPPED
# define TEE_IOCTL_SHM_MAPPED 0x1
#endif
#ifndef TEE_IOCTL_SHM_DMA_BUF
# define TEE_IOCTL_SHM_DMA_BUF 0x2
#endif

/* Not in mainline.  */
struct tee_ioctl_shm_register_fd_data {
	__s64 fd;
	__u64 size;
	__u32 flags;
	__u8  _pad1[4];
	__u32 id;
	__u8  _pad2[4];
} ATTRIBUTE_ALIGNED(8);

#define TEE_IOC_SHM_REGISTER_FD _IOWR(TEE_IOC_MAGIC, TEE_IOC_BASE + 8, \
					struct tee_ioctl_shm_register_fd_data)

typedef struct {
	uint8_t b[UUID_SIZE];
} uuid_t;

#define UUID_INIT(a_, b_, c_, d0, d1, d2, d3, d4, d5, d6, d7)		\
{ .b = {((a_) >> 24) & 0xff, ((a_) >> 16) & 0xff,			\
        ((a_) >> 8) & 0xff, (a_) & 0xff,				\
        ((b_) >> 8) & 0xff, (b_) & 0xff,				\
        ((c_) >> 8) & 0xff, (c_) & 0xff,				\
        (d0), (d1), (d2), (d3), (d4), (d5), (d6), (d7)} }

#define CHK_NULL(ioctl_)						\
	do {								\
		ioctl(-1, ioctl_, NULL);				\
		printf("ioctl(-1, " #ioctl_ ", NULL)" RVAL_EBADF);	\
	} while (0)

#define CHK_BUF(ioctl_)						\
	do {								\
		ioctl(-1, ioctl_, &buf_data);				\
		printf("ioctl(-1, " #ioctl_				\
		       ", {buf_len=%llu, buf_ptr=%#llx})" RVAL_EBADF,	\
		       (unsigned long long) buf_data.buf_len,		\
		       (unsigned long long) buf_data.buf_ptr);		\
	} while (0)

#define DEFINE_BUF_W_PARAMS(type_, shorthand_)				\
	union {								\
		type_ shorthand_;					\
		struct {						\
			uint8_t type_buf[sizeof(type_)];		\
			struct tee_ioctl_param params[NUM_PARAMS];	\
		} data;							\
	} shorthand_ ## _buf

static const unsigned long one_beef = (unsigned long) 0xcafef00ddeadbeefULL;
static const unsigned long two_beef = (unsigned long) 0xbadc0dedbadc0dedULL;
static const unsigned long red_beef = (unsigned long) 0xdefacedbeef0beefULL;
static const unsigned long blu_beef = (unsigned long) 0xfacefeedcafef00dULL;

static const uuid_t uuid_beef = UUID_INIT(0xdeadbeef, 0xcafe, 0xc0de,
					  0xba, 0xdc, 0x0d, 0xed,
					  0xfa, 0xce, 0xfe, 0xed);

static void
fill_params(struct tee_ioctl_param *params)
{
	for (unsigned i = 0; i < NUM_PARAMS; i++) {
		params[i].a = two_beef;
		params[i].b = red_beef;
		params[i].c = blu_beef;
	}
	params[0].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;
	params[1].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INPUT;
	params[2].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_OUTPUT;
	params[3].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INOUT;
	params[4].attr = TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_INPUT;
	params[5].attr = TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_OUTPUT;
	params[6].attr = TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_INOUT;
	params[7].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INPUT |
			 TEE_IOCTL_PARAM_ATTR_META;
}

static void
print_params(struct tee_ioctl_param *params)
{
	printf("{attr=TEE_IOCTL_PARAM_ATTR_TYPE_NONE}, "
	       "{attr=TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INPUT, a=%#llx, b=%#llx, c=%#llx}, "
	       "{attr=TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_OUTPUT, a=%#llx, b=%#llx, c=%#llx}, "
	       "{attr=TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INOUT, a=%#llx, b=%#llx, c=%#llx}, "
	       "{attr=TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_INPUT, shm_offs=%#llx, size=%#llx, shm_id=%llu}, "
	       "{attr=TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_OUTPUT, shm_offs=%#llx, size=%#llx, shm_id=%llu}, "
	       "{attr=TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_INOUT, shm_offs=%#llx, size=%#llx, shm_id=%llu}, "
	       "{attr=TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INPUT|TEE_IOCTL_PARAM_ATTR_META, a=%#llx, b=%#llx, c=%#llx}",
	       (unsigned long long) two_beef, (unsigned long long) red_beef,
	       (unsigned long long) blu_beef, (unsigned long long) two_beef,
	       (unsigned long long) red_beef, (unsigned long long) blu_beef,
	       (unsigned long long) two_beef, (unsigned long long) red_beef,
	       (unsigned long long) blu_beef, (unsigned long long) two_beef,
	       (unsigned long long) red_beef, (unsigned long long) blu_beef,
	       (unsigned long long) two_beef, (unsigned long long) red_beef,
	       (unsigned long long) blu_beef, (unsigned long long) two_beef,
	       (unsigned long long) red_beef, (unsigned long long) blu_beef,
	       (unsigned long long) two_beef, (unsigned long long) red_beef,
	       (unsigned long long) blu_beef
	       );
}

int
main(void)
{
	gid_t gid;

	struct tee_ioctl_cancel_arg cancel;
	struct tee_ioctl_shm_alloc_data shm_alloc;
	struct tee_ioctl_shm_register_data shm_register;
	struct tee_ioctl_close_session_arg close_session;
	struct tee_ioctl_shm_register_fd_data shm_register_fd;

	struct tee_ioctl_buf_data buf_data;

	DEFINE_BUF_W_PARAMS(struct tee_ioctl_invoke_arg, invoke);
	DEFINE_BUF_W_PARAMS(struct tee_iocl_supp_recv_arg, supp_recv);
	DEFINE_BUF_W_PARAMS(struct tee_iocl_supp_send_arg, supp_send);
	DEFINE_BUF_W_PARAMS(struct tee_ioctl_open_session_arg, open_session);

	static const char null_path[] = "/dev/null";
	int fd = open(null_path, O_RDONLY);

	/* NULL as arg */
	CHK_NULL(TEE_IOC_CANCEL);
	CHK_NULL(TEE_IOC_INVOKE);
	CHK_NULL(TEE_IOC_VERSION);
	CHK_NULL(TEE_IOC_SHM_ALLOC);
	CHK_NULL(TEE_IOC_SUPPL_RECV);
	CHK_NULL(TEE_IOC_SUPPL_SEND);
	CHK_NULL(TEE_IOC_OPEN_SESSION);
	CHK_NULL(TEE_IOC_SHM_REGISTER);
	CHK_NULL(TEE_IOC_CLOSE_SESSION);

	/* Valid parameterless calls */
	ioctl(-1, TEE_IOC_SHM_REGISTER_FD, NULL);
	printf("ioctl(-1, _IOC(_IOC_READ|_IOC_WRITE, 0xa4, 0x8, 0x20), NULL)"
	       RVAL_EBADF);
	ioctl(-1, _IOC(_IOC_NONE, 0xa4, 0xa, 0), NULL);
	printf("ioctl(-1, _IOC(_IOC_NONE, 0xa4, 0xa, 0), 0)" RVAL_EBADF);

	cancel.cancel_id = (uint32_t) one_beef;
	cancel.session = (uint32_t) two_beef;
	ioctl(-1, TEE_IOC_CANCEL, &cancel);
	printf("ioctl(-1, TEE_IOC_CANCEL, {cancel_id=%u, session=%#x})" RVAL_EBADF,
	       (uint32_t) one_beef, (uint32_t) two_beef);

	close_session.session = (uint32_t) red_beef;
	ioctl(-1, TEE_IOC_CLOSE_SESSION, &close_session);
	printf("ioctl(-1, TEE_IOC_CLOSE_SESSION, {session=%#x})" RVAL_EBADF,
	       (uint32_t) red_beef);

	shm_alloc.size = one_beef;
	shm_alloc.flags = TEE_IOCTL_SHM_MAPPED | TEE_IOCTL_SHM_DMA_BUF | 0x80;
	ioctl(-1, TEE_IOC_SHM_ALLOC, &shm_alloc);
	printf("ioctl(-1, TEE_IOC_SHM_ALLOC, {size=%#llx, "
	       "flags=TEE_IOCTL_SHM_MAPPED|TEE_IOCTL_SHM_DMA_BUF|0x80})" RVAL_EBADF,
	       (unsigned long long) one_beef);

	shm_register.addr = red_beef;
	shm_register.length = blu_beef;
	shm_register.flags = TEE_IOCTL_SHM_MAPPED | 0x80;
	ioctl(-1, TEE_IOC_SHM_REGISTER, &shm_register);
	printf("ioctl(-1, TEE_IOC_SHM_REGISTER, {addr=%#llx, length=%#llx, "
	       "flags=TEE_IOCTL_SHM_MAPPED|0x80})" RVAL_EBADF,
	       (unsigned long long) red_beef,
	       (unsigned long long) blu_beef);

	if (fd >= 0) {
		shm_register_fd.fd = fd;
		shm_register_fd.flags = TEE_IOCTL_SHM_DMA_BUF;
		ioctl(-1, TEE_IOC_SHM_REGISTER_FD, &shm_register_fd);
		printf("ioctl(-1, _IOC(_IOC_READ|_IOC_WRITE, 0xa4, 0x8, 0x20), "
		       "{fd=%d, flags=TEE_IOCTL_SHM_DMA_BUF})" RVAL_EBADF, fd);
	}

	/* Beef in buf_data */
	buf_data.buf_ptr = one_beef;
	buf_data.buf_len = two_beef;
	CHK_BUF(TEE_IOC_INVOKE);
	CHK_BUF(TEE_IOC_OPEN_SESSION);
	CHK_BUF(TEE_IOC_SUPPL_RECV);
	CHK_BUF(TEE_IOC_SUPPL_SEND);

	/* Valid calls with parameters */
	invoke_buf.invoke.func = (uint32_t) one_beef;
	invoke_buf.invoke.session = (uint32_t) two_beef;
	invoke_buf.invoke.cancel_id = (uint32_t) red_beef;
	invoke_buf.invoke.num_params = NUM_PARAMS;
	fill_params(invoke_buf.data.params);
	buf_data.buf_ptr = (uintptr_t) &invoke_buf;
	buf_data.buf_len = sizeof(invoke_buf);
	ioctl(-1, TEE_IOC_INVOKE, &buf_data);
	printf("ioctl(-1, TEE_IOC_INVOKE, {buf_len=%llu, "
	       "buf_ptr={func=%u, session=%#x, cancel_id=%u, "
	       "num_params=%u, params=[",
	       (unsigned long long) buf_data.buf_len,
	       (uint32_t) one_beef, (uint32_t) two_beef,
	       (uint32_t) red_beef, NUM_PARAMS);
	print_params(invoke_buf.data.params);
	printf("]}})" RVAL_EBADF);

	open_session_buf.open_session.clnt_login = TEE_IOCTL_LOGIN_PUBLIC;
	gid = (gid_t) blu_beef;
	memcpy(&open_session_buf.open_session.clnt_uuid, &gid, sizeof(gid_t));
	memcpy(&open_session_buf.open_session.uuid, &uuid_beef, UUID_SIZE);
	open_session_buf.open_session.cancel_id = (uint32_t) red_beef;
	open_session_buf.open_session.num_params = NUM_PARAMS;
	fill_params(open_session_buf.data.params);
	buf_data.buf_ptr = (uintptr_t) &open_session_buf;
	buf_data.buf_len = sizeof(open_session_buf);
	ioctl(-1, TEE_IOC_OPEN_SESSION, &buf_data);
	printf("ioctl(-1, TEE_IOC_OPEN_SESSION, {buf_len=%llu, "
	       "buf_ptr={uuid=deadbeef-cafe-c0de-badc-0dedfacefeed, "
	       "clnt_login=TEE_IOCTL_LOGIN_PUBLIC, "
	       "cancel_id=%u, "
	       "num_params=%u, params=[",
	       (unsigned long long) buf_data.buf_len,
	       (uint32_t) red_beef, NUM_PARAMS);
	print_params(open_session_buf.data.params);
	printf("]}})" RVAL_EBADF);

	/* All the login types */
	open_session_buf.open_session.clnt_login = TEE_IOCTL_LOGIN_USER;
	ioctl(-1, TEE_IOC_OPEN_SESSION, &buf_data);
	printf("ioctl(-1, TEE_IOC_OPEN_SESSION, {buf_len=%llu, "
	       "buf_ptr={uuid=deadbeef-cafe-c0de-badc-0dedfacefeed, "
	       "clnt_login=TEE_IOCTL_LOGIN_USER, "
	       "cancel_id=%u, "
	       "num_params=%u, params=[",
	       (unsigned long long) buf_data.buf_len,
	       (uint32_t) red_beef, NUM_PARAMS);
	print_params(open_session_buf.data.params);
	printf("]}})" RVAL_EBADF);

	open_session_buf.open_session.clnt_login = TEE_IOCTL_LOGIN_GROUP;
	ioctl(-1, TEE_IOC_OPEN_SESSION, &buf_data);
	printf("ioctl(-1, TEE_IOC_OPEN_SESSION, {buf_len=%llu, "
	       "buf_ptr={uuid=deadbeef-cafe-c0de-badc-0dedfacefeed, "
	       "clnt_login=TEE_IOCTL_LOGIN_GROUP, "
	       "clnt_uuid=%u, cancel_id=%u, "
	       "num_params=%u, params=[",
	       (unsigned long long) buf_data.buf_len,
	       gid, (uint32_t) red_beef, NUM_PARAMS);
	print_params(open_session_buf.data.params);
	printf("]}})" RVAL_EBADF);

	open_session_buf.open_session.clnt_login = TEE_IOCTL_LOGIN_APPLICATION;
	ioctl(-1, TEE_IOC_OPEN_SESSION, &buf_data);
	printf("ioctl(-1, TEE_IOC_OPEN_SESSION, {buf_len=%llu, "
	       "buf_ptr={uuid=deadbeef-cafe-c0de-badc-0dedfacefeed, "
	       "clnt_login=TEE_IOCTL_LOGIN_APPLICATION, "
	       "cancel_id=%u, "
	       "num_params=%u, params=[",
	       (unsigned long long) buf_data.buf_len,
	       (uint32_t) red_beef, NUM_PARAMS);
	print_params(open_session_buf.data.params);
	printf("]}})" RVAL_EBADF);

	open_session_buf.open_session.clnt_login = TEE_IOCTL_LOGIN_USER_APPLICATION;
	ioctl(-1, TEE_IOC_OPEN_SESSION, &buf_data);
	printf("ioctl(-1, TEE_IOC_OPEN_SESSION, {buf_len=%llu, "
	       "buf_ptr={uuid=deadbeef-cafe-c0de-badc-0dedfacefeed, "
	       "clnt_login=TEE_IOCTL_LOGIN_USER_APPLICATION, "
	       "cancel_id=%u, "
	       "num_params=%u, params=[",
	       (unsigned long long) buf_data.buf_len,
	       (uint32_t) red_beef, NUM_PARAMS);
	print_params(open_session_buf.data.params);
	printf("]}})" RVAL_EBADF);

	open_session_buf.open_session.clnt_login = TEE_IOCTL_LOGIN_GROUP_APPLICATION;
	ioctl(-1, TEE_IOC_OPEN_SESSION, &buf_data);
	printf("ioctl(-1, TEE_IOC_OPEN_SESSION, {buf_len=%llu, "
	       "buf_ptr={uuid=deadbeef-cafe-c0de-badc-0dedfacefeed, "
	       "clnt_login=TEE_IOCTL_LOGIN_GROUP_APPLICATION, "
	       "clnt_uuid=%u, cancel_id=%u, "
	       "num_params=%u, params=[",
	       (unsigned long long) buf_data.buf_len,
	       gid, (uint32_t) red_beef, NUM_PARAMS);
	print_params(open_session_buf.data.params);
	printf("]}})" RVAL_EBADF);

	open_session_buf.open_session.clnt_login = 0xff;
	ioctl(-1, TEE_IOC_OPEN_SESSION, &buf_data);
	printf("ioctl(-1, TEE_IOC_OPEN_SESSION, {buf_len=%llu, "
	       "buf_ptr={uuid=deadbeef-cafe-c0de-badc-0dedfacefeed, "
	       "clnt_login=%#x /* TEE_IOCTL_LOGIN_??? */, "
	       "clnt_uuid=[", (unsigned long long) buf_data.buf_len,
	       open_session_buf.open_session.clnt_login);
	for (unsigned i = 0; i < UUID_SIZE; i++) {
		if (i > 0)
			printf(", ");
		printf("%#x", open_session_buf.open_session.clnt_uuid[i]);
	}
	printf("], cancel_id=%u, "
	       "num_params=%u, params=[",
	       (uint32_t) red_beef, NUM_PARAMS);
	print_params(open_session_buf.data.params);
	printf("]}})" RVAL_EBADF);

	supp_recv_buf.supp_recv.func = (uint32_t) blu_beef;
	supp_recv_buf.supp_recv.num_params = NUM_PARAMS;
	fill_params(supp_recv_buf.data.params);
	buf_data.buf_ptr = (uintptr_t) &supp_recv_buf;
	buf_data.buf_len = sizeof(supp_recv_buf);
	ioctl(-1, TEE_IOC_SUPPL_RECV, &buf_data);
	printf("ioctl(-1, TEE_IOC_SUPPL_RECV, {buf_len=%llu, "
	       "buf_ptr={func=%u, "
	       "num_params=%u, params=[",
	       (unsigned long long) buf_data.buf_len,
	       (uint32_t) blu_beef, NUM_PARAMS);
	print_params(supp_recv_buf.data.params);
	printf("]}})" RVAL_EBADF);

	supp_send_buf.supp_send.num_params = NUM_PARAMS;
	fill_params(supp_send_buf.data.params);
	buf_data.buf_ptr = (uintptr_t) &supp_send_buf;
	buf_data.buf_len = sizeof(supp_send_buf);
	ioctl(-1, TEE_IOC_SUPPL_SEND, &buf_data);
	printf("ioctl(-1, TEE_IOC_SUPPL_SEND, {buf_len=%llu, "
	       "buf_ptr={num_params=%u, params=[",
	       (unsigned long long) buf_data.buf_len,
	       NUM_PARAMS);
	print_params(supp_send_buf.data.params);
	printf("]}})" RVAL_EBADF);

	/* Valid buf, but unmatching num_params */
	invoke_buf.invoke.num_params = 0;
	supp_recv_buf.supp_recv.num_params = 0;
	supp_send_buf.supp_send.num_params = 0;
	open_session_buf.open_session.num_params = 0;

	buf_data.buf_ptr = (uintptr_t) &invoke_buf;
	buf_data.buf_len = sizeof(invoke_buf);
	CHK_BUF(TEE_IOC_INVOKE);
	buf_data.buf_ptr = (uintptr_t) &open_session_buf;
	buf_data.buf_len = sizeof(open_session_buf);
	CHK_BUF(TEE_IOC_OPEN_SESSION);
	buf_data.buf_ptr = (uintptr_t) &supp_recv_buf;
	buf_data.buf_len = sizeof(supp_recv_buf);
	CHK_BUF(TEE_IOC_SUPPL_RECV);
	buf_data.buf_ptr = (uintptr_t) &supp_send_buf;
	buf_data.buf_len = sizeof(supp_send_buf);
	CHK_BUF(TEE_IOC_SUPPL_SEND);

	invoke_buf.invoke.num_params = NUM_PARAMS;
	supp_recv_buf.supp_recv.num_params = NUM_PARAMS;
	supp_send_buf.supp_send.num_params = NUM_PARAMS;
	open_session_buf.open_session.num_params = NUM_PARAMS;

	/* Invalid buf_len */
	buf_data.buf_len = 0;
	CHK_BUF(TEE_IOC_INVOKE);
	CHK_BUF(TEE_IOC_OPEN_SESSION);
	CHK_BUF(TEE_IOC_SUPPL_RECV);
	CHK_BUF(TEE_IOC_SUPPL_SEND);

	buf_data.buf_len = (unsigned long long) -1;
	CHK_BUF(TEE_IOC_INVOKE);
	CHK_BUF(TEE_IOC_OPEN_SESSION);
	CHK_BUF(TEE_IOC_SUPPL_RECV);
	CHK_BUF(TEE_IOC_SUPPL_SEND);

	/* Valid buf_len, invalid buf_ptr */
	buf_data.buf_ptr = one_beef;
	buf_data.buf_len = sizeof(invoke_buf);
	CHK_BUF(TEE_IOC_INVOKE);
	buf_data.buf_len = sizeof(open_session_buf);
	CHK_BUF(TEE_IOC_OPEN_SESSION);
	buf_data.buf_len = sizeof(supp_recv_buf);
	CHK_BUF(TEE_IOC_SUPPL_RECV);
	buf_data.buf_len = sizeof(supp_send_buf);
	CHK_BUF(TEE_IOC_SUPPL_SEND);

	puts("+++ exited with 0 +++");
	return 0;
}
