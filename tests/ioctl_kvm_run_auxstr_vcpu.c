/*
 * Copyright (c) 2018-2026 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#define KVM_NO_CPUID_CALLBACK	\
	error_msg_and_skip("newer kernel (>= 4.16) is needed")

#include "ioctl_kvm_run_common.c"

#if need_print_KVM_RUN

# ifdef print_KVM_RUN_MORE
static void
print_KVM_RUN_MORE(const int fd, const char *const dev,
		   const char *str_before, const struct kvm_run *run_before,
		   const char *str_after, const struct kvm_run *run_after);
# endif

static const char *
decode_exit_reason(int exit_reason)
{
# define CASE_ENTRY(R) case R: return #R
	switch (exit_reason) {
		CASE_ENTRY(KVM_EXIT_UNKNOWN);
		CASE_ENTRY(KVM_EXIT_HLT);
		CASE_ENTRY(KVM_EXIT_IO);
		CASE_ENTRY(KVM_EXIT_MMIO);
		default: return "???";
	}
}

static void
print_KVM_RUN(const int fd, const char *const dev,
	      const struct kvm_run *run_before, const struct kvm_run *run_after)
{
	const char *str_after = decode_exit_reason(run_after->exit_reason);
	printf("ioctl(%d<%s>, KVM_RUN, 0) = 0 (%s)\n", fd, dev, str_after);

# ifdef print_KVM_RUN_MORE
	const char *str_before = decode_exit_reason(run_before->exit_reason);
	print_KVM_RUN_MORE(fd, dev, str_before, run_before, str_after, run_after);
# endif
}
#endif
