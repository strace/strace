/*
 * Check decoding of ioctl SG_IO v4 commands.
 *
 * Copyright (c) 2017-2018 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2017-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <inttypes.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/uio.h>
#include <linux/bsg.h>
#define XLAT_MACROS_ONLY
# include "xlat/scsi_sg_commands.h"
#undef XLAT_MACROS_ONLY

int
main(void)
{
	ioctl(-1, SG_IO, 0);
	printf("ioctl(-1, SG_IO, NULL) = -1 EBADF (%m)\n");

	TAIL_ALLOC_OBJECT_CONST_PTR(struct sg_io_v4, sg_io);
	fill_memory(sg_io, sizeof(*sg_io));

	const void *const efault = sg_io + 1;
	ioctl(-1, SG_IO, efault);
	printf("ioctl(-1, SG_IO, %p) = -1 EBADF (%m)\n", efault);

	ioctl(-1, SG_IO, sg_io);
	printf("ioctl(-1, SG_IO, [%u]) = -1 EBADF (%m)\n", sg_io->guard);

	TAIL_ALLOC_OBJECT_CONST_PTR(unsigned int, pguard);
	*pguard = (unsigned char) 'Q';
	ioctl(-1, SG_IO, pguard);
	printf("ioctl(-1, SG_IO, {guard='Q', %p}) = -1 EBADF (%m)\n", pguard + 1);

	sg_io->guard = (unsigned char) 'Q';
	sg_io->protocol = 0;
	sg_io->subprotocol = 1;
	sg_io->flags = -1U;
	sg_io->info = -1U;
	sg_io->request = (kernel_ulong_t) 0xfacefeedfffffff1ULL;
	sg_io->response = (kernel_ulong_t) 0xfacefeedfffffff2ULL;
	sg_io->dout_xferp = (kernel_ulong_t) 0xfacefeedfffffff3ULL;
	sg_io->din_xferp = (kernel_ulong_t) 0xfacefeedfffffff4ULL;
	ioctl(-1, SG_IO, sg_io);
	printf("ioctl(-1, SG_IO, {guard='Q'"
	       ", protocol=BSG_PROTOCOL_SCSI"
	       ", subprotocol=BSG_SUB_PROTOCOL_SCSI_TMF"
	       ", request_len=%u"
	       ", request=%#llx"
	       ", request_tag=%#" PRI__x64
	       ", request_attr=%u"
	       ", request_priority=%u"
	       ", request_extra=%u"
	       ", max_response_len=%u"
	       ", dout_iovec_count=%u"
	       ", dout_xfer_len=%u"
	       ", din_iovec_count=%u"
	       ", din_xfer_len=%u"
	       ", dout_xferp=%#llx"
	       ", timeout=%u"
	       ", flags=BSG_FLAG_Q_AT_TAIL|BSG_FLAG_Q_AT_HEAD|0xffffffcf"
	       ", usr_ptr=%#" PRI__x64
	       ", response_len=%u"
	       ", response=%#llx"
	       ", din_xferp=%#llx"
	       ", driver_status=%#x"
	       ", transport_status=%#x"
	       ", device_status=%#x"
	       ", retry_delay=%u"
	       ", info=SG_INFO_CHECK|SG_INFO_DIRECT_IO|SG_INFO_MIXED_IO|0xfffffff8"
	       ", duration=%u"
	       ", response_len=%u"
	       ", din_resid=%d"
	       ", dout_resid=%d"
	       ", generated_tag=%#" PRI__x64 "}) = -1 EBADF (%m)\n",
	       sg_io->request_len,
	       (unsigned long long) (kernel_ulong_t) sg_io->request,
	       sg_io->request_tag,
	       sg_io->request_attr,
	       sg_io->request_priority,
	       sg_io->request_extra,
	       sg_io->max_response_len,
	       sg_io->dout_iovec_count,
	       sg_io->dout_xfer_len,
	       sg_io->din_iovec_count,
	       sg_io->din_xfer_len,
	       (unsigned long long) (kernel_ulong_t) sg_io->dout_xferp,
	       sg_io->timeout,
	       sg_io->usr_ptr,
	       sg_io->response_len,
	       (unsigned long long) (kernel_ulong_t) sg_io->response,
	       (unsigned long long) (kernel_ulong_t) sg_io->din_xferp,
	       sg_io->driver_status,
	       sg_io->transport_status,
	       sg_io->device_status,
	       sg_io->retry_delay,
	       sg_io->duration,
	       sg_io->response_len,
	       sg_io->din_resid,
	       sg_io->dout_resid,
	       sg_io->generated_tag);

	const struct iovec iov[] = {
		{
			.iov_base = (void *) efault - 2,
			.iov_len = 2
		}, {
			.iov_base = (void *) efault - 3,
			.iov_len = 4
		}
	};
	const struct iovec *const t_iov = tail_memdup(iov, sizeof(iov));
	sg_io->dout_iovec_count = ARRAY_SIZE(iov);
	sg_io->dout_xfer_len = iov[0].iov_len + iov[1].iov_len - 1;
	sg_io->dout_xferp = (unsigned long) t_iov;

	sg_io->din_iovec_count = 0;
	sg_io->din_xfer_len = 5;
	sg_io->din_resid = 1;
	sg_io->din_xferp = (unsigned long) efault -
		(sg_io->dout_xfer_len - sg_io->din_resid);

	sg_io->request_len = 3;
	sg_io->request = (unsigned long) efault - sg_io->request_len;
	sg_io->response_len = 2;
	sg_io->response = (unsigned long) efault - sg_io->response_len;

	sg_io->flags = 0x20;
	sg_io->info = 1;

	ioctl(-1, SG_IO, sg_io);
	printf("ioctl(-1, SG_IO, {guard='Q'"
	       ", protocol=BSG_PROTOCOL_SCSI"
	       ", subprotocol=BSG_SUB_PROTOCOL_SCSI_TMF"
	       ", request_len=%u"
	       ", request=\"\\x%x\\x%x\\x%x\""
	       ", request_tag=%#" PRI__x64
	       ", request_attr=%u"
	       ", request_priority=%u"
	       ", request_extra=%u"
	       ", max_response_len=%u"
	       ", dout_iovec_count=%u"
	       ", dout_xfer_len=%u"
	       ", din_iovec_count=%u"
	       ", din_xfer_len=%u"
	       ", dout_xferp=[{iov_base=\"\\%o\\%o\", iov_len=%u}"
	       ", {iov_base=\"\\%o\\%o\\%o\", iov_len=%u}]"
	       ", timeout=%u, flags=BSG_FLAG_Q_AT_HEAD"
	       ", usr_ptr=%#" PRI__x64
	       ", response_len=%u"
	       ", response=\"\\x%x\\x%x\""
	       ", din_xferp=\"\\x%x\\x%x\\x%x\\x%x\""
	       ", driver_status=%#x"
	       ", transport_status=%#x"
	       ", device_status=%#x"
	       ", retry_delay=%u"
	       ", info=SG_INFO_CHECK"
	       ", duration=%u"
	       ", response_len=%u"
	       ", din_resid=%d"
	       ", dout_resid=%d"
	       ", generated_tag=%#" PRI__x64 "}) = -1 EBADF (%m)\n",
	       sg_io->request_len,
	       *(unsigned char *) ((unsigned long) sg_io->request + 0),
	       *(unsigned char *) ((unsigned long) sg_io->request + 1),
	       *(unsigned char *) ((unsigned long) sg_io->request + 2),
	       sg_io->request_tag,
	       sg_io->request_attr,
	       sg_io->request_priority,
	       sg_io->request_extra,
	       sg_io->max_response_len,
	       sg_io->dout_iovec_count,
	       sg_io->dout_xfer_len,
	       sg_io->din_iovec_count,
	       sg_io->din_xfer_len,
	       *(unsigned char *) (iov[0].iov_base + 0),
	       *(unsigned char *) (iov[0].iov_base + 1),
	       (unsigned int) iov[0].iov_len,
	       *(unsigned char *) (iov[1].iov_base + 0),
	       *(unsigned char *) (iov[1].iov_base + 1),
	       *(unsigned char *) (iov[1].iov_base + 2),
	       (unsigned int) iov[1].iov_len,
	       sg_io->timeout,
	       sg_io->usr_ptr,
	       sg_io->response_len,
	       *(unsigned char *) ((unsigned long) sg_io->response + 0),
	       *(unsigned char *) ((unsigned long) sg_io->response + 1),
	       *(unsigned char *) ((unsigned long) sg_io->din_xferp + 0),
	       *(unsigned char *) ((unsigned long) sg_io->din_xferp + 1),
	       *(unsigned char *) ((unsigned long) sg_io->din_xferp + 2),
	       *(unsigned char *) ((unsigned long) sg_io->din_xferp + 3),
	       sg_io->driver_status,
	       sg_io->transport_status,
	       sg_io->device_status,
	       sg_io->retry_delay,
	       sg_io->duration,
	       sg_io->response_len,
	       sg_io->din_resid,
	       sg_io->dout_resid,
	       sg_io->generated_tag);

	puts("+++ exited with 0 +++");
	return 0;
}
