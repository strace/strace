/*
 * Check decoding of ioctl SG_IO v3 commands.
 *
 * Copyright (c) 2017-2021 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#ifdef HAVE_SCSI_SG_H

# include <inttypes.h>
# include <stdio.h>
# include <sys/ioctl.h>
# include <sys/uio.h>
# include <scsi/sg.h>

int
main(void)
{
	ioctl(-1, SG_IO, 0);
	printf("ioctl(-1, SG_IO, NULL) = -1 EBADF (%m)\n");

	TAIL_ALLOC_OBJECT_CONST_PTR(struct sg_io_hdr, sg_io);
	fill_memory(sg_io, sizeof(*sg_io));

	const void *const efault = sg_io + 1;
	ioctl(-1, SG_IO, efault);
	printf("ioctl(-1, SG_IO, %p) = -1 EBADF (%m)\n", efault);

	ioctl(-1, SG_IO, sg_io);
	printf("ioctl(-1, SG_IO, [%u]) = -1 EBADF (%m)\n", sg_io->interface_id);

	TAIL_ALLOC_OBJECT_CONST_PTR(unsigned int, piid);
	*piid = (unsigned char) 'S';
	ioctl(-1, SG_IO, piid);
	printf("ioctl(-1, SG_IO, {interface_id='S', %p}) = -1 EBADF (%m)\n", piid + 1);

	sg_io->interface_id = (unsigned char) 'S';
	sg_io->dxfer_direction = -2;
	sg_io->flags = -1U;
	sg_io->info = -1U;
	sg_io->dxferp = (void *) (unsigned long) 0xfacefeedfffffff1ULL;
	sg_io->cmdp = (void *) (unsigned long) 0xfacefeedfffffff2ULL;
	sg_io->sbp = (void *) (unsigned long) 0xfacefeedfffffff3ULL;

	ioctl(-1, SG_IO, sg_io);
	printf("ioctl(-1, SG_IO, {interface_id='S'"
	       ", dxfer_direction=SG_DXFER_TO_DEV"
	       ", cmd_len=%u"
	       ", cmdp=%p"
	       ", mx_sb_len=%u"
	       ", iovec_count=%u"
	       ", dxfer_len=%u"
	       ", timeout=%u"
	       ", flags=SG_FLAG_DIRECT_IO|SG_FLAG_UNUSED_LUN_INHIBIT"
	       "|SG_FLAG_MMAP_IO|SG_FLAG_NO_DXFER"
	       "|SG_FLAG_Q_AT_TAIL|SG_FLAG_Q_AT_HEAD|0xfffeffc8"
	       ", dxferp=%p"
	       ", status=%#x"
	       ", masked_status=%#x"
	       ", msg_status=%#x"
	       ", sb_len_wr=%u"
	       ", sbp=%p"
	       ", host_status=%#x"
	       ", driver_status=%#x"
	       ", resid=%d"
	       ", duration=%u"
	       ", info=SG_INFO_CHECK|SG_INFO_DIRECT_IO|SG_INFO_MIXED_IO|0xfffffff8"
	       "}) = -1 EBADF (%m)\n",
	       sg_io->cmd_len,
	       sg_io->cmdp,
	       sg_io->mx_sb_len,
	       sg_io->iovec_count,
	       sg_io->dxfer_len,
	       sg_io->timeout,
	       sg_io->dxferp,
	       sg_io->status,
	       sg_io->masked_status,
	       sg_io->msg_status,
	       sg_io->sb_len_wr,
	       sg_io->sbp,
	       sg_io->host_status,
	       sg_io->driver_status,
	       sg_io->resid,
	       sg_io->duration);

	sg_io->dxfer_direction = -3;

	ioctl(-1, SG_IO, sg_io);
	printf("ioctl(-1, SG_IO, {interface_id='S'"
	       ", dxfer_direction=SG_DXFER_FROM_DEV"
	       ", cmd_len=%u"
	       ", cmdp=%p"
	       ", mx_sb_len=%u"
	       ", iovec_count=%u"
	       ", dxfer_len=%u"
	       ", timeout=%u"
	       ", flags=SG_FLAG_DIRECT_IO|SG_FLAG_UNUSED_LUN_INHIBIT"
	       "|SG_FLAG_MMAP_IO|SG_FLAG_NO_DXFER"
	       "|SG_FLAG_Q_AT_TAIL|SG_FLAG_Q_AT_HEAD|0xfffeffc8"
	       ", dxferp=%p"
	       ", status=%#x"
	       ", masked_status=%#x"
	       ", msg_status=%#x"
	       ", sb_len_wr=%u"
	       ", sbp=%p"
	       ", host_status=%#x"
	       ", driver_status=%#x"
	       ", resid=%d"
	       ", duration=%u"
	       ", info=SG_INFO_CHECK|SG_INFO_DIRECT_IO|SG_INFO_MIXED_IO|0xfffffff8"
	       "}) = -1 EBADF (%m)\n",
	       sg_io->cmd_len,
	       sg_io->cmdp,
	       sg_io->mx_sb_len,
	       sg_io->iovec_count,
	       sg_io->dxfer_len,
	       sg_io->timeout,
	       sg_io->dxferp,
	       sg_io->status,
	       sg_io->masked_status,
	       sg_io->msg_status,
	       sg_io->sb_len_wr,
	       sg_io->sbp,
	       sg_io->host_status,
	       sg_io->driver_status,
	       sg_io->resid,
	       sg_io->duration);

	const struct iovec iov[] = {
		{
			.iov_base = (void *) efault - 2,
			.iov_len = 2
		}, {
			.iov_base = (void *) efault - 3,
			.iov_len = 4
		}
	};
	struct iovec *const t_iov = tail_memdup(iov, sizeof(iov));

	sg_io->flags = 0x24;
	sg_io->info = 1;
	sg_io->dxfer_direction = -2;

	sg_io->iovec_count = ARRAY_SIZE(iov);
	sg_io->dxfer_len = iov[0].iov_len + iov[1].iov_len - 1;
	sg_io->dxferp = t_iov;

	ioctl(-1, SG_IO, sg_io);
	printf("ioctl(-1, SG_IO, {interface_id='S'"
	       ", dxfer_direction=SG_DXFER_TO_DEV"
	       ", cmd_len=%u"
	       ", cmdp=%p"
	       ", mx_sb_len=%u"
	       ", iovec_count=%u"
	       ", dxfer_len=%u"
	       ", timeout=%u"
	       ", flags=SG_FLAG_MMAP_IO|SG_FLAG_Q_AT_HEAD"
	       ", dxferp=[{iov_base=\"\\%o\\%o\", iov_len=%u}"
	       ", {iov_base=\"\\%o\\%o\\%o\", iov_len=%u}]"
	       ", status=%#x"
	       ", masked_status=%#x"
	       ", msg_status=%#x"
	       ", sb_len_wr=%u"
	       ", sbp=%p"
	       ", host_status=%#x"
	       ", driver_status=%#x"
	       ", resid=%d"
	       ", duration=%u"
	       ", info=SG_INFO_CHECK"
	       "}) = -1 EBADF (%m)\n",
	       sg_io->cmd_len,
	       sg_io->cmdp,
	       sg_io->mx_sb_len,
	       sg_io->iovec_count,
	       sg_io->dxfer_len,
	       sg_io->timeout,
	       *(unsigned char *) (iov[0].iov_base + 0),
	       *(unsigned char *) (iov[0].iov_base + 1),
	       (unsigned int) iov[0].iov_len,
	       *(unsigned char *) (iov[1].iov_base + 0),
	       *(unsigned char *) (iov[1].iov_base + 1),
	       *(unsigned char *) (iov[1].iov_base + 2),
	       (unsigned int) iov[1].iov_len,
	       sg_io->status,
	       sg_io->masked_status,
	       sg_io->msg_status,
	       sg_io->sb_len_wr,
	       sg_io->sbp,
	       sg_io->host_status,
	       sg_io->driver_status,
	       sg_io->resid,
	       sg_io->duration);

	sg_io->flags = 0x11;
	sg_io->dxfer_direction = -3;
	sg_io->resid = sg_io->dxfer_len + 1;

	ioctl(-1, SG_IO, sg_io);
	printf("ioctl(-1, SG_IO, {interface_id='S'"
	       ", dxfer_direction=SG_DXFER_FROM_DEV"
	       ", cmd_len=%u"
	       ", cmdp=%p"
	       ", mx_sb_len=%u"
	       ", iovec_count=%u"
	       ", dxfer_len=%u"
	       ", timeout=%u"
	       ", flags=SG_FLAG_DIRECT_IO|SG_FLAG_Q_AT_TAIL"
	       ", dxferp=[{iov_base=\"\\%o\\%o\", iov_len=%u}"
	       ", {iov_base=\"\\%o\\%o\\%o\", iov_len=%u}]"
	       ", status=%#x"
	       ", masked_status=%#x"
	       ", msg_status=%#x"
	       ", sb_len_wr=%u"
	       ", sbp=%p"
	       ", host_status=%#x"
	       ", driver_status=%#x"
	       ", resid=%d"
	       ", duration=%u"
	       ", info=SG_INFO_CHECK"
	       "}) = -1 EBADF (%m)\n",
	       sg_io->cmd_len,
	       sg_io->cmdp,
	       sg_io->mx_sb_len,
	       sg_io->iovec_count,
	       sg_io->dxfer_len,
	       sg_io->timeout,
	       *(unsigned char *) (iov[0].iov_base + 0),
	       *(unsigned char *) (iov[0].iov_base + 1),
	       (unsigned int) iov[0].iov_len,
	       *(unsigned char *) (iov[1].iov_base + 0),
	       *(unsigned char *) (iov[1].iov_base + 1),
	       *(unsigned char *) (iov[1].iov_base + 2),
	       (unsigned int) iov[1].iov_len,
	       sg_io->status,
	       sg_io->masked_status,
	       sg_io->msg_status,
	       sg_io->sb_len_wr,
	       sg_io->sbp,
	       sg_io->host_status,
	       sg_io->driver_status,
	       sg_io->resid,
	       sg_io->duration);

	sg_io->flags = 0x10000;
	sg_io->info = 0xdeadbeef;
	sg_io->iovec_count = 0;
	sg_io->dxfer_len = 5;
	sg_io->resid = 1;
	sg_io->dxferp = (void *) efault - (sg_io->dxfer_len - sg_io->resid);

	ioctl(-1, SG_IO, sg_io);
	printf("ioctl(-1, SG_IO, {interface_id='S'"
	       ", dxfer_direction=SG_DXFER_FROM_DEV"
	       ", cmd_len=%u"
	       ", cmdp=%p"
	       ", mx_sb_len=%u"
	       ", iovec_count=%u"
	       ", dxfer_len=%u"
	       ", timeout=%u"
	       ", flags=SG_FLAG_NO_DXFER"
	       ", dxferp=\"\\x%x\\x%x\\x%x\\x%x\""
	       ", status=%#x"
	       ", masked_status=%#x"
	       ", msg_status=%#x"
	       ", sb_len_wr=%u"
	       ", sbp=%p"
	       ", host_status=%#x"
	       ", driver_status=%#x"
	       ", resid=%d"
	       ", duration=%u"
	       ", info=SG_INFO_CHECK|SG_INFO_DIRECT_IO|SG_INFO_MIXED_IO|0xdeadbee8"
	       "}) = -1 EBADF (%m)\n",
	       sg_io->cmd_len,
	       sg_io->cmdp,
	       sg_io->mx_sb_len,
	       sg_io->iovec_count,
	       sg_io->dxfer_len,
	       sg_io->timeout,
	       *(unsigned char *) (sg_io->dxferp + 0),
	       *(unsigned char *) (sg_io->dxferp + 1),
	       *(unsigned char *) (sg_io->dxferp + 2),
	       *(unsigned char *) (sg_io->dxferp + 3),
	       sg_io->status,
	       sg_io->masked_status,
	       sg_io->msg_status,
	       sg_io->sb_len_wr,
	       sg_io->sbp,
	       sg_io->host_status,
	       sg_io->driver_status,
	       sg_io->resid,
	       sg_io->duration);

	sg_io->flags = 2;
	sg_io->dxfer_direction = -4;
	sg_io->dxfer_len = 3;
	sg_io->resid = 1;
	sg_io->dxferp = (void *) efault - sg_io->dxfer_len;

	ioctl(-1, SG_IO, sg_io);
	printf("ioctl(-1, SG_IO, {interface_id='S'"
	       ", dxfer_direction=SG_DXFER_TO_FROM_DEV"
	       ", cmd_len=%u"
	       ", cmdp=%p"
	       ", mx_sb_len=%u"
	       ", iovec_count=%u"
	       ", dxfer_len=%u"
	       ", timeout=%u"
	       ", flags=SG_FLAG_UNUSED_LUN_INHIBIT"
	       ", dxferp=\"\\x%x\\x%x\\x%x\" => dxferp=\"\\x%x\\x%x\""
	       ", status=%#x"
	       ", masked_status=%#x"
	       ", msg_status=%#x"
	       ", sb_len_wr=%u"
	       ", sbp=%p"
	       ", host_status=%#x"
	       ", driver_status=%#x"
	       ", resid=%d"
	       ", duration=%u"
	       ", info=SG_INFO_CHECK|SG_INFO_DIRECT_IO|SG_INFO_MIXED_IO|0xdeadbee8"
	       "}) = -1 EBADF (%m)\n",
	       sg_io->cmd_len,
	       sg_io->cmdp,
	       sg_io->mx_sb_len,
	       sg_io->iovec_count,
	       sg_io->dxfer_len,
	       sg_io->timeout,
	       *(unsigned char *) (sg_io->dxferp + 0),
	       *(unsigned char *) (sg_io->dxferp + 1),
	       *(unsigned char *) (sg_io->dxferp + 2),
	       *(unsigned char *) (sg_io->dxferp + 0),
	       *(unsigned char *) (sg_io->dxferp + 1),
	       sg_io->status,
	       sg_io->masked_status,
	       sg_io->msg_status,
	       sg_io->sb_len_wr,
	       sg_io->sbp,
	       sg_io->host_status,
	       sg_io->driver_status,
	       sg_io->resid,
	       sg_io->duration);

	sg_io->flags = 0;
	sg_io->resid = sg_io->dxfer_len;

	ioctl(-1, SG_IO, sg_io);
	printf("ioctl(-1, SG_IO, {interface_id='S'"
	       ", dxfer_direction=SG_DXFER_TO_FROM_DEV"
	       ", cmd_len=%u"
	       ", cmdp=%p"
	       ", mx_sb_len=%u"
	       ", iovec_count=%u"
	       ", dxfer_len=%u"
	       ", timeout=%u"
	       ", flags=0"
	       ", dxferp=\"\\x%x\\x%x\\x%x\""
	       ", status=%#x"
	       ", masked_status=%#x"
	       ", msg_status=%#x"
	       ", sb_len_wr=%u"
	       ", sbp=%p"
	       ", host_status=%#x"
	       ", driver_status=%#x"
	       ", resid=%d"
	       ", duration=%u"
	       ", info=SG_INFO_CHECK|SG_INFO_DIRECT_IO|SG_INFO_MIXED_IO|0xdeadbee8"
	       "}) = -1 EBADF (%m)\n",
	       sg_io->cmd_len,
	       sg_io->cmdp,
	       sg_io->mx_sb_len,
	       sg_io->iovec_count,
	       sg_io->dxfer_len,
	       sg_io->timeout,
	       *(unsigned char *) (sg_io->dxferp + 0),
	       *(unsigned char *) (sg_io->dxferp + 1),
	       *(unsigned char *) (sg_io->dxferp + 2),
	       sg_io->status,
	       sg_io->masked_status,
	       sg_io->msg_status,
	       sg_io->sb_len_wr,
	       sg_io->sbp,
	       sg_io->host_status,
	       sg_io->driver_status,
	       sg_io->resid,
	       sg_io->duration);

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("HAVE_SCSI_SG_H")

#endif
