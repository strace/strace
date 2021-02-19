# Automake input for bundled.
#
# Copyright (c) 2021 The strace developers.
# All rights reserved.
#
# SPDX-License-Identifier: LGPL-2.1-or-later

EXTRA_DIST = \
	linux/COPYING \
	linux/GPL-2.0 \
	linux/Linux-syscall-note \
	linux/arch/alpha/include/uapi/asm/fcntl.h \
	linux/arch/arm/include/uapi/asm/fcntl.h \
	linux/arch/arm64/include/uapi/asm/fcntl.h \
	linux/arch/ia64/include/uapi/asm/fcntl.h \
	linux/arch/m68k/include/uapi/asm/fcntl.h \
	linux/arch/mips/include/uapi/asm/fcntl.h \
	linux/arch/parisc/include/uapi/asm/fcntl.h \
	linux/arch/powerpc/include/uapi/asm/fcntl.h \
	linux/arch/sparc/include/uapi/asm/fcntl.h \
	linux/include/uapi/asm-generic/fcntl.h \
	linux/include/uapi/linux/bsg.h \
	linux/include/uapi/linux/btrfs.h \
	linux/include/uapi/linux/btrfs_tree.h \
	linux/include/uapi/linux/close_range.h \
	linux/include/uapi/linux/const.h \
	linux/include/uapi/linux/cryptouser.h \
	linux/include/uapi/linux/dm-ioctl.h \
	linux/include/uapi/linux/dqblk_xfs.h \
	linux/include/uapi/linux/falloc.h \
	linux/include/uapi/linux/fib_rules.h \
	linux/include/uapi/linux/fiemap.h \
	linux/include/uapi/linux/fs.h \
	linux/include/uapi/linux/fscrypt.h \
	linux/include/uapi/linux/gpio.h \
	linux/include/uapi/linux/hiddev.h \
	linux/include/uapi/linux/if_addr.h \
	linux/include/uapi/linux/if_bridge.h \
	linux/include/uapi/linux/if_link.h \
	linux/include/uapi/linux/in6.h \
	linux/include/uapi/linux/inet_diag.h \
	linux/include/uapi/linux/input-event-codes.h \
	linux/include/uapi/linux/input.h \
	linux/include/uapi/linux/io_uring.h \
	linux/include/uapi/linux/libc-compat.h \
	linux/include/uapi/linux/loop.h \
	linux/include/uapi/linux/mmtimer.h \
	linux/include/uapi/linux/mount.h \
	linux/include/uapi/linux/neighbour.h \
	linux/include/uapi/linux/netlink.h \
	linux/include/uapi/linux/netlink_diag.h \
	linux/include/uapi/linux/openat2.h \
	linux/include/uapi/linux/packet_diag.h \
	linux/include/uapi/linux/rtnetlink.h \
	linux/include/uapi/linux/smc_diag.h \
	linux/include/uapi/linux/sock_diag.h \
	linux/include/uapi/linux/socket.h \
	linux/include/uapi/linux/tee.h \
	linux/include/uapi/linux/types.h \
	linux/include/uapi/linux/unix_diag.h \
	linux/include/uapi/linux/v4l2-common.h \
	linux/include/uapi/linux/v4l2-controls.h \
	linux/include/uapi/linux/videodev2.h \
	linux/include/uapi/rdma/ib_user_verbs.h \
	# end of EXTRA_DIST

dist-hook:
	$(LN_S) asm-generic $(distdir)/linux/include/uapi/asm
