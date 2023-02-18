/*
 * Copyright (c) 2023 Hongren (Zenithal) Zheng <i@zenithal.me>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include <linux/kfd_ioctl.h>

static int
print_amdkfd_ioc_get_version(struct tcb *const tcp,
	const kernel_ulong_t arg)
{
	struct kfd_ioctl_get_version_args args;

	if (entering(tcp))
		return 0;

	tprint_arg_next();
	if (umove_or_printaddr(tcp, arg, &args))
		return RVAL_IOCTL_DECODED;

	tprint_struct_begin();
	PRINT_FIELD_U(args, major_version);
	tprint_struct_next();
	PRINT_FIELD_U(args, minor_version);
	tprint_struct_end();

	return RVAL_IOCTL_DECODED;
}

#include "xlat/kfd_queue_type.h"

static int
print_amdkfd_ioc_create_queue(struct tcb *const tcp,
	const kernel_ulong_t arg)
{
	struct kfd_ioctl_create_queue_args args;

	if (entering(tcp))
		tprint_arg_next();
	else if (syserror(tcp))
		return RVAL_IOCTL_DECODED;
	else
		tprint_value_changed();

	if (umove_or_printaddr(tcp, arg, &args))
		return RVAL_IOCTL_DECODED;

	if (exiting(tcp)) {
		tprint_struct_begin();
		PRINT_FIELD_X(args, write_pointer_address);
		tprint_struct_next();
		PRINT_FIELD_X(args, read_pointer_address);
		tprint_struct_next();
		PRINT_FIELD_X(args, doorbell_offset);
		tprint_struct_next();
		PRINT_FIELD_X(args, queue_id);
		tprint_struct_end();
		return RVAL_IOCTL_DECODED;
	}

	/* entering */
	tprint_struct_begin();
	PRINT_FIELD_X(args, ring_base_address);
	tprint_struct_next();

	PRINT_FIELD_X(args, ring_size);
	tprint_struct_next();
	PRINT_FIELD_X(args, gpu_id);
	tprint_struct_next();
	PRINT_FIELD_XVAL(args, queue_type, kfd_queue_type, "KFD_IOC_QUEUE_TYPE_???");
	tprint_struct_next();
	PRINT_FIELD_U(args, queue_percentage);
	tprint_struct_next();
	PRINT_FIELD_U(args, queue_priority);
	tprint_struct_next();

	PRINT_FIELD_X(args, eop_buffer_address);
	tprint_struct_next();
	PRINT_FIELD_X(args, eop_buffer_size);
	tprint_struct_next();
	PRINT_FIELD_X(args, ctx_save_restore_address);
	tprint_struct_next();
	PRINT_FIELD_X(args, ctx_save_restore_size);
	tprint_struct_next();
	PRINT_FIELD_X(args, ctl_stack_size);
	tprint_struct_end();
	return 0;
}

static int
print_amdkfd_ioc_destroy_queue(struct tcb *const tcp,
	const kernel_ulong_t arg)
{
	struct kfd_ioctl_destroy_queue_args args;

	tprint_arg_next();
	if (!umove_or_printaddr(tcp, arg, &args)) {
		tprint_struct_begin();
		PRINT_FIELD_X(args, queue_id);
		tprint_struct_end();
	}

	return RVAL_IOCTL_DECODED;
}

#include "xlat/kfd_cache_policy.h"

static int
print_amdkfd_ioc_set_memory_policy(struct tcb *const tcp,
	const kernel_ulong_t arg)
{
	struct kfd_ioctl_set_memory_policy_args args;

	tprint_arg_next();
	if (!umove_or_printaddr(tcp, arg, &args)) {
		tprint_struct_begin();
		PRINT_FIELD_X(args, alternate_aperture_base);
		tprint_struct_next();
		PRINT_FIELD_X(args, alternate_aperture_size);
		tprint_struct_next();

		PRINT_FIELD_X(args, gpu_id);
		tprint_struct_next();
		PRINT_FIELD_XVAL(args, default_policy, kfd_cache_policy,
			"KFD_IOC_CACHE_POLICY_???");
		tprint_struct_next();
		PRINT_FIELD_XVAL(args, alternate_policy, kfd_cache_policy,
			"KFD_IOC_CACHE_POLICY_???");
		tprint_struct_end();
	}

	return RVAL_IOCTL_DECODED;
}

static int
print_amdkfd_ioc_get_clock_counters(struct tcb *const tcp,
	const kernel_ulong_t arg)
{
	struct kfd_ioctl_get_clock_counters_args args;

	if (entering(tcp))
		tprint_arg_next();
	else if (syserror(tcp))
		return RVAL_IOCTL_DECODED;
	else
		tprint_value_changed();

	if (umove_or_printaddr(tcp, arg, &args))
		return RVAL_IOCTL_DECODED;

	if (entering(tcp)) {
		tprint_struct_begin();
		PRINT_FIELD_X(args, gpu_id);
		tprint_struct_end();
		return 0;
	}

	/* exiting */
	tprint_struct_begin();
	PRINT_FIELD_X(args, gpu_clock_counter);
	tprint_struct_next();
	PRINT_FIELD_X(args, cpu_clock_counter);
	tprint_struct_next();
	PRINT_FIELD_X(args, system_clock_counter);
	tprint_struct_next();
	PRINT_FIELD_U(args, system_clock_freq);
	tprint_struct_end();
	return RVAL_IOCTL_DECODED;
}

#include "xlat/kfd_event_type.h"

static int
print_amdkfd_ioc_create_event(struct tcb *const tcp,
	const kernel_ulong_t arg)
{
	struct kfd_ioctl_create_event_args args;

	if (entering(tcp))
		tprint_arg_next();
	else if (syserror(tcp))
		return RVAL_IOCTL_DECODED;
	else
		tprint_value_changed();

	if (umove_or_printaddr(tcp, arg, &args))
		return RVAL_IOCTL_DECODED;

	if (exiting(tcp)) {
		tprint_struct_begin();
		PRINT_FIELD_X(args, event_page_offset);
		tprint_struct_next();
		PRINT_FIELD_X(args, event_trigger_data);
		tprint_struct_next();
		PRINT_FIELD_X(args, event_id);
		tprint_struct_next();
		PRINT_FIELD_X(args, event_slot_index);
		tprint_struct_end();
		return RVAL_IOCTL_DECODED;
	}

	/* entering */
	tprint_struct_begin();
	PRINT_FIELD_XVAL(args, event_type, kfd_event_type, "KFD_IOC_EVENT_TYPE_???");
	tprint_struct_next();
	PRINT_FIELD_X(args, auto_reset);
	tprint_struct_next();
	PRINT_FIELD_X(args, node_id);
	tprint_struct_end();
	return 0;
}

static int
print_amdkfd_ioc_destroy_event(struct tcb *const tcp,
	const kernel_ulong_t arg)
{
	struct kfd_ioctl_destroy_event_args args;

	tprint_arg_next();
	if (!umove_or_printaddr(tcp, arg, &args)) {
		tprint_struct_begin();
		PRINT_FIELD_X(args, event_id);
		tprint_struct_end();
	}

	return RVAL_IOCTL_DECODED;
}

static int
print_amdkfd_ioc_set_event(struct tcb *const tcp,
	const kernel_ulong_t arg)
{
	struct kfd_ioctl_set_event_args args;

	tprint_arg_next();
	if (!umove_or_printaddr(tcp, arg, &args)) {
		tprint_struct_begin();
		PRINT_FIELD_X(args, event_id);
		tprint_struct_end();
	}

	return RVAL_IOCTL_DECODED;
}

static int
print_amdkfd_ioc_set_scratch_backing_va(struct tcb *const tcp,
	const kernel_ulong_t arg)
{
	struct kfd_ioctl_set_scratch_backing_va_args args;

	tprint_arg_next();
	if (!umove_or_printaddr(tcp, arg, &args)) {
		tprint_struct_begin();
		PRINT_FIELD_X(args, va_addr);
		tprint_struct_next();
		PRINT_FIELD_X(args, gpu_id);
		tprint_struct_end();
	}

	return RVAL_IOCTL_DECODED;
}

static int
print_amdkfd_ioc_get_tile_config(struct tcb *const tcp,
	const kernel_ulong_t arg)
{
	struct kfd_ioctl_get_tile_config_args args;

	if (entering(tcp))
		tprint_arg_next();
	else if (syserror(tcp))
		return RVAL_IOCTL_DECODED;
	else
		tprint_value_changed();

	if (umove_or_printaddr(tcp, arg, &args))
		return RVAL_IOCTL_DECODED;

	if (exiting(tcp)) {
		tprint_struct_begin();
		PRINT_FIELD_U(args, num_tile_configs);
		tprint_struct_next();
		PRINT_FIELD_U(args, num_macro_tile_configs);
		tprint_struct_next();

		PRINT_FIELD_U(args, gb_addr_config);
		tprint_struct_next();
		PRINT_FIELD_U(args, num_banks);
		tprint_struct_next();
		PRINT_FIELD_U(args, num_ranks);
		tprint_struct_end();
		return RVAL_IOCTL_DECODED;
	}

	/* entering */
	tprint_struct_begin();
	PRINT_FIELD_X(args, tile_config_ptr);
	tprint_struct_next();
	PRINT_FIELD_X(args, macro_tile_config_ptr);
	tprint_struct_next();
	PRINT_FIELD_U(args, num_tile_configs);
	tprint_struct_next();
	PRINT_FIELD_U(args, num_macro_tile_configs);
	tprint_struct_next();

	PRINT_FIELD_X(args, gpu_id);
	tprint_struct_end();
	return 0;
}

static int
print_amdkfd_ioc_set_trap_handler(struct tcb *const tcp,
	const kernel_ulong_t arg)
{
	struct kfd_ioctl_set_trap_handler_args args;

	tprint_arg_next();
	if (!umove_or_printaddr(tcp, arg, &args)) {
		tprint_struct_begin();
		PRINT_FIELD_X(args, tba_addr);
		tprint_struct_next();
		PRINT_FIELD_X(args, tma_addr);
		tprint_struct_next();
		PRINT_FIELD_X(args, gpu_id);
		tprint_struct_end();
	}

	return RVAL_IOCTL_DECODED;
}

static int
print_amdkfd_ioc_get_process_apertures_new(struct tcb *const tcp,
	const kernel_ulong_t arg)
{
	struct kfd_ioctl_get_process_apertures_new_args args;

	if (entering(tcp))
		return 0;

	tprint_arg_next();
	if (umove_or_printaddr(tcp, arg, &args))
		return RVAL_IOCTL_DECODED;

	tprint_struct_begin();
	PRINT_FIELD_X(args, kfd_process_device_apertures_ptr); /* TODO: print array */
	tprint_struct_next();
	PRINT_FIELD_U(args, num_of_nodes);
	tprint_struct_end();

	return RVAL_IOCTL_DECODED;
}

static int
print_amdkfd_ioc_acquire_vm(struct tcb *const tcp,
	const kernel_ulong_t arg)
{
	struct kfd_ioctl_acquire_vm_args args;

	tprint_arg_next();
	if (!umove_or_printaddr(tcp, arg, &args)) {
		tprint_struct_begin();
		PRINT_FIELD_FD(args, drm_fd, tcp);
		tprint_struct_next();
		PRINT_FIELD_X(args, gpu_id);
		tprint_struct_end();
	}

	return RVAL_IOCTL_DECODED;
}

#include "xlat/kfd_alloc_mem_flags.h"

static int
print_amdkfd_ioc_alloc_memory_of_gpu(struct tcb *const tcp,
	const kernel_ulong_t arg)
{
	struct kfd_ioctl_alloc_memory_of_gpu_args args;

	if (entering(tcp))
		tprint_arg_next();
	else if (syserror(tcp))
		return RVAL_IOCTL_DECODED;
	else
		tprint_value_changed();

	if (umove_or_printaddr(tcp, arg, &args))
		return RVAL_IOCTL_DECODED;

	if (exiting(tcp)) {
		tprint_struct_begin();
		PRINT_FIELD_X(args, handle);
		tprint_struct_end();
		return RVAL_IOCTL_DECODED;
	}

	/* entering */
	tprint_struct_begin();
	PRINT_FIELD_X(args, va_addr);
	tprint_struct_next();
	PRINT_FIELD_X(args, size);
	tprint_struct_next();
	PRINT_FIELD_X(args, mmap_offset);
	tprint_struct_next();
	PRINT_FIELD_X(args, gpu_id);
	tprint_struct_next();
	PRINT_FIELD_FLAGS(args, flags, kfd_alloc_mem_flags, "KFD_IOC_ALLOC_MEM_FLAGS_???");
	tprint_struct_end();
	return 0;
}

static int
print_amdkfd_ioc_free_memory_of_gpu(struct tcb *const tcp,
	const kernel_ulong_t arg)
{
	struct kfd_ioctl_free_memory_of_gpu_args args;

	tprint_arg_next();
	if (!umove_or_printaddr(tcp, arg, &args)) {
		tprint_struct_begin();
		PRINT_FIELD_X(args, handle);
		tprint_struct_end();
	}

	return RVAL_IOCTL_DECODED;
}

static int
print_amdkfd_ioc_map_memory_to_gpu(struct tcb *const tcp,
	const kernel_ulong_t arg)
{
	struct kfd_ioctl_map_memory_to_gpu_args args;

	if (entering(tcp))
		tprint_arg_next();
	else if (syserror(tcp))
		return RVAL_IOCTL_DECODED;
	else
		tprint_value_changed();

	if (umove_or_printaddr(tcp, arg, &args))
		return RVAL_IOCTL_DECODED;

	if (exiting(tcp)) {
		tprint_struct_begin();
		PRINT_FIELD_U(args, n_success);
		tprint_struct_end();
		return RVAL_IOCTL_DECODED;
	}

	/* entering */
	tprint_struct_begin();
	PRINT_FIELD_X(args, handle);
	tprint_struct_next();
	PRINT_FIELD_X(args, device_ids_array_ptr);
	tprint_struct_next();
	PRINT_FIELD_U(args, n_devices);
	tprint_struct_next();
	PRINT_FIELD_U(args, n_success);
	tprint_struct_end();
	return 0;
}

static int
print_amdkfd_ioc_unmap_memory_from_gpu(struct tcb *const tcp,
	const kernel_ulong_t arg)
{
	struct kfd_ioctl_unmap_memory_from_gpu_args args;

	if (entering(tcp))
		tprint_arg_next();
	else if (syserror(tcp))
		return RVAL_IOCTL_DECODED;
	else
		tprint_value_changed();

	if (umove_or_printaddr(tcp, arg, &args))
		return RVAL_IOCTL_DECODED;

	if (exiting(tcp)) {
		tprint_struct_begin();
		PRINT_FIELD_U(args, n_success);
		tprint_struct_end();
		return RVAL_IOCTL_DECODED;
	}

	/* entering */
	tprint_struct_begin();
	PRINT_FIELD_X(args, handle);
	tprint_struct_next();
	PRINT_FIELD_X(args, device_ids_array_ptr);
	tprint_struct_next();
	PRINT_FIELD_U(args, n_devices);
	tprint_struct_next();
	PRINT_FIELD_U(args, n_success);
	tprint_struct_end();
	return 0;
}

static int
print_amdkfd_ioc_set_xnack_mode(struct tcb *const tcp,
	const kernel_ulong_t arg)
{
	struct kfd_ioctl_set_xnack_mode_args args;

	if (entering(tcp))
		tprint_arg_next();
	else if (syserror(tcp))
		return RVAL_IOCTL_DECODED;
	else
		tprint_value_changed();

	if (umove_or_printaddr(tcp, arg, &args))
		return RVAL_IOCTL_DECODED;

	if (exiting(tcp)) {
		tprint_struct_begin();
		PRINT_FIELD_X(args, xnack_enabled);
		tprint_struct_end();
		return RVAL_IOCTL_DECODED;
	}

	/* entering */
	tprint_struct_begin();
	PRINT_FIELD_X(args, xnack_enabled);
	tprint_struct_end();
	return 0;
}

int
kfd_ioctl(struct tcb *const tcp, const unsigned int code,
	const kernel_ulong_t arg)
{
	switch (code) {
	case AMDKFD_IOC_GET_VERSION:
		return print_amdkfd_ioc_get_version(tcp, arg);
	case AMDKFD_IOC_CREATE_QUEUE:
		return print_amdkfd_ioc_create_queue(tcp, arg);
	case AMDKFD_IOC_DESTROY_QUEUE:
		return print_amdkfd_ioc_destroy_queue(tcp, arg);
	case AMDKFD_IOC_SET_MEMORY_POLICY:
		return print_amdkfd_ioc_set_memory_policy(tcp, arg);
	case AMDKFD_IOC_GET_CLOCK_COUNTERS:
		return print_amdkfd_ioc_get_clock_counters(tcp, arg);
	/* TODO: AMDKFD_IOC_GET_PROCESS_APERTURES */
	/* TODO: AMDKFD_IOC_UPDATE_QUEUE */
	case AMDKFD_IOC_CREATE_EVENT:
		return print_amdkfd_ioc_create_event(tcp, arg);
	case AMDKFD_IOC_DESTROY_EVENT:
		return print_amdkfd_ioc_destroy_event(tcp, arg);
	case AMDKFD_IOC_SET_EVENT:
		return print_amdkfd_ioc_set_event(tcp, arg);
	/* TODO: AMDKFD_IOC_RESET_EVENT */
	/* TODO: AMDKFD_IOC_WAIT_EVENTS */
	/* TODO: DEPRECATED ioctl */
	case AMDKFD_IOC_SET_SCRATCH_BACKING_VA:
		return print_amdkfd_ioc_set_scratch_backing_va(tcp, arg);
	case AMDKFD_IOC_GET_TILE_CONFIG:
		return print_amdkfd_ioc_get_tile_config(tcp, arg);
	case AMDKFD_IOC_SET_TRAP_HANDLER:
		return print_amdkfd_ioc_set_trap_handler(tcp, arg);
	case AMDKFD_IOC_GET_PROCESS_APERTURES_NEW:
		return print_amdkfd_ioc_get_process_apertures_new(tcp, arg);
	case AMDKFD_IOC_ACQUIRE_VM:
		return print_amdkfd_ioc_acquire_vm(tcp, arg);
	case AMDKFD_IOC_ALLOC_MEMORY_OF_GPU:
		return print_amdkfd_ioc_alloc_memory_of_gpu(tcp, arg);
	case AMDKFD_IOC_FREE_MEMORY_OF_GPU:
		return print_amdkfd_ioc_free_memory_of_gpu(tcp, arg);
	case AMDKFD_IOC_MAP_MEMORY_TO_GPU:
		return print_amdkfd_ioc_map_memory_to_gpu(tcp, arg);
	case AMDKFD_IOC_UNMAP_MEMORY_FROM_GPU:
		return print_amdkfd_ioc_unmap_memory_from_gpu(tcp, arg);
	/* TODO: remaining ioctls */
	case AMDKFD_IOC_SET_XNACK_MODE:
		return print_amdkfd_ioc_set_xnack_mode(tcp, arg);
	/* TODO: remaining ioctls */
	}
	return RVAL_DECODED;
}
