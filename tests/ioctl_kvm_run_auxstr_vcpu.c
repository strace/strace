/*
 * Copyright (c) 2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#define KVM_NO_CPUID_CALLBACK	\
	error_msg_and_skip("newer kernel (>= 4.16) is needed")

#include "ioctl_kvm_run_common.c"

#if need_print_KVM_RUN

#ifdef print_KVM_RUN_MORE
static void
print_KVM_RUN_MORE(const int fd, const char *const dev, const char *str,
		   const struct kvm_run *run_before, const struct kvm_run *run_after);
#endif

static void
print_KVM_RUN(const int fd, const char *const dev,
	      const struct kvm_run *run_before, const struct kvm_run *run_after)
{
	const char *str;

# define CASE_ENTRY(R) case R: str = #R; break
	switch (run_after->exit_reason) {
		CASE_ENTRY(KVM_EXIT_HLT);
		CASE_ENTRY(KVM_EXIT_IO);
		CASE_ENTRY(KVM_EXIT_MMIO);
		default: str = "???";
	}

	printf("ioctl(%d<%s>, KVM_RUN, 0) = 0 (%s)\n", fd, dev, str);

#ifdef print_KVM_RUN_MORE
	print_KVM_RUN_MORE(fd, dev, str, run_before, run_after);
#endif
}
#endif
