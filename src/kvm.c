/*
 * Support for decoding of KVM_* ioctl commands.
 *
 * Copyright (c) 2017 Masatake YAMATO <yamato@redhat.com>
 * Copyright (c) 2017 Red Hat, Inc.
 * Copyright (c) 2017-2026 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#ifdef HAVE_LINUX_KVM_H
# include <linux/kvm.h>
# include "arch_kvm.c"
# include "xmalloc.h"
# include "mmap_cache.h"
# include "xlat/kvm_exit_io.h"

struct vcpu_info {
	struct vcpu_info *next;
	int fd;
	int cpuid;
	unsigned long mmap_addr;
	unsigned long mmap_len;
	bool resolved;
};

enum decode_kvm_run_structure_modes decode_kvm_run_structure;

static struct vcpu_info *
vcpu_find(struct tcb *const tcp, int fd)
{
	for (struct vcpu_info *vcpu_info = tcp->vcpu_info_list;
	     vcpu_info;
	     vcpu_info = vcpu_info->next)
		if (vcpu_info->fd == fd)
			return vcpu_info;

	return NULL;
}

static struct vcpu_info *
vcpu_alloc(struct tcb *const tcp, int fd, int cpuid)
{
	struct vcpu_info *vcpu_info = xzalloc(sizeof(*vcpu_info));

	vcpu_info->fd = fd;
	vcpu_info->cpuid = cpuid;

	vcpu_info->next = tcp->vcpu_info_list;
	tcp->vcpu_info_list = vcpu_info;

	return vcpu_info;
}

void
kvm_vcpu_info_free(struct tcb *tcp)
{
	struct vcpu_info *next;

	for (struct vcpu_info *head = tcp->vcpu_info_list; head; head = next) {
		next = head->next;
		free(head);
	}

	tcp->vcpu_info_list = NULL;
	tcp->vcpu_leaving = NULL;
	if (tcp->vcpu_entering) {
		free(tcp->vcpu_entering);
		tcp->vcpu_entering = NULL;
	}
}

static void
vcpu_register(struct tcb *const tcp, int fd, int cpuid)
{
	if (fd < 0)
		return;

	struct vcpu_info *vcpu_info = vcpu_find(tcp, fd);

	if (!vcpu_info) {
		vcpu_alloc(tcp, fd, cpuid);
	} else if (vcpu_info->cpuid != cpuid) {
		vcpu_info->cpuid = cpuid;
		vcpu_info->resolved = false;
	}
}

static bool
is_map_for_file(struct mmap_cache_entry_t *map_info, void *data)
{
	/* major version for anon inode may be given in get_anon_bdev()
	 * in linux kernel.
	 *
	 *	*p = MKDEV(0, dev & MINORMASK);
	 *-----------------^
	 */
	return map_info->binary_filename &&
		map_info->major == 0 &&
		strcmp(map_info->binary_filename, data) == 0;
}

static unsigned long
map_len(struct mmap_cache_entry_t *map_info)
{
	return map_info->start_addr < map_info->end_addr
		? map_info->end_addr - map_info->start_addr
		: 0;
}

# define VCPU_DENTRY_PREFIX "anon_inode:kvm-vcpu:"

static struct vcpu_info*
vcpu_get_info(struct tcb *const tcp, int fd)
{
	struct vcpu_info *vcpu_info = vcpu_find(tcp, fd);
	struct mmap_cache_entry_t *map_info;
	const char *cpuid_str;

	enum mmap_cache_rebuild_result mc_stat =
		mmap_cache_rebuild_if_invalid(tcp, __func__);
	if (mc_stat == MMAP_CACHE_REBUILD_NOCACHE)
		return NULL;

	if (vcpu_info && vcpu_info->resolved) {
		if (mc_stat == MMAP_CACHE_REBUILD_READY)
			return vcpu_info;
		else {
			map_info = mmap_cache_search(tcp, vcpu_info->mmap_addr);
			if (map_info) {
				cpuid_str =
					STR_STRIP_PREFIX(map_info->binary_filename,
							 VCPU_DENTRY_PREFIX);
				if (cpuid_str != map_info->binary_filename) {
					int cpuid = string_to_uint(cpuid_str);
					if (cpuid < 0)
						return NULL;
					if (vcpu_info->cpuid == cpuid)
						return vcpu_info;
				}
			}

			/* The vcpu vma may be mremap'ed. */
			vcpu_info->resolved = false;
		}
	}

	/* Slow path: !vcpu_info || !vcpu_info->resolved */
	char path[PATH_MAX + 1];
	cpuid_str = path;
	if (getfdpath(tcp, fd, path, sizeof(path)) >= 0)
		cpuid_str = STR_STRIP_PREFIX(path, VCPU_DENTRY_PREFIX);
	if (cpuid_str == path)
		map_info = NULL;
	else
		map_info = mmap_cache_search_custom(tcp, is_map_for_file, path);

	if (map_info) {
		int cpuid = string_to_uint(cpuid_str);
		if (cpuid < 0)
			return NULL;
		if (!vcpu_info)
			vcpu_info = vcpu_alloc(tcp, fd, cpuid);
		else if (vcpu_info->cpuid != cpuid)
			vcpu_info->cpuid = cpuid;
		vcpu_info->mmap_addr = map_info->start_addr;
		vcpu_info->mmap_len  = map_len(map_info);
		vcpu_info->resolved  = true;
		return vcpu_info;
	}

	return NULL;
}

static int
kvm_ioctl_create_vcpu(struct tcb *const tcp, const kernel_ulong_t arg)
{
	uint32_t cpuid = arg;

	if (entering(tcp)) {
		tprints_arg_next_name("argp");
		PRINT_VAL_U(cpuid);
		if (decode_kvm_run_structure)
			return 0;
	} else if (!syserror(tcp)) {
		vcpu_register(tcp, tcp->u_rval, cpuid);
	}

	return RVAL_IOCTL_DECODED | RVAL_FD;
}

# ifdef HAVE_STRUCT_KVM_USERSPACE_MEMORY_REGION
#  include "xlat/kvm_mem_flags.h"
static int
kvm_ioctl_set_user_memory_region(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct kvm_userspace_memory_region u_memory_region;

	tprints_arg_next_name("argp");
	if (umove_or_printaddr(tcp, arg, &u_memory_region))
		return RVAL_IOCTL_DECODED;

	tprint_struct_begin();
	PRINT_FIELD_U(u_memory_region, slot);
	tprint_struct_next();
	PRINT_FIELD_FLAGS(u_memory_region, flags, kvm_mem_flags, "KVM_MEM_???");
	tprint_struct_next();
	PRINT_FIELD_X(u_memory_region, guest_phys_addr);
	tprint_struct_next();
	PRINT_FIELD_U(u_memory_region, memory_size);
	tprint_struct_next();
	PRINT_FIELD_X(u_memory_region, userspace_addr);
	tprint_struct_end();

	return RVAL_IOCTL_DECODED;
}
# endif /* HAVE_STRUCT_KVM_USERSPACE_MEMORY_REGION */

# ifdef HAVE_STRUCT_KVM_REGS
static int
kvm_ioctl_decode_regs(struct tcb *const tcp, const unsigned int code,
		      const kernel_ulong_t arg)
{
	struct kvm_regs regs;

	if (code == KVM_GET_REGS && entering(tcp))
		return 0;

	tprints_arg_next_name("argp");
	if (!umove_or_printaddr(tcp, arg, &regs))
		arch_print_kvm_regs(tcp, arg, &regs);

	return RVAL_IOCTL_DECODED;
}
# endif /* HAVE_STRUCT_KVM_REGS */

# ifdef HAVE_STRUCT_KVM_CPUID2
#  include "xlat/kvm_cpuid_flags.h"
static bool
print_kvm_cpuid_entry(struct tcb *const tcp,
		      void* elem_buf, size_t elem_size, void* data)
{
	const struct kvm_cpuid_entry2 *entry = elem_buf;
	tprint_struct_begin();
	PRINT_FIELD_X(*entry, function);
	tprint_struct_next();
	PRINT_FIELD_X(*entry, index);
	tprint_struct_next();
	PRINT_FIELD_FLAGS(*entry, flags, kvm_cpuid_flags, "KVM_CPUID_FLAG_???");
	tprint_struct_next();
	PRINT_FIELD_X(*entry, eax);
	tprint_struct_next();
	PRINT_FIELD_X(*entry, ebx);
	tprint_struct_next();
	PRINT_FIELD_X(*entry, ecx);
	tprint_struct_next();
	PRINT_FIELD_X(*entry, edx);
	tprint_struct_end();

	return true;
}

static int
kvm_ioctl_decode_cpuid2(struct tcb *const tcp, const unsigned int code,
			const kernel_ulong_t arg)
{
	struct kvm_cpuid2 cpuid;

	if (entering(tcp) && (code == KVM_GET_SUPPORTED_CPUID
#  ifdef KVM_GET_EMULATED_CPUID
			      || code == KVM_GET_EMULATED_CPUID
#  endif
			     ))
		return 0;

	tprints_arg_next_name("argp");
	if (!umove_or_printaddr(tcp, arg, &cpuid)) {
		tprint_struct_begin();
		PRINT_FIELD_U(cpuid, nent);

		tprint_struct_next();
		tprints_field_name("entries");
		if (abbrev(tcp)) {
			tprint_array_begin();
			if (cpuid.nent)
				tprint_more_data_follows();
			tprint_array_end();

		} else {
			struct kvm_cpuid_entry2 entry;
			print_array(tcp, arg + sizeof(cpuid), cpuid.nent,
				    &entry, sizeof(entry), tfetch_mem,
				    print_kvm_cpuid_entry, NULL);
		}
		tprint_struct_end();
	}

	return RVAL_IOCTL_DECODED;
}
# endif /* HAVE_STRUCT_KVM_CPUID2 */

# ifdef HAVE_STRUCT_KVM_SREGS
static int
kvm_ioctl_decode_sregs(struct tcb *const tcp, const unsigned int code,
		       const kernel_ulong_t arg)
{
	struct kvm_sregs sregs;

	if (code == KVM_GET_SREGS && entering(tcp))
		return 0;

	tprints_arg_next_name("argp");
	if (!umove_or_printaddr(tcp, arg, &sregs))
		arch_print_kvm_sregs(tcp, arg, &sregs);

	return RVAL_IOCTL_DECODED;
}
# endif /* HAVE_STRUCT_KVM_SREGS */

# include "xlat/kvm_cap.h"
static int
kvm_ioctl_decode_check_extension(struct tcb *const tcp, const unsigned int code,
				 const kernel_ulong_t arg)
{
	tprints_arg_next_name("argp");
	printxval64(kvm_cap, arg, "KVM_CAP_???");
	return RVAL_IOCTL_DECODED;
}

static bool
kvm_ioctl_run_umove(struct tcb *const tcp, struct vcpu_info *info,
		    struct kvm_run *buf)
{
	if (info->mmap_len < sizeof(*buf))
		return false;

	if (umove(tcp, info->mmap_addr, buf) < 0)
		return false;

	return true;
}

# include "xlat/kvm_exit_reason.h"
static void
kvm_ioctl_run_attach_auxstr(struct tcb *const tcp,
			    struct vcpu_info *info)

{
	static struct kvm_run vcpu_run_struct;

	if (!kvm_ioctl_run_umove(tcp, info, &vcpu_run_struct))
		return;

	tcp->auxstr = xlookup(kvm_exit_reason, vcpu_run_struct.exit_reason);
	if (!tcp->auxstr)
		tcp->auxstr = "KVM_EXIT_???";
	if (decode_kvm_run_structure > DECODE_KVM_RUN_STRUCTURE_EXIT_REASON)
		tcp->vcpu_leaving = &vcpu_run_struct;
}

static int
kvm_ioctl_decode_run(struct tcb *const tcp)
{

	if (entering(tcp)) {
		if (decode_kvm_run_structure > DECODE_KVM_RUN_STRUCTURE_EXIT_REASON) {
			int fd = tcp->u_arg[0];
			struct vcpu_info *info = vcpu_get_info(tcp, fd);
			if (info) {
				struct kvm_run tmp;
				if (kvm_ioctl_run_umove(tcp, info, &tmp)) {
					tcp->vcpu_entering
						= malloc(sizeof(*tcp->vcpu_entering));
					if (tcp->vcpu_entering)
						memcpy(tcp->vcpu_entering, &tmp, sizeof(tmp));
				}
			}
		}
		return 0;
	}

	int r = RVAL_DECODED;

	if (syserror(tcp))
		return r;

	if (decode_kvm_run_structure) {
		tcp->auxstr = NULL;
		int fd = tcp->u_arg[0];
		struct vcpu_info *info = vcpu_get_info(tcp, fd);

		if (info) {
			kvm_ioctl_run_attach_auxstr(tcp, info);
			if (tcp->auxstr)
				r |= RVAL_STR;
		}
	}

	return r;
}

int
kvm_ioctl(struct tcb *const tcp, const unsigned int code, const kernel_ulong_t arg)
{
	switch (code) {
	case KVM_CREATE_VCPU:
		return kvm_ioctl_create_vcpu(tcp, arg);

# ifdef HAVE_STRUCT_KVM_USERSPACE_MEMORY_REGION
	case KVM_SET_USER_MEMORY_REGION:
		return kvm_ioctl_set_user_memory_region(tcp, arg);
# endif

# ifdef HAVE_STRUCT_KVM_REGS
	case KVM_SET_REGS:
	case KVM_GET_REGS:
		return kvm_ioctl_decode_regs(tcp, code, arg);
# endif

# ifdef HAVE_STRUCT_KVM_SREGS
	case KVM_SET_SREGS:
	case KVM_GET_SREGS:
		return kvm_ioctl_decode_sregs(tcp, code, arg);
# endif

# ifdef HAVE_STRUCT_KVM_CPUID2
       case KVM_SET_CPUID2:
       case KVM_GET_SUPPORTED_CPUID:
#  ifdef KVM_GET_EMULATED_CPUID
       case KVM_GET_EMULATED_CPUID:
#  endif
               return kvm_ioctl_decode_cpuid2(tcp, code, arg);
# endif

	case KVM_CHECK_EXTENSION:
		return kvm_ioctl_decode_check_extension(tcp, code, arg);

	case KVM_CREATE_VM:
		return RVAL_DECODED | RVAL_FD;

	case KVM_RUN:
		tcp->flags |= TCP_AFTER_KVM_RUN;
		return kvm_ioctl_decode_run(tcp);

	case KVM_GET_VCPU_MMAP_SIZE:
	case KVM_GET_API_VERSION:
	default:
		return RVAL_DECODED;
	}
}

static void
kvm_run_structure_decode_io(struct tcb *tcp,
			      struct kvm_run *state, struct vcpu_info *info)
{
	tprints_field_name("io");
	tprint_struct_begin();
	PRINT_FIELD_XVAL(state->io, direction, kvm_exit_io, "KVM_EXIT_IO_???");
	tprint_struct_next();
	PRINT_FIELD_U(state->io, size);
	tprint_struct_next();
	PRINT_FIELD_0X(state->io, port);
	tprint_struct_next();
	PRINT_FIELD_U(state->io, count);
	tprint_struct_next();
	PRINT_FIELD_0X(state->io, data_offset);

	if (info) {
		unsigned long data = info->mmap_addr + state->io.data_offset;
		unsigned long data_len = state->io.size * state->io.count;
		unsigned char buf;

		tprint_struct_next();
		tprints_field_name("data");
		print_array(tcp, data, data_len, &buf, sizeof(buf), tfetch_mem,
			    print_xint_array_member, NULL);
	}

	tprint_struct_end();
}

static void
kvm_run_structure_decode_mmio(struct tcb *tcp,
			      struct kvm_run *state)
{
	tprints_field_name("mmio");
	tprint_struct_begin();
	PRINT_FIELD_0X(state->mmio, phys_addr);
	tprint_struct_next();
	PRINT_FIELD_ARRAY(state->mmio, data, tcp, print_xint_array_member);
	tprint_struct_next();
	PRINT_FIELD_U(state->mmio, len);
	tprint_struct_next();
	PRINT_FIELD_U(state->mmio, is_write);
	tprint_struct_end();
}

static void
kvm_run_structure_decode_main(struct tcb *tcp,
			      struct kvm_run *state,
			      struct vcpu_info *info,
			      const char *auxstr)
{
	tprint_struct_begin();

	PRINT_FIELD_U(*state, request_interrupt_window);
	tprint_struct_next();
# ifdef HAVE_STRUCT_KVM_RUN_IMMEDIATE_EXIT
	PRINT_FIELD_U(*state, immediate_exit);
	tprint_struct_next();
# endif

	PRINT_FIELD_U(*state, exit_reason);
	if (auxstr)
		tprints_comment(auxstr);
	tprint_struct_next();
	PRINT_FIELD_U(*state, ready_for_interrupt_injection);
	tprint_struct_next();
	PRINT_FIELD_U(*state, if_flag);
	tprint_struct_next();
# ifdef	HAVE_STRUCT_KVM_RUN_FLAGS
	PRINT_FIELD_X(*state, flags);
	tprint_struct_next();
# endif

	PRINT_FIELD_0X(*state, cr8);
	tprint_struct_next();
	PRINT_FIELD_0X(*state, apic_base);

# define DECODE_UNION(...)			\
	do {					\
		tprint_struct_next();		\
		tprint_union_begin();		\
		__VA_ARGS__;			\
		tprint_union_end();		\
	} while (0)

	switch (state->exit_reason) {
	case KVM_EXIT_IO:
		DECODE_UNION(kvm_run_structure_decode_io(tcp, state, info));
		break;
	case KVM_EXIT_MMIO:
		DECODE_UNION(kvm_run_structure_decode_mmio(tcp, state));
		break;
	}

	tprint_struct_end();
	tprint_newline();
}

void
kvm_run_structure_decode(struct tcb * tcp)
{
	int fd = tcp->u_arg[0];
	struct vcpu_info * info = vcpu_get_info(tcp, fd);

	/* Before */
	if (tcp->vcpu_entering) {
		if (info)
			tprintf_string(" VCPU:%d< ", info->cpuid);
		else
			tprints_string(" VCPU< ");
		const char *auxstr = xlookup(kvm_exit_reason,
					     tcp->vcpu_entering->exit_reason);
		kvm_run_structure_decode_main(tcp, tcp->vcpu_entering, info,
					      auxstr);
	}

	if (syserror(tcp)) {
		tcp->vcpu_leaving = NULL;
		return;
	}

	/* After */
	if (tcp->vcpu_leaving) {
		if (info)
			tprintf_string(" VCPU:%d> ", info->cpuid);
		else
			tprints_string(" VCPU> ");
		kvm_run_structure_decode_main(tcp, tcp->vcpu_leaving, info,
					      tcp->auxstr);
	}

	tcp->vcpu_leaving = NULL;
}

void
kvm_run_structure_decoder_init(enum decode_kvm_run_structure_modes mode)
{
	decode_kvm_run_structure = mode;
	mmap_cache_enable();
}

#endif /* HAVE_LINUX_KVM_H */
