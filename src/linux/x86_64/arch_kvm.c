/*
 * Copyright (c) 2017-2021 The strace developers.
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
	tprint_struct_begin();
	PRINT_FIELD_X(*regs, rax);
	tprint_struct_next();
	if (abbrev(tcp)) {
		tprint_more_data_follows();
	} else {
		PRINT_FIELD_X(*regs, rbx);
		tprint_struct_next();
		PRINT_FIELD_X(*regs, rcx);
		tprint_struct_next();
		PRINT_FIELD_X(*regs, rdx);
		tprint_struct_next();
		PRINT_FIELD_X(*regs, rsi);
		tprint_struct_next();
		PRINT_FIELD_X(*regs, rdi);
	}
	tprint_struct_next();
	PRINT_FIELD_X(*regs, rsp);
	tprint_struct_next();
	PRINT_FIELD_X(*regs, rbp);
	tprint_struct_next();
	if (abbrev(tcp)) {
		tprint_more_data_follows();
	} else {
		PRINT_FIELD_X(*regs, r8);
		tprint_struct_next();
		PRINT_FIELD_X(*regs, r9);
		tprint_struct_next();
		PRINT_FIELD_X(*regs, r10);
		tprint_struct_next();
		PRINT_FIELD_X(*regs, r11);
		tprint_struct_next();
		PRINT_FIELD_X(*regs, r12);
		tprint_struct_next();
		PRINT_FIELD_X(*regs, r13);
		tprint_struct_next();
		PRINT_FIELD_X(*regs, r14);
		tprint_struct_next();
		PRINT_FIELD_X(*regs, r15);
	}
	tprint_struct_next();
	PRINT_FIELD_X(*regs, rip);

	/* TODO: we can decode this more */
	tprint_struct_next();
	PRINT_FIELD_X(*regs, rflags);

	tprint_struct_end();
}
#endif	/* HAVE_STRUCT_KVM_REGS */

#ifdef HAVE_STRUCT_KVM_SREGS
static void
kvm_ioctl_decode_regs_segment(const struct kvm_segment *const segment)
{
	tprint_struct_begin();
	PRINT_FIELD_X(*segment, base);
	tprint_struct_next();
	PRINT_FIELD_U(*segment, limit);
	tprint_struct_next();
	PRINT_FIELD_U(*segment, selector);
	tprint_struct_next();
	PRINT_FIELD_U(*segment, type);
	tprint_struct_next();
	PRINT_FIELD_U(*segment, present);
	tprint_struct_next();
	PRINT_FIELD_U(*segment, dpl);
	tprint_struct_next();
	PRINT_FIELD_U(*segment, db);
	tprint_struct_next();
	PRINT_FIELD_U(*segment, s);
	tprint_struct_next();
	PRINT_FIELD_U(*segment, l);
	tprint_struct_next();
	PRINT_FIELD_U(*segment, g);
	tprint_struct_next();
	PRINT_FIELD_U(*segment, avl);
	tprint_struct_end();
}

static void
kvm_ioctl_decode_regs_dtable(const struct kvm_dtable *const dtable)
{
	tprint_struct_begin();
	PRINT_FIELD_X(*dtable, base);
	tprint_struct_next();
	PRINT_FIELD_U(*dtable, limit);
	tprint_struct_end();
}

# define PRINT_FIELD_KVM_SREGS_STRUCT(where_, field_, type_)	\
	PRINT_FIELD_OBJ_PTR(where_, field_,			\
			    kvm_ioctl_decode_regs_ ## type_)

static void
arch_print_kvm_sregs(struct tcb *const tcp,
		     const kernel_ulong_t addr,
		     const struct kvm_sregs *const sregs)
{
	tprint_struct_begin();
	PRINT_FIELD_KVM_SREGS_STRUCT(*sregs, cs, segment);
	if (abbrev(tcp)) {
		tprint_struct_next();
		tprint_more_data_follows();
		tprint_struct_end();
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
	tprint_struct_next();
	PRINT_FIELD_U(*sregs, cr0);
	tprint_struct_next();
	PRINT_FIELD_U(*sregs, cr2);
	tprint_struct_next();
	PRINT_FIELD_U(*sregs, cr3);
	tprint_struct_next();
	PRINT_FIELD_U(*sregs, cr4);
	tprint_struct_next();
	PRINT_FIELD_U(*sregs, cr8);
	tprint_struct_next();
	PRINT_FIELD_U(*sregs, efer);
	tprint_struct_next();
	PRINT_FIELD_X(*sregs, apic_base);
	tprint_struct_next();
	PRINT_FIELD_X_ARRAY(*sregs, interrupt_bitmap);
	tprint_struct_end();
}
#endif	/* HAVE_STRUCT_KVM_SREGS */
