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
kvm_ioctl_decode_regs_segment(const struct kvm_segment *const segment)
{
	PRINT_FIELD_X("{", *segment, base);
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
kvm_ioctl_decode_regs_dtable(const struct kvm_dtable *const dtable)
{
	PRINT_FIELD_X("{", *dtable, base);
	PRINT_FIELD_U(", ", *dtable, limit);
	tprints("}");
}

# define PRINT_FIELD_KVM_SREGS_STRUCT(where_, field_, type_)	\
	PRINT_FIELD_OBJ_PTR("", where_, field_,			\
			    kvm_ioctl_decode_regs_ ## type_)

static void
arch_print_kvm_sregs(struct tcb *const tcp,
		     const kernel_ulong_t addr,
		     const struct kvm_sregs *const sregs)
{
	tprint_struct_begin();
	PRINT_FIELD_KVM_SREGS_STRUCT(*sregs, cs, segment);
	if (abbrev(tcp)) {
		tprints(", ...}");
		return;
	}

	tprint_struct_next();
	PRINT_FIELD_KVM_SREGS_STRUCT(*sregs, ds, segment);
	tprint_struct_next();
	PRINT_FIELD_KVM_SREGS_STRUCT(*sregs, es, segment);
	tprint_struct_next();
	PRINT_FIELD_KVM_SREGS_STRUCT(*sregs, fs, segment);
	tprint_struct_next();
	PRINT_FIELD_KVM_SREGS_STRUCT(*sregs, gs, segment);
	tprint_struct_next();
	PRINT_FIELD_KVM_SREGS_STRUCT(*sregs, ss, segment);
	tprint_struct_next();
	PRINT_FIELD_KVM_SREGS_STRUCT(*sregs, tr, segment);
	tprint_struct_next();
	PRINT_FIELD_KVM_SREGS_STRUCT(*sregs, ldt, segment);
	tprint_struct_next();
	PRINT_FIELD_KVM_SREGS_STRUCT(*sregs, gdt, dtable);
	tprint_struct_next();
	PRINT_FIELD_KVM_SREGS_STRUCT(*sregs, idt, dtable);
	PRINT_FIELD_U(", ", *sregs, cr0);
	PRINT_FIELD_U(", ", *sregs, cr2);
	PRINT_FIELD_U(", ", *sregs, cr3);
	PRINT_FIELD_U(", ", *sregs, cr4);
	PRINT_FIELD_U(", ", *sregs, cr8);
	PRINT_FIELD_U(", ", *sregs, efer);
	PRINT_FIELD_X(", ", *sregs, apic_base);
	tprint_struct_next();
	PRINT_FIELD_X_ARRAY(*sregs, interrupt_bitmap);
	tprints("}");
}
#endif	/* HAVE_STRUCT_KVM_SREGS */
