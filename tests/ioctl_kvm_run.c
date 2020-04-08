/*
 * Copyright (c) 2017-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "ioctl_kvm_run_common.c"

#if need_print_KVM_RUN

static void
print_KVM_RUN(const int fd, const char *const dev, const unsigned int reason)
{
	printf("ioctl(%d<%s>, KVM_RUN, 0) = 0\n", fd, dev);
}

#endif
