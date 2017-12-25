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
print_KVM_RUN_MORE(const int fd, const char *const dev, const char *str,
		   const struct kvm_run *run_before, const struct kvm_run *run_after)
{
	printf(" VCPU:0> " "{request_interrupt_window=%u, immediate_exit=%u",
	       run_before->request_interrupt_window, run_before->immediate_exit);
	printf(", exit_reason=%d /* %s */, "
	       "ready_for_interrupt_injection=%u, if_flag=%u, flags=%u",
	       run_after->exit_reason, str,
	       run_after->ready_for_interrupt_injection, run_after->if_flag,
	       run_after->flags);
	printf(", cr8=%#016llx", run_before->cr8);
	if (run_before->cr8 != run_after->cr8)
		printf(" => %#016llx", run_after->cr8);
	printf(", apic_base=%#016llx", run_before->apic_base);
	if (run_before->apic_base != run_after->apic_base)
		printf(" => %#016llx", run_after->apic_base);

	switch (run_after->exit_reason) {
	case KVM_EXIT_IO:
		printf(", {io={direction=%s"
		       ", size=%u, port=%#04x, count=%u, data_offset=%#016llx}}",
		       run_after->io.direction == KVM_EXIT_IO_IN
		       ? "KVM_EXIT_IO_IN" : "KVM_EXIT_IO_OUT",
		       run_after->io.size, run_after->io.port,
		       run_after->io.count, run_after->io.data_offset);
		break;
	case KVM_EXIT_MMIO:
		printf(", {mmio={phys_addr=%#016llx"
		       ", data=[%#0x, %#0x, %#0x, %#0x, %#0x, %#0x, %#0x, %#0x]"
		       ", len=%u, is_write=%u}}",
		       run_after->mmio.phys_addr,
		       run_after->mmio.data[0], run_after->mmio.data[1],
		       run_after->mmio.data[2], run_after->mmio.data[3],
		       run_after->mmio.data[4], run_after->mmio.data[5],
		       run_after->mmio.data[6], run_after->mmio.data[7],
		       run_after->mmio.len,
		       run_after->mmio.is_write);
		break;
	}

	puts("}");
}
#endif
