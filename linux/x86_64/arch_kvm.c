/*
 * Copyright (c) 2017-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifdef HAVE_STRUCT_KVM_REGS
static void
arch_print_kvm_regs(struct tcb *const tcp,
		    const kernel_ulong_t addr,
		    const struct kvm_regs *const regs)
{
	PRINT_FIELD_X("{", *regs, rax);
	if (abbrev(tcp))
		tprints(", ...");
	else {
		PRINT_FIELD_X(", ",  *regs, rbx);
		PRINT_FIELD_X(", ",  *regs, rcx);
		PRINT_FIELD_X(", ",  *regs, rdx);
		PRINT_FIELD_X(", ",  *regs, rsi);
		PRINT_FIELD_X(", ",  *regs, rdi);
	}
	PRINT_FIELD_X(", ",  *regs, rsp);
	PRINT_FIELD_X(", ",  *regs, rbp);
	if (abbrev(tcp))
		tprints(", ...");
	else {
		PRINT_FIELD_X(", ",  *regs, r8);
		PRINT_FIELD_X(", ",  *regs, r9);
		PRINT_FIELD_X(", ",  *regs, r10);
		PRINT_FIELD_X(", ",  *regs, r11);
		PRINT_FIELD_X(", ",  *regs, r12);
		PRINT_FIELD_X(", ",  *regs, r13);
		PRINT_FIELD_X(", ",  *regs, r14);
		PRINT_FIELD_X(", ",  *regs, r15);
	}
	PRINT_FIELD_X(", ",  *regs, rip);

	/* TODO: we can decode this more */
	PRINT_FIELD_X(", ",  *regs, rflags);

	tprints("}");
}
#endif	/* HAVE_STRUCT_KVM_REGS */

#ifdef HAVE_STRUCT_KVM_SREGS
static void
kvm_ioctl_decode_regs_segment(const char *prefix,
			      const struct kvm_segment *const segment)
{
	tprints(prefix);
	PRINT_FIELD_X("={", *segment, base);
	PRINT_FIELD_U(", ", *segment, limit);
	PRINT_FIELD_U(", ", *segment, selector);
	PRINT_FIELD_U(", ", *segment, type);
	PRINT_FIELD_U(", ", *segment, present);
	PRINT_FIELD_U(", ", *segment, dpl);
	PRINT_FIELD_U(", ", *segment, db);
	PRINT_FIELD_U(", ", *segment, s);
	PRINT_FIELD_U(", ", *segment, l);
	PRINT_FIELD_U(", ", *segment, g);
	PRINT_FIELD_U(", ", *segment, avl);
	tprints("}");
}

static void
kvm_ioctl_decode_regs_dtable(const char *prefix,
			     const struct kvm_dtable *const dtable)
{
	tprints(prefix);
	PRINT_FIELD_X("={", *dtable, base);
	PRINT_FIELD_U(", ", *dtable, limit);
	tprints("}");
}

# define PRINT_FIELD_KVM_SREGS_STRUCT(prefix_, where_, type_, field_)	\
	kvm_ioctl_decode_regs_ ## type_(prefix_ #field_, &(where_)->field_)

static void
arch_print_kvm_sregs(struct tcb *const tcp,
		     const kernel_ulong_t addr,
		     const struct kvm_sregs *const sregs)
{
	PRINT_FIELD_KVM_SREGS_STRUCT("{", sregs, segment, cs);
	if (abbrev(tcp)) {
		tprints(", ...}");
		return;
	}

	PRINT_FIELD_KVM_SREGS_STRUCT(", ", sregs, segment, ds);
	PRINT_FIELD_KVM_SREGS_STRUCT(", ", sregs, segment, es);
	PRINT_FIELD_KVM_SREGS_STRUCT(", ", sregs, segment, fs);
	PRINT_FIELD_KVM_SREGS_STRUCT(", ", sregs, segment, gs);
	PRINT_FIELD_KVM_SREGS_STRUCT(", ", sregs, segment, ss);
	PRINT_FIELD_KVM_SREGS_STRUCT(", ", sregs, segment, tr);
	PRINT_FIELD_KVM_SREGS_STRUCT(", ", sregs, segment, ldt);
	PRINT_FIELD_KVM_SREGS_STRUCT(", ", sregs, dtable, gdt);
	PRINT_FIELD_KVM_SREGS_STRUCT(", ", sregs, dtable, idt);
	PRINT_FIELD_U(", ", *sregs, cr0);
	PRINT_FIELD_U(", ", *sregs, cr2);
	PRINT_FIELD_U(", ", *sregs, cr3);
	PRINT_FIELD_U(", ", *sregs, cr4);
	PRINT_FIELD_U(", ", *sregs, cr8);
	PRINT_FIELD_U(", ", *sregs, efer);
	PRINT_FIELD_X(", ", *sregs, apic_base);
	PRINT_FIELD_X_ARRAY(", ", *sregs, interrupt_bitmap);
	tprints("}");
}
#endif	/* HAVE_STRUCT_KVM_SREGS */
