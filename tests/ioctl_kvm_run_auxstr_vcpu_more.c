/*
 * Copyright (c) 2025 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#define print_KVM_RUN_MORE print_KVM_RUN_MORE

#include "ioctl_kvm_run_auxstr_vcpu.c"

#if need_print_KVM_RUN
static void
print_kvm_run_more(const char *prefix, const char *reason_str, const struct kvm_run *run)
{
	printf(" %s {request_interrupt_window=%u"
# ifdef HAVE_STRUCT_KVM_RUN_IMMEDIATE_EXIT
	       ", immediate_exit=%u"
# endif
	       ", exit_reason=%d /* %s */, ready_for_interrupt_injection=%u"
	       ", if_flag=%u"
# ifdef HAVE_STRUCT_KVM_RUN_FLAGS
	       ", flags=%u"
# endif
	       ", cr8=%#016llx, apic_base=%#016llx",
	       prefix, run->request_interrupt_window,
# ifdef HAVE_STRUCT_KVM_RUN_IMMEDIATE_EXIT
	       run->immediate_exit,
# endif
	       run->exit_reason, reason_str, run->ready_for_interrupt_injection,
	       run->if_flag,
# ifdef HAVE_STRUCT_KVM_RUN_FLAGS
	       run->flags,
# endif
	       run->cr8, run->apic_base);

	switch (run->exit_reason) {
	case KVM_EXIT_IO:
		printf(", {io={direction=%s"
		       ", size=%u, port=%#04x, count=%u, data_offset=%#016llx",
		       run->io.direction == KVM_EXIT_IO_IN
		       ? "KVM_EXIT_IO_IN" : "KVM_EXIT_IO_OUT",
		       run->io.size, run->io.port,
		       run->io.count, run->io.data_offset);
		fputs(", data=[", stdout);
		for (unsigned long i = 0; i < (run->io.size * run->io.count); ++i)
			printf("%s%#x", i == 0?  "" : ", ",
			       ((unsigned char *)run + run->io.data_offset)[i]);
		fputs("]}}", stdout);
		break;
	case KVM_EXIT_MMIO:
		printf(", {mmio={phys_addr=%#016llx"
		       ", data=[%#0x, %#0x, %#0x, %#0x, %#0x, %#0x, %#0x, %#0x]"
		       ", len=%u, is_write=%u}}",
		       run->mmio.phys_addr,
		       run->mmio.data[0], run->mmio.data[1],
		       run->mmio.data[2], run->mmio.data[3],
		       run->mmio.data[4], run->mmio.data[5],
		       run->mmio.data[6], run->mmio.data[7],
		       run->mmio.len,
		       run->mmio.is_write);
		break;
	}

	puts("}");
}

static void
print_KVM_RUN_MORE(const int fd, const char *const dev,
		   const char *str_before, const struct kvm_run *run_before,
		   const char *str_after, const struct kvm_run *run_after)
{
	print_kvm_run_more("VCPU:0<", str_before, run_before);
	print_kvm_run_more("VCPU:0>", str_after, run_after);
}
#endif
