/*
 * Check AMDKFD_* ioctl decoding
 *
 * Copyright (C) 2022 Hongren (Zenithal) Zheng <i@zenithal.me>
 */

#include "tests.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <linux/kfd_ioctl.h>

static const char *errstr;

static long
do_ioctl(kernel_ulong_t cmd, const void *arg)
{
	long rc = ioctl(-1, cmd, arg);

	errstr = sprintrc(rc);

#ifdef INJECT_RETVAL
	if (rc != INJECT_RETVAL)
		error_msg_and_fail("Got a return value of %ld != %ld",
				   rc, (long) INJECT_RETVAL);

	static char inj_errstr[4096];

	snprintf(inj_errstr, sizeof(inj_errstr), "%s (INJECTED)", errstr);
	errstr = inj_errstr;
#endif

	return rc;
}

#ifdef INJECT_RETVAL
static void
test_print_amdkfd_ioc_get_version(void)
{
	do_ioctl(AMDKFD_IOC_GET_VERSION, 0);
	printf("ioctl(-1, AMDKFD_IOC_GET_VERSION, NULL) = %s\n", errstr);

	TAIL_ALLOC_OBJECT_CONST_PTR(struct kfd_ioctl_get_version_args, args);

	do_ioctl(AMDKFD_IOC_GET_VERSION, (char *) args + 1);
	printf("ioctl(-1, AMDKFD_IOC_GET_VERSION, %p) = %s\n",
	       (char *) args + 1, errstr);

	args->major_version = 1;
	args->minor_version = 8;

	do_ioctl(AMDKFD_IOC_GET_VERSION, args);
	printf("ioctl(-1, AMDKFD_IOC_GET_VERSION, {"
	       "major_version=1, "
	       "minor_version=8"
	       "}) = %s\n", errstr);

}
#endif

#ifdef INJECT_RETVAL
static void
test_print_amdkfd_ioc_get_process_apertures_new(void)
{
	do_ioctl(AMDKFD_IOC_GET_PROCESS_APERTURES_NEW, 0);
	printf("ioctl(-1, AMDKFD_IOC_GET_PROCESS_APERTURES_NEW, NULL) = %s\n", errstr);

	TAIL_ALLOC_OBJECT_CONST_PTR(struct kfd_ioctl_get_process_apertures_new_args, args);

	do_ioctl(AMDKFD_IOC_GET_PROCESS_APERTURES_NEW, (char *) args + 1);
	printf("ioctl(-1, AMDKFD_IOC_GET_PROCESS_APERTURES_NEW, %p) = %s\n",
	       (char *) args + 1, errstr);

	args->kfd_process_device_apertures_ptr = 0x55c10af54f30;
	args->num_of_nodes = 1;

	do_ioctl(AMDKFD_IOC_GET_PROCESS_APERTURES_NEW, args);
	printf("ioctl(-1, AMDKFD_IOC_GET_PROCESS_APERTURES_NEW, {"
	       "kfd_process_device_apertures_ptr=0x55c10af54f30, "
	       "num_of_nodes=1"
	       "}) = %s\n", errstr);
}
#endif

static void
test_print_amdkfd_ioc_acquire_vm(void)
{
	do_ioctl(AMDKFD_IOC_ACQUIRE_VM, 0);
	printf("ioctl(-1, AMDKFD_IOC_ACQUIRE_VM, NULL) = %s\n", errstr);

	TAIL_ALLOC_OBJECT_CONST_PTR(struct kfd_ioctl_acquire_vm_args, args);

	do_ioctl(AMDKFD_IOC_ACQUIRE_VM, (char *) args + 1);
	printf("ioctl(-1, AMDKFD_IOC_ACQUIRE_VM, %p) = %s\n",
	       (char *) args + 1, errstr);

	args->drm_fd = 6;
	args->gpu_id = 0x99c7;

	do_ioctl(AMDKFD_IOC_ACQUIRE_VM, args);
	printf("ioctl(-1, AMDKFD_IOC_ACQUIRE_VM, {"
	       "drm_fd=6, "
	       "gpu_id=0x99c7"
	       "}) = %s\n", errstr);
}

static void
test_print_amdkfd_ioc_set_memory_policy(void)
{
	do_ioctl(AMDKFD_IOC_SET_MEMORY_POLICY, 0);
	printf("ioctl(-1, AMDKFD_IOC_SET_MEMORY_POLICY, NULL) = %s\n", errstr);

	TAIL_ALLOC_OBJECT_CONST_PTR(struct kfd_ioctl_set_memory_policy_args, args);

	do_ioctl(AMDKFD_IOC_SET_MEMORY_POLICY, (char *) args + 1);
	printf("ioctl(-1, AMDKFD_IOC_SET_MEMORY_POLICY, %p) = %s\n",
	       (char *) args + 1, errstr);

	args->alternate_aperture_base = 0x200000;
	args->alternate_aperture_size = 0x7fffffe00000;
	args->gpu_id = 0x99c7;
	args->default_policy = KFD_IOC_CACHE_POLICY_NONCOHERENT;
	args->alternate_policy = KFD_IOC_CACHE_POLICY_COHERENT;

	do_ioctl(AMDKFD_IOC_SET_MEMORY_POLICY, args);
	printf("ioctl(-1, AMDKFD_IOC_SET_MEMORY_POLICY, {"
	       "alternate_aperture_base=0x200000, "
	       "alternate_aperture_size=0x7fffffe00000, "
	       "gpu_id=0x99c7, "
	       "default_policy=KFD_IOC_CACHE_POLICY_NONCOHERENT, "
	       "alternate_policy=KFD_IOC_CACHE_POLICY_COHERENT"
	       "}) = %s\n", errstr);
}

static void
test_print_amdkfd_ioc_alloc_memory_of_gpu(void)
{
	long rc;

	do_ioctl(AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, 0);
	printf("ioctl(-1, AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, NULL) = %s\n", errstr);

	TAIL_ALLOC_OBJECT_CONST_PTR(struct kfd_ioctl_alloc_memory_of_gpu_args, args);

	do_ioctl(AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, (char *) args + 1);
	printf("ioctl(-1, AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, %p) = %s\n",
	       (char *) args + 1, errstr);

	args->va_addr = 0x7fee58273000;
	args->size = 0x1000;
	args->mmap_offset = 0;
	args->gpu_id = 0x99c7;
	args->flags = KFD_IOC_ALLOC_MEM_FLAGS_MMIO_REMAP|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE|KFD_IOC_ALLOC_MEM_FLAGS_COHERENT;

	args->handle = 0x99c700000000;

	rc = do_ioctl(AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, args);
	printf("ioctl(-1, AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, {"
	       "va_addr=0x7fee58273000, "
	       "size=0x1000, "
	       "mmap_offset=0, "
	       "gpu_id=0x99c7, "
	       "flags=KFD_IOC_ALLOC_MEM_FLAGS_MMIO_REMAP|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE|KFD_IOC_ALLOC_MEM_FLAGS_COHERENT"
	       "}");
	if (rc >= 0)
		printf(" => {"
		       "handle=0x99c700000000"
		       "}");
	printf(") = %s\n", errstr);

	args->va_addr = 0x7fee58273000;
	args->size = 0x1000;
	args->mmap_offset = 0;
	args->gpu_id = 0x99c7;
	args->flags = KFD_IOC_ALLOC_MEM_FLAGS_MMIO_REMAP|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE|KFD_IOC_ALLOC_MEM_FLAGS_COHERENT;

	args->handle = 0x99c700000000;

	rc = do_ioctl(AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, args);
	printf("ioctl(-1, AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, {"
	       "va_addr=0x7fee58273000, "
	       "size=0x1000, "
	       "mmap_offset=0, "
	       "gpu_id=0x99c7, "
	       "flags=KFD_IOC_ALLOC_MEM_FLAGS_MMIO_REMAP|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE|KFD_IOC_ALLOC_MEM_FLAGS_COHERENT"
	       "}");
	if (rc >= 0)
		printf(" => {"
		       "handle=0x99c700000000"
		       "}");
	printf(") = %s\n", errstr);

	args->va_addr = 0x7fee579b0000;
	args->size = 0x8000;
	args->mmap_offset = 0;
	args->gpu_id = 0x99c7;
	args->flags = KFD_IOC_ALLOC_MEM_FLAGS_GTT|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE|KFD_IOC_ALLOC_MEM_FLAGS_COHERENT|KFD_IOC_ALLOC_MEM_FLAGS_UNCACHED;

	args->handle = 0x99c700000001;

	rc = do_ioctl(AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, args);
	printf("ioctl(-1, AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, {"
	       "va_addr=0x7fee579b0000, "
	       "size=0x8000, "
	       "mmap_offset=0, "
	       "gpu_id=0x99c7, "
	       "flags=KFD_IOC_ALLOC_MEM_FLAGS_GTT|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE|KFD_IOC_ALLOC_MEM_FLAGS_COHERENT|KFD_IOC_ALLOC_MEM_FLAGS_UNCACHED"
	       "}");
	if (rc >= 0)
		printf(" => {"
		       "handle=0x99c700000001"
		       "}");
	printf(") = %s\n", errstr);

	args->va_addr = 0x7fee58271000;
	args->size = 0x1000;
	args->mmap_offset = 0x7fee58271000;
	args->gpu_id = 0x99c7;
	args->flags = KFD_IOC_ALLOC_MEM_FLAGS_USERPTR|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE|KFD_IOC_ALLOC_MEM_FLAGS_COHERENT|KFD_IOC_ALLOC_MEM_FLAGS_UNCACHED;

	args->handle = 0x99c700000002;

	rc = do_ioctl(AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, args);
	printf("ioctl(-1, AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, {"
	       "va_addr=0x7fee58271000, "
	       "size=0x1000, "
	       "mmap_offset=0x7fee58271000, "
	       "gpu_id=0x99c7, "
	       "flags=KFD_IOC_ALLOC_MEM_FLAGS_USERPTR|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE|KFD_IOC_ALLOC_MEM_FLAGS_COHERENT|KFD_IOC_ALLOC_MEM_FLAGS_UNCACHED"
	       "}");
	if (rc >= 0)
		printf(" => {"
		       "handle=0x99c700000002"
		       "}");
	printf(") = %s\n", errstr);

	args->va_addr = 0x7fee5826f000;
	args->size = 0x1000;
	args->mmap_offset = 0x7fee5826f000;
	args->gpu_id = 0x99c7;
	args->flags = KFD_IOC_ALLOC_MEM_FLAGS_USERPTR|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE|KFD_IOC_ALLOC_MEM_FLAGS_COHERENT|KFD_IOC_ALLOC_MEM_FLAGS_UNCACHED;

	args->handle = 0x99c700000003;

	rc = do_ioctl(AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, args);
	printf("ioctl(-1, AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, {"
	       "va_addr=0x7fee5826f000, "
	       "size=0x1000, "
	       "mmap_offset=0x7fee5826f000, "
	       "gpu_id=0x99c7, "
	       "flags=KFD_IOC_ALLOC_MEM_FLAGS_USERPTR|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE|KFD_IOC_ALLOC_MEM_FLAGS_COHERENT|KFD_IOC_ALLOC_MEM_FLAGS_UNCACHED"
	       "}");
	if (rc >= 0)
		printf(" => {"
		       "handle=0x99c700000003"
		       "}");
	printf(") = %s\n", errstr);

	args->va_addr = 0x7fee5826d000;
	args->size = 0x1000;
	args->mmap_offset = 0x7fee5826d000;
	args->gpu_id = 0x99c7;
	args->flags = KFD_IOC_ALLOC_MEM_FLAGS_USERPTR|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE|KFD_IOC_ALLOC_MEM_FLAGS_COHERENT;

	args->handle = 0x99c700000004;

	rc = do_ioctl(AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, args);
	printf("ioctl(-1, AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, {"
	       "va_addr=0x7fee5826d000, "
	       "size=0x1000, "
	       "mmap_offset=0x7fee5826d000, "
	       "gpu_id=0x99c7, "
	       "flags=KFD_IOC_ALLOC_MEM_FLAGS_USERPTR|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE|KFD_IOC_ALLOC_MEM_FLAGS_COHERENT"
	       "}");
	if (rc >= 0)
		printf(" => {"
		       "handle=0x99c700000004"
		       "}");
	printf(") = %s\n", errstr);

	args->va_addr = 0x7fed56600000;
	args->size = 0x101000;
	args->mmap_offset = 0x7fed56600000;
	args->gpu_id = 0x99c7;
	args->flags = KFD_IOC_ALLOC_MEM_FLAGS_USERPTR|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE;

	args->handle = 0x99c700000005;

	rc = do_ioctl(AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, args);
	printf("ioctl(-1, AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, {"
	       "va_addr=0x7fed56600000, "
	       "size=0x101000, "
	       "mmap_offset=0x7fed56600000, "
	       "gpu_id=0x99c7, "
	       "flags=KFD_IOC_ALLOC_MEM_FLAGS_USERPTR|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE"
	       "}");
	if (rc >= 0)
		printf(" => {"
		       "handle=0x99c700000005"
		       "}");
	printf(") = %s\n", errstr);

	args->va_addr = 0x7fed56400000;
	args->size = 0x101000;
	args->mmap_offset = 0x7fed56400000;
	args->gpu_id = 0x99c7;
	args->flags = KFD_IOC_ALLOC_MEM_FLAGS_USERPTR|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE;

	args->handle = 0x99c700000006;

	rc = do_ioctl(AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, args);
	printf("ioctl(-1, AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, {"
	       "va_addr=0x7fed56400000, "
	       "size=0x101000, "
	       "mmap_offset=0x7fed56400000, "
	       "gpu_id=0x99c7, "
	       "flags=KFD_IOC_ALLOC_MEM_FLAGS_USERPTR|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE"
	       "}");
	if (rc >= 0)
		printf(" => {"
		       "handle=0x99c700000006"
		       "}");
	printf(") = %s\n", errstr);

	args->va_addr = 0x7fee57158000;
	args->size = 0xf000;
	args->mmap_offset = 0;
	args->gpu_id = 0x99c7;
	args->flags = KFD_IOC_ALLOC_MEM_FLAGS_VRAM|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_PUBLIC|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE;

	args->handle = 0x99c700000007;

	rc = do_ioctl(AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, args);
	printf("ioctl(-1, AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, {"
	       "va_addr=0x7fee57158000, "
	       "size=0xf000, "
	       "mmap_offset=0, "
	       "gpu_id=0x99c7, "
	       "flags=KFD_IOC_ALLOC_MEM_FLAGS_VRAM|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_PUBLIC|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE"
	       "}");
	if (rc >= 0)
		printf(" => {"
		       "handle=0x99c700000007"
		       "}");
	printf(") = %s\n", errstr);

	args->va_addr = 0x7fee57148000;
	args->size = 0xf000;
	args->mmap_offset = 0x7fee57148000;
	args->gpu_id = 0x99c7;
	args->flags = KFD_IOC_ALLOC_MEM_FLAGS_USERPTR|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE|KFD_IOC_ALLOC_MEM_FLAGS_COHERENT;

	args->handle = 0x99c700000008;

	rc = do_ioctl(AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, args);
	printf("ioctl(-1, AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, {"
	       "va_addr=0x7fee57148000, "
	       "size=0xf000, "
	       "mmap_offset=0x7fee57148000, "
	       "gpu_id=0x99c7, "
	       "flags=KFD_IOC_ALLOC_MEM_FLAGS_USERPTR|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE|KFD_IOC_ALLOC_MEM_FLAGS_COHERENT"
	       "}");
	if (rc >= 0)
		printf(" => {"
		       "handle=0x99c700000008"
		       "}");
	printf(") = %s\n", errstr);

	args->va_addr = 0x7fee5826b000;
	args->size = 0x1000;
	args->mmap_offset = 0x7fee5826b000;
	args->gpu_id = 0x99c7;
	args->flags = KFD_IOC_ALLOC_MEM_FLAGS_USERPTR|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE|KFD_IOC_ALLOC_MEM_FLAGS_COHERENT|KFD_IOC_ALLOC_MEM_FLAGS_UNCACHED;

	args->handle = 0x99c700000009;

	rc = do_ioctl(AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, args);
	printf("ioctl(-1, AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, {"
	       "va_addr=0x7fee5826b000, "
	       "size=0x1000, "
	       "mmap_offset=0x7fee5826b000, "
	       "gpu_id=0x99c7, "
	       "flags=KFD_IOC_ALLOC_MEM_FLAGS_USERPTR|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE|KFD_IOC_ALLOC_MEM_FLAGS_COHERENT|KFD_IOC_ALLOC_MEM_FLAGS_UNCACHED"
	       "}");
	if (rc >= 0)
		printf(" => {"
		       "handle=0x99c700000009"
		       "}");
	printf(") = %s\n", errstr);

	args->va_addr = 0x7fee58269000;
	args->size = 0x1000;
	args->mmap_offset = 0x7fee58269000;
	args->gpu_id = 0x99c7;
	args->flags = KFD_IOC_ALLOC_MEM_FLAGS_USERPTR|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE|KFD_IOC_ALLOC_MEM_FLAGS_COHERENT|KFD_IOC_ALLOC_MEM_FLAGS_UNCACHED;

	args->handle = 0x99c70000000a;

	rc = do_ioctl(AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, args);
	printf("ioctl(-1, AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, {"
	       "va_addr=0x7fee58269000, "
	       "size=0x1000, "
	       "mmap_offset=0x7fee58269000, "
	       "gpu_id=0x99c7, "
	       "flags=KFD_IOC_ALLOC_MEM_FLAGS_USERPTR|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE|KFD_IOC_ALLOC_MEM_FLAGS_COHERENT|KFD_IOC_ALLOC_MEM_FLAGS_UNCACHED"
	       "}");
	if (rc >= 0)
		printf(" => {"
		       "handle=0x99c70000000a"
		       "}");
	printf(") = %s\n", errstr);

	args->va_addr = 0x7fee58267000;
	args->size = 0x1000;
	args->mmap_offset = 0;
	args->gpu_id = 0x99c7;
	args->flags = KFD_IOC_ALLOC_MEM_FLAGS_GTT|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE|KFD_IOC_ALLOC_MEM_FLAGS_COHERENT|KFD_IOC_ALLOC_MEM_FLAGS_UNCACHED;

	args->handle = 0x99c70000000b;

	rc = do_ioctl(AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, args);
	printf("ioctl(-1, AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, {"
	       "va_addr=0x7fee58267000, "
	       "size=0x1000, "
	       "mmap_offset=0, "
	       "gpu_id=0x99c7, "
	       "flags=KFD_IOC_ALLOC_MEM_FLAGS_GTT|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE|KFD_IOC_ALLOC_MEM_FLAGS_COHERENT|KFD_IOC_ALLOC_MEM_FLAGS_UNCACHED"
	       "}");
	if (rc >= 0)
		printf(" => {"
		       "handle=0x99c70000000b"
		       "}");
	printf(") = %s\n", errstr);

	args->va_addr = 0x7fee58265000;
	args->size = 0x1000;
	args->mmap_offset = 0;
	args->gpu_id = 0x99c7;
	args->flags = KFD_IOC_ALLOC_MEM_FLAGS_VRAM|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE;

	args->handle = 0x99c70000000c;

	rc = do_ioctl(AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, args);
	printf("ioctl(-1, AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, {"
	       "va_addr=0x7fee58265000, "
	       "size=0x1000, "
	       "mmap_offset=0, "
	       "gpu_id=0x99c7, "
	       "flags=KFD_IOC_ALLOC_MEM_FLAGS_VRAM|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE"
	       "}");
	if (rc >= 0)
		printf(" => {"
		       "handle=0x99c70000000c"
		       "}");
	printf(") = %s\n", errstr);

	args->va_addr = 0x7fed54800000;
	args->size = 0x1aab000;
	args->mmap_offset = 0x7fed54800000;
	args->gpu_id = 0x99c7;
	args->flags = KFD_IOC_ALLOC_MEM_FLAGS_USERPTR|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE|KFD_IOC_ALLOC_MEM_FLAGS_COHERENT;

	args->handle = 0x99c70000000d;

	rc = do_ioctl(AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, args);
	printf("ioctl(-1, AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, {"
	       "va_addr=0x7fed54800000, "
	       "size=0x1aab000, "
	       "mmap_offset=0x7fed54800000, "
	       "gpu_id=0x99c7, "
	       "flags=KFD_IOC_ALLOC_MEM_FLAGS_USERPTR|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE|KFD_IOC_ALLOC_MEM_FLAGS_COHERENT"
	       "}");
	if (rc >= 0)
		printf(" => {"
		       "handle=0x99c70000000d"
		       "}");
	printf(") = %s\n", errstr);

	args->va_addr = 0x7fee579ba000;
	args->size = 0x2000;
	args->mmap_offset = 0;
	args->gpu_id = 0x99c7;
	args->flags = KFD_IOC_ALLOC_MEM_FLAGS_DOORBELL|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE|KFD_IOC_ALLOC_MEM_FLAGS_COHERENT;

	args->handle = 0x99c70000000e;

	rc = do_ioctl(AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, args);
	printf("ioctl(-1, AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, {"
	       "va_addr=0x7fee579ba000, "
	       "size=0x2000, "
	       "mmap_offset=0, "
	       "gpu_id=0x99c7, "
	       "flags=KFD_IOC_ALLOC_MEM_FLAGS_DOORBELL|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE|KFD_IOC_ALLOC_MEM_FLAGS_COHERENT"
	       "}");
	if (rc >= 0)
		printf(" => {"
		       "handle=0x99c70000000e"
		       "}");
	printf(") = %s\n", errstr);

	args->va_addr = 0x7fee571ad000;
	args->size = 0x1000;
	args->mmap_offset = 0x7fee571ad000;
	args->gpu_id = 0x99c7;
	args->flags = KFD_IOC_ALLOC_MEM_FLAGS_USERPTR|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE|KFD_IOC_ALLOC_MEM_FLAGS_COHERENT|KFD_IOC_ALLOC_MEM_FLAGS_UNCACHED;

	args->handle = 0x99c70000000f;

	rc = do_ioctl(AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, args);
	printf("ioctl(-1, AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, {"
	       "va_addr=0x7fee571ad000, "
	       "size=0x1000, "
	       "mmap_offset=0x7fee571ad000, "
	       "gpu_id=0x99c7, "
	       "flags=KFD_IOC_ALLOC_MEM_FLAGS_USERPTR|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE|KFD_IOC_ALLOC_MEM_FLAGS_COHERENT|KFD_IOC_ALLOC_MEM_FLAGS_UNCACHED"
	       "}");
	if (rc >= 0)
		printf(" => {"
		       "handle=0x99c70000000f"
		       "}");
	printf(") = %s\n", errstr);

	args->va_addr = 0x7fee571aa000;
	args->size = 0x2000;
	args->mmap_offset = 0x7fee571aa000;
	args->gpu_id = 0x99c7;
	args->flags = KFD_IOC_ALLOC_MEM_FLAGS_USERPTR|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE|KFD_IOC_ALLOC_MEM_FLAGS_COHERENT|KFD_IOC_ALLOC_MEM_FLAGS_UNCACHED;

	args->handle = 0x99c700000010;

	rc = do_ioctl(AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, args);
	printf("ioctl(-1, AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, {"
	       "va_addr=0x7fee571aa000, "
	       "size=0x2000, "
	       "mmap_offset=0x7fee571aa000, "
	       "gpu_id=0x99c7, "
	       "flags=KFD_IOC_ALLOC_MEM_FLAGS_USERPTR|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE|KFD_IOC_ALLOC_MEM_FLAGS_COHERENT|KFD_IOC_ALLOC_MEM_FLAGS_UNCACHED"
	       "}");
	if (rc >= 0)
		printf(" => {"
		       "handle=0x99c700000010"
		       "}");
	printf(") = %s\n", errstr);

	args->va_addr = 0x7fee571a8000;
	args->size = 0x1000;
	args->mmap_offset = 0x7fee571a8000;
	args->gpu_id = 0x99c7;
	args->flags = KFD_IOC_ALLOC_MEM_FLAGS_USERPTR|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE|KFD_IOC_ALLOC_MEM_FLAGS_COHERENT|KFD_IOC_ALLOC_MEM_FLAGS_UNCACHED;

	args->handle = 0x99c700000011;

	rc = do_ioctl(AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, args);
	printf("ioctl(-1, AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, {"
	       "va_addr=0x7fee571a8000, "
	       "size=0x1000, "
	       "mmap_offset=0x7fee571a8000, "
	       "gpu_id=0x99c7, "
	       "flags=KFD_IOC_ALLOC_MEM_FLAGS_USERPTR|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE|KFD_IOC_ALLOC_MEM_FLAGS_COHERENT|KFD_IOC_ALLOC_MEM_FLAGS_UNCACHED"
	       "}");
	if (rc >= 0)
		printf(" => {"
		       "handle=0x99c700000011"
		       "}");
	printf(") = %s\n", errstr);

	args->va_addr = 0x7fee571a6000;
	args->size = 0x1000;
	args->mmap_offset = 0x7fee571a6000;
	args->gpu_id = 0x99c7;
	args->flags = KFD_IOC_ALLOC_MEM_FLAGS_USERPTR|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE|KFD_IOC_ALLOC_MEM_FLAGS_COHERENT|KFD_IOC_ALLOC_MEM_FLAGS_UNCACHED;

	args->handle = 0x99c700000012;

	rc = do_ioctl(AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, args);
	printf("ioctl(-1, AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, {"
	       "va_addr=0x7fee571a6000, "
	       "size=0x1000, "
	       "mmap_offset=0x7fee571a6000, "
	       "gpu_id=0x99c7, "
	       "flags=KFD_IOC_ALLOC_MEM_FLAGS_USERPTR|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE|KFD_IOC_ALLOC_MEM_FLAGS_COHERENT|KFD_IOC_ALLOC_MEM_FLAGS_UNCACHED"
	       "}");
	if (rc >= 0)
		printf(" => {"
		       "handle=0x99c700000012"
		       "}");
	printf(") = %s\n", errstr);

	args->va_addr = 0x7fee571a4000;
	args->size = 0x1000;
	args->mmap_offset = 0x7fee571a4000;
	args->gpu_id = 0x99c7;
	args->flags = KFD_IOC_ALLOC_MEM_FLAGS_USERPTR|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE|KFD_IOC_ALLOC_MEM_FLAGS_COHERENT|KFD_IOC_ALLOC_MEM_FLAGS_UNCACHED;

	args->handle = 0x99c700000013;

	rc = do_ioctl(AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, args);
	printf("ioctl(-1, AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, {"
	       "va_addr=0x7fee571a4000, "
	       "size=0x1000, "
	       "mmap_offset=0x7fee571a4000, "
	       "gpu_id=0x99c7, "
	       "flags=KFD_IOC_ALLOC_MEM_FLAGS_USERPTR|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE|KFD_IOC_ALLOC_MEM_FLAGS_COHERENT|KFD_IOC_ALLOC_MEM_FLAGS_UNCACHED"
	       "}");
	if (rc >= 0)
		printf(" => {"
		       "handle=0x99c700000013"
		       "}");
	printf(") = %s\n", errstr);

	args->va_addr = 0x7fed32200000;
	args->size = 0x67c4000;
	args->mmap_offset = 0;
	args->gpu_id = 0x99c7;
	args->flags = KFD_IOC_ALLOC_MEM_FLAGS_VRAM|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_PUBLIC|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE;

	args->handle = 0x99c700000014;

	rc = do_ioctl(AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, args);
	printf("ioctl(-1, AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, {"
	       "va_addr=0x7fed32200000, "
	       "size=0x67c4000, "
	       "mmap_offset=0, "
	       "gpu_id=0x99c7, "
	       "flags=KFD_IOC_ALLOC_MEM_FLAGS_VRAM|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_PUBLIC|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE"
	       "}");
	if (rc >= 0)
		printf(" => {"
		       "handle=0x99c700000014"
		       "}");
	printf(") = %s\n", errstr);

	args->va_addr = 0x7fed2ba00000;
	args->size = 0x67c4000;
	args->mmap_offset = 0x7fed2ba00000;
	args->gpu_id = 0x99c7;
	args->flags = KFD_IOC_ALLOC_MEM_FLAGS_USERPTR|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE|KFD_IOC_ALLOC_MEM_FLAGS_COHERENT;

	args->handle = 0x99c700000015;

	rc = do_ioctl(AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, args);
	printf("ioctl(-1, AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, {"
	       "va_addr=0x7fed2ba00000, "
	       "size=0x67c4000, "
	       "mmap_offset=0x7fed2ba00000, "
	       "gpu_id=0x99c7, "
	       "flags=KFD_IOC_ALLOC_MEM_FLAGS_USERPTR|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE|KFD_IOC_ALLOC_MEM_FLAGS_COHERENT"
	       "}");
	if (rc >= 0)
		printf(" => {"
		       "handle=0x99c700000015"
		       "}");
	printf(") = %s\n", errstr);

	args->va_addr = 0x7fed4d800000;
	args->size = 0x12a3000;
	args->mmap_offset = 0;
	args->gpu_id = 0x99c7;
	args->flags = KFD_IOC_ALLOC_MEM_FLAGS_VRAM|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_PUBLIC|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE;

	args->handle = 0x99c700000016;

	rc = do_ioctl(AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, args);
	printf("ioctl(-1, AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, {"
	       "va_addr=0x7fed4d800000, "
	       "size=0x12a3000, "
	       "mmap_offset=0, "
	       "gpu_id=0x99c7, "
	       "flags=KFD_IOC_ALLOC_MEM_FLAGS_VRAM|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_PUBLIC|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE"
	       "}");
	if (rc >= 0)
		printf(" => {"
		       "handle=0x99c700000016"
		       "}");
	printf(") = %s\n", errstr);

	args->va_addr = 0x7fed4c400000;
	args->size = 0x12a3000;
	args->mmap_offset = 0x7fed4c400000;
	args->gpu_id = 0x99c7;
	args->flags = KFD_IOC_ALLOC_MEM_FLAGS_USERPTR|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE|KFD_IOC_ALLOC_MEM_FLAGS_COHERENT;

	args->handle = 0x99c700000017;

	rc = do_ioctl(AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, args);
	printf("ioctl(-1, AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, {"
	       "va_addr=0x7fed4c400000, "
	       "size=0x12a3000, "
	       "mmap_offset=0x7fed4c400000, "
	       "gpu_id=0x99c7, "
	       "flags=KFD_IOC_ALLOC_MEM_FLAGS_USERPTR|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE|KFD_IOC_ALLOC_MEM_FLAGS_COHERENT"
	       "}");
	if (rc >= 0)
		printf(" => {"
		       "handle=0x99c700000017"
		       "}");
	printf(") = %s\n", errstr);

	args->va_addr = 0x7fee571a0000;
	args->size = 0x3000;
	args->mmap_offset = 0;
	args->gpu_id = 0x99c7;
	args->flags = KFD_IOC_ALLOC_MEM_FLAGS_VRAM|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_PUBLIC|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE;

	args->handle = 0x99c700000018;

	rc = do_ioctl(AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, args);
	printf("ioctl(-1, AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, {"
	       "va_addr=0x7fee571a0000, "
	       "size=0x3000, "
	       "mmap_offset=0, "
	       "gpu_id=0x99c7, "
	       "flags=KFD_IOC_ALLOC_MEM_FLAGS_VRAM|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_PUBLIC|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE"
	       "}");
	if (rc >= 0)
		printf(" => {"
		       "handle=0x99c700000018"
		       "}");
	printf(") = %s\n", errstr);

	args->va_addr = 0x7fee57144000;
	args->size = 0x3000;
	args->mmap_offset = 0x7fee57144000;
	args->gpu_id = 0x99c7;
	args->flags = KFD_IOC_ALLOC_MEM_FLAGS_USERPTR|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE|KFD_IOC_ALLOC_MEM_FLAGS_COHERENT;

	args->handle = 0x99c700000019;

	rc = do_ioctl(AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, args);
	printf("ioctl(-1, AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, {"
	       "va_addr=0x7fee57144000, "
	       "size=0x3000, "
	       "mmap_offset=0x7fee57144000, "
	       "gpu_id=0x99c7, "
	       "flags=KFD_IOC_ALLOC_MEM_FLAGS_USERPTR|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE|KFD_IOC_ALLOC_MEM_FLAGS_COHERENT"
	       "}");
	if (rc >= 0)
		printf(" => {"
		       "handle=0x99c700000019"
		       "}");
	printf(") = %s\n", errstr);

	args->va_addr = 0x7fed54700000;
	args->size = 0xfe000;
	args->mmap_offset = 0;
	args->gpu_id = 0x99c7;
	args->flags = KFD_IOC_ALLOC_MEM_FLAGS_VRAM|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_PUBLIC|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE;

	args->handle = 0x99c70000001a;

	rc = do_ioctl(AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, args);
	printf("ioctl(-1, AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, {"
	       "va_addr=0x7fed54700000, "
	       "size=0xfe000, "
	       "mmap_offset=0, "
	       "gpu_id=0x99c7, "
	       "flags=KFD_IOC_ALLOC_MEM_FLAGS_VRAM|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_PUBLIC|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE"
	       "}");
	if (rc >= 0)
		printf(" => {"
		       "handle=0x99c70000001a"
		       "}");
	printf(") = %s\n", errstr);

	args->va_addr = 0x7fed54600000;
	args->size = 0xfe000;
	args->mmap_offset = 0x7fed54600000;
	args->gpu_id = 0x99c7;
	args->flags = KFD_IOC_ALLOC_MEM_FLAGS_USERPTR|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE|KFD_IOC_ALLOC_MEM_FLAGS_COHERENT;

	args->handle = 0x99c70000001b;

	rc = do_ioctl(AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, args);
	printf("ioctl(-1, AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, {"
	       "va_addr=0x7fed54600000, "
	       "size=0xfe000, "
	       "mmap_offset=0x7fed54600000, "
	       "gpu_id=0x99c7, "
	       "flags=KFD_IOC_ALLOC_MEM_FLAGS_USERPTR|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE|KFD_IOC_ALLOC_MEM_FLAGS_COHERENT"
	       "}");
	if (rc >= 0)
		printf(" => {"
		       "handle=0x99c70000001b"
		       "}");
	printf(") = %s\n", errstr);

	args->va_addr = 0x7fee57138000;
	args->size = 0x9000;
	args->mmap_offset = 0;
	args->gpu_id = 0x99c7;
	args->flags = KFD_IOC_ALLOC_MEM_FLAGS_VRAM|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_PUBLIC|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE;

	args->handle = 0x99c70000001c;

	rc = do_ioctl(AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, args);
	printf("ioctl(-1, AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, {"
	       "va_addr=0x7fee57138000, "
	       "size=0x9000, "
	       "mmap_offset=0, "
	       "gpu_id=0x99c7, "
	       "flags=KFD_IOC_ALLOC_MEM_FLAGS_VRAM|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_PUBLIC|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE"
	       "}");
	if (rc >= 0)
		printf(" => {"
		       "handle=0x99c70000001c"
		       "}");
	printf(") = %s\n", errstr);

	args->va_addr = 0x7fee57128000;
	args->size = 0x9000;
	args->mmap_offset = 0x7fee57128000;
	args->gpu_id = 0x99c7;
	args->flags = KFD_IOC_ALLOC_MEM_FLAGS_USERPTR|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE|KFD_IOC_ALLOC_MEM_FLAGS_COHERENT;

	args->handle = 0x99c70000001d;

	rc = do_ioctl(AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, args);
	printf("ioctl(-1, AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, {"
	       "va_addr=0x7fee57128000, "
	       "size=0x9000, "
	       "mmap_offset=0x7fee57128000, "
	       "gpu_id=0x99c7, "
	       "flags=KFD_IOC_ALLOC_MEM_FLAGS_USERPTR|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE|KFD_IOC_ALLOC_MEM_FLAGS_COHERENT"
	       "}");
	if (rc >= 0)
		printf(" => {"
		       "handle=0x99c70000001d"
		       "}");
	printf(") = %s\n", errstr);

	args->va_addr = 0x7fee57120000;
	args->size = 0x6000;
	args->mmap_offset = 0;
	args->gpu_id = 0x99c7;
	args->flags = KFD_IOC_ALLOC_MEM_FLAGS_VRAM|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_PUBLIC|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE;

	args->handle = 0x99c70000001e;

	rc = do_ioctl(AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, args);
	printf("ioctl(-1, AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, {"
	       "va_addr=0x7fee57120000, "
	       "size=0x6000, "
	       "mmap_offset=0, "
	       "gpu_id=0x99c7, "
	       "flags=KFD_IOC_ALLOC_MEM_FLAGS_VRAM|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_PUBLIC|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE"
	       "}");
	if (rc >= 0)
		printf(" => {"
		       "handle=0x99c70000001e"
		       "}");
	printf(") = %s\n", errstr);

	args->va_addr = 0x7fee57118000;
	args->size = 0x6000;
	args->mmap_offset = 0x7fee57118000;
	args->gpu_id = 0x99c7;
	args->flags = KFD_IOC_ALLOC_MEM_FLAGS_USERPTR|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE|KFD_IOC_ALLOC_MEM_FLAGS_COHERENT;

	args->handle = 0x99c70000001f;

	rc = do_ioctl(AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, args);
	printf("ioctl(-1, AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, {"
	       "va_addr=0x7fee57118000, "
	       "size=0x6000, "
	       "mmap_offset=0x7fee57118000, "
	       "gpu_id=0x99c7, "
	       "flags=KFD_IOC_ALLOC_MEM_FLAGS_USERPTR|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE|KFD_IOC_ALLOC_MEM_FLAGS_COHERENT"
	       "}");
	if (rc >= 0)
		printf(" => {"
		       "handle=0x99c70000001f"
		       "}");
	printf(") = %s\n", errstr);

	args->va_addr = 0x7fed54200000;
	args->size = 0x200000;
	args->mmap_offset = 0;
	args->gpu_id = 0x99c7;
	args->flags = KFD_IOC_ALLOC_MEM_FLAGS_VRAM|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_PUBLIC|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE;

	args->handle = 0x99c700000020;

	rc = do_ioctl(AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, args);
	printf("ioctl(-1, AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, {"
	       "va_addr=0x7fed54200000, "
	       "size=0x200000, "
	       "mmap_offset=0, "
	       "gpu_id=0x99c7, "
	       "flags=KFD_IOC_ALLOC_MEM_FLAGS_VRAM|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_PUBLIC|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE"
	       "}");
	if (rc >= 0)
		printf(" => {"
		       "handle=0x99c700000020"
		       "}");
	printf(") = %s\n", errstr);

	args->va_addr = 0x7fed4fc00000;
	args->size = 0x200000;
	args->mmap_offset = 0;
	args->gpu_id = 0x99c7;
	args->flags = KFD_IOC_ALLOC_MEM_FLAGS_VRAM|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_PUBLIC|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE;

	args->handle = 0x99c700000021;

	rc = do_ioctl(AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, args);
	printf("ioctl(-1, AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, {"
	       "va_addr=0x7fed4fc00000, "
	       "size=0x200000, "
	       "mmap_offset=0, "
	       "gpu_id=0x99c7, "
	       "flags=KFD_IOC_ALLOC_MEM_FLAGS_VRAM|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_PUBLIC|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE"
	       "}");
	if (rc >= 0)
		printf(" => {"
		       "handle=0x99c700000021"
		       "}");
	printf(") = %s\n", errstr);

	args->va_addr = 0x7fed3ea00000;
	args->size = 0x1400000;
	args->mmap_offset = 0;
	args->gpu_id = 0x99c7;
	args->flags = KFD_IOC_ALLOC_MEM_FLAGS_VRAM|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_PUBLIC|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE;

	args->handle = 0x99c700000022;

	rc = do_ioctl(AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, args);
	printf("ioctl(-1, AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, {"
	       "va_addr=0x7fed3ea00000, "
	       "size=0x1400000, "
	       "mmap_offset=0, "
	       "gpu_id=0x99c7, "
	       "flags=KFD_IOC_ALLOC_MEM_FLAGS_VRAM|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_PUBLIC|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE"
	       "}");
	if (rc >= 0)
		printf(" => {"
		       "handle=0x99c700000022"
		       "}");
	printf(") = %s\n", errstr);

	args->va_addr = 0x7fed3d400000;
	args->size = 0x1400000;
	args->mmap_offset = 0;
	args->gpu_id = 0x99c7;
	args->flags = KFD_IOC_ALLOC_MEM_FLAGS_VRAM|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_PUBLIC|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE;

	args->handle = 0x99c700000023;

	rc = do_ioctl(AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, args);
	printf("ioctl(-1, AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, {"
	       "va_addr=0x7fed3d400000, "
	       "size=0x1400000, "
	       "mmap_offset=0, "
	       "gpu_id=0x99c7, "
	       "flags=KFD_IOC_ALLOC_MEM_FLAGS_VRAM|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_PUBLIC|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE"
	       "}");
	if (rc >= 0)
		printf(" => {"
		       "handle=0x99c700000023"
		       "}");
	printf(") = %s\n", errstr);

	args->va_addr = 0x7fed4f800000;
	args->size = 0x200000;
	args->mmap_offset = 0;
	args->gpu_id = 0x99c7;
	args->flags = KFD_IOC_ALLOC_MEM_FLAGS_VRAM|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_PUBLIC|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE;

	args->handle = 0x99c700000024;

	rc = do_ioctl(AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, args);
	printf("ioctl(-1, AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, {"
	       "va_addr=0x7fed4f800000, "
	       "size=0x200000, "
	       "mmap_offset=0, "
	       "gpu_id=0x99c7, "
	       "flags=KFD_IOC_ALLOC_MEM_FLAGS_VRAM|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_PUBLIC|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE"
	       "}");
	if (rc >= 0)
		printf(" => {"
		       "handle=0x99c700000024"
		       "}");
	printf(") = %s\n", errstr);

	args->va_addr = 0x7fed3be00000;
	args->size = 0x1400000;
	args->mmap_offset = 0;
	args->gpu_id = 0x99c7;
	args->flags = KFD_IOC_ALLOC_MEM_FLAGS_VRAM|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_PUBLIC|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE;

	args->handle = 0x99c700000025;

	rc = do_ioctl(AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, args);
	printf("ioctl(-1, AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, {"
	       "va_addr=0x7fed3be00000, "
	       "size=0x1400000, "
	       "mmap_offset=0, "
	       "gpu_id=0x99c7, "
	       "flags=KFD_IOC_ALLOC_MEM_FLAGS_VRAM|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_PUBLIC|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE"
	       "}");
	if (rc >= 0)
		printf(" => {"
		       "handle=0x99c700000025"
		       "}");
	printf(") = %s\n", errstr);

	args->va_addr = 0x7fed39c00000;
	args->size = 0x2000000;
	args->mmap_offset = 0;
	args->gpu_id = 0x99c7;
	args->flags = KFD_IOC_ALLOC_MEM_FLAGS_VRAM|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_PUBLIC|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE;

	args->handle = 0x99c700000026;

	rc = do_ioctl(AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, args);
	printf("ioctl(-1, AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, {"
	       "va_addr=0x7fed39c00000, "
	       "size=0x2000000, "
	       "mmap_offset=0, "
	       "gpu_id=0x99c7, "
	       "flags=KFD_IOC_ALLOC_MEM_FLAGS_VRAM|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_PUBLIC|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE"
	       "}");
	if (rc >= 0)
		printf(" => {"
		       "handle=0x99c700000026"
		       "}");
	printf(") = %s\n", errstr);

	args->va_addr = 0x7fed4f400000;
	args->size = 0x200000;
	args->mmap_offset = 0;
	args->gpu_id = 0x99c7;
	args->flags = KFD_IOC_ALLOC_MEM_FLAGS_VRAM|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_PUBLIC|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE;

	args->handle = 0x99c700000027;

	rc = do_ioctl(AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, args);
	printf("ioctl(-1, AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, {"
	       "va_addr=0x7fed4f400000, "
	       "size=0x200000, "
	       "mmap_offset=0, "
	       "gpu_id=0x99c7, "
	       "flags=KFD_IOC_ALLOC_MEM_FLAGS_VRAM|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_PUBLIC|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE"
	       "}");
	if (rc >= 0)
		printf(" => {"
		       "handle=0x99c700000027"
		       "}");
	printf(") = %s\n", errstr);

	args->va_addr = 0x7fed4f000000;
	args->size = 0x200000;
	args->mmap_offset = 0;
	args->gpu_id = 0x99c7;
	args->flags = KFD_IOC_ALLOC_MEM_FLAGS_VRAM|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_PUBLIC|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE;

	args->handle = 0x99c700000028;

	rc = do_ioctl(AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, args);
	printf("ioctl(-1, AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, {"
	       "va_addr=0x7fed4f000000, "
	       "size=0x200000, "
	       "mmap_offset=0, "
	       "gpu_id=0x99c7, "
	       "flags=KFD_IOC_ALLOC_MEM_FLAGS_VRAM|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_PUBLIC|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE"
	       "}");
	if (rc >= 0)
		printf(" => {"
		       "handle=0x99c700000028"
		       "}");
	printf(") = %s\n", errstr);

	args->va_addr = 0x7fed4ec00000;
	args->size = 0x200000;
	args->mmap_offset = 0;
	args->gpu_id = 0x99c7;
	args->flags = KFD_IOC_ALLOC_MEM_FLAGS_VRAM|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_PUBLIC|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE;

	args->handle = 0x99c700000029;

	rc = do_ioctl(AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, args);
	printf("ioctl(-1, AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, {"
	       "va_addr=0x7fed4ec00000, "
	       "size=0x200000, "
	       "mmap_offset=0, "
	       "gpu_id=0x99c7, "
	       "flags=KFD_IOC_ALLOC_MEM_FLAGS_VRAM|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_PUBLIC|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE"
	       "}");
	if (rc >= 0)
		printf(" => {"
		       "handle=0x99c700000029"
		       "}");
	printf(") = %s\n", errstr);

	args->va_addr = 0x7fed39800000;
	args->size = 0x200000;
	args->mmap_offset = 0;
	args->gpu_id = 0x99c7;
	args->flags = KFD_IOC_ALLOC_MEM_FLAGS_VRAM|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_PUBLIC|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE;

	args->handle = 0x99c70000002a;

	rc = do_ioctl(AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, args);
	printf("ioctl(-1, AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, {"
	       "va_addr=0x7fed39800000, "
	       "size=0x200000, "
	       "mmap_offset=0, "
	       "gpu_id=0x99c7, "
	       "flags=KFD_IOC_ALLOC_MEM_FLAGS_VRAM|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_PUBLIC|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE"
	       "}");
	if (rc >= 0)
		printf(" => {"
		       "handle=0x99c70000002a"
		       "}");
	printf(") = %s\n", errstr);

	args->va_addr = 0x7fed39400000;
	args->size = 0x200000;
	args->mmap_offset = 0;
	args->gpu_id = 0x99c7;
	args->flags = KFD_IOC_ALLOC_MEM_FLAGS_VRAM|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_PUBLIC|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE;

	args->handle = 0x99c70000002b;

	rc = do_ioctl(AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, args);
	printf("ioctl(-1, AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, {"
	       "va_addr=0x7fed39400000, "
	       "size=0x200000, "
	       "mmap_offset=0, "
	       "gpu_id=0x99c7, "
	       "flags=KFD_IOC_ALLOC_MEM_FLAGS_VRAM|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_PUBLIC|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE"
	       "}");
	if (rc >= 0)
		printf(" => {"
		       "handle=0x99c70000002b"
		       "}");
	printf(") = %s\n", errstr);

	args->va_addr = 0x7fed39000000;
	args->size = 0x200000;
	args->mmap_offset = 0;
	args->gpu_id = 0x99c7;
	args->flags = KFD_IOC_ALLOC_MEM_FLAGS_VRAM|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_PUBLIC|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE;

	args->handle = 0x99c70000002c;

	rc = do_ioctl(AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, args);
	printf("ioctl(-1, AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, {"
	       "va_addr=0x7fed39000000, "
	       "size=0x200000, "
	       "mmap_offset=0, "
	       "gpu_id=0x99c7, "
	       "flags=KFD_IOC_ALLOC_MEM_FLAGS_VRAM|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_PUBLIC|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE"
	       "}");
	if (rc >= 0)
		printf(" => {"
		       "handle=0x99c70000002c"
		       "}");
	printf(") = %s\n", errstr);

	args->va_addr = 0x7fed38c00000;
	args->size = 0x200000;
	args->mmap_offset = 0;
	args->gpu_id = 0x99c7;
	args->flags = KFD_IOC_ALLOC_MEM_FLAGS_VRAM|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_PUBLIC|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE;

	args->handle = 0x99c70000002d;

	rc = do_ioctl(AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, args);
	printf("ioctl(-1, AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, {"
	       "va_addr=0x7fed38c00000, "
	       "size=0x200000, "
	       "mmap_offset=0, "
	       "gpu_id=0x99c7, "
	       "flags=KFD_IOC_ALLOC_MEM_FLAGS_VRAM|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_PUBLIC|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE"
	       "}");
	if (rc >= 0)
		printf(" => {"
		       "handle=0x99c70000002d"
		       "}");
	printf(") = %s\n", errstr);

	args->va_addr = 0x7fed2b600000;
	args->size = 0x200000;
	args->mmap_offset = 0;
	args->gpu_id = 0x99c7;
	args->flags = KFD_IOC_ALLOC_MEM_FLAGS_VRAM|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_PUBLIC|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE;

	args->handle = 0x99c70000002e;

	rc = do_ioctl(AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, args);
	printf("ioctl(-1, AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, {"
	       "va_addr=0x7fed2b600000, "
	       "size=0x200000, "
	       "mmap_offset=0, "
	       "gpu_id=0x99c7, "
	       "flags=KFD_IOC_ALLOC_MEM_FLAGS_VRAM|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_PUBLIC|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE"
	       "}");
	if (rc >= 0)
		printf(" => {"
		       "handle=0x99c70000002e"
		       "}");
	printf(") = %s\n", errstr);

	args->va_addr = 0x7fed2b200000;
	args->size = 0x200000;
	args->mmap_offset = 0;
	args->gpu_id = 0x99c7;
	args->flags = KFD_IOC_ALLOC_MEM_FLAGS_VRAM|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_PUBLIC|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE;

	args->handle = 0x99c70000002f;

	rc = do_ioctl(AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, args);
	printf("ioctl(-1, AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, {"
	       "va_addr=0x7fed2b200000, "
	       "size=0x200000, "
	       "mmap_offset=0, "
	       "gpu_id=0x99c7, "
	       "flags=KFD_IOC_ALLOC_MEM_FLAGS_VRAM|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_PUBLIC|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE"
	       "}");
	if (rc >= 0)
		printf(" => {"
		       "handle=0x99c70000002f"
		       "}");
	printf(") = %s\n", errstr);

	args->va_addr = 0x7fed2ae00000;
	args->size = 0x200000;
	args->mmap_offset = 0;
	args->gpu_id = 0x99c7;
	args->flags = KFD_IOC_ALLOC_MEM_FLAGS_VRAM|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_PUBLIC|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE;

	args->handle = 0x99c700000030;

	rc = do_ioctl(AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, args);
	printf("ioctl(-1, AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, {"
	       "va_addr=0x7fed2ae00000, "
	       "size=0x200000, "
	       "mmap_offset=0, "
	       "gpu_id=0x99c7, "
	       "flags=KFD_IOC_ALLOC_MEM_FLAGS_VRAM|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_PUBLIC|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE"
	       "}");
	if (rc >= 0)
		printf(" => {"
		       "handle=0x99c700000030"
		       "}");
	printf(") = %s\n", errstr);

	args->va_addr = 0x7fed28c00000;
	args->size = 0x2000000;
	args->mmap_offset = 0;
	args->gpu_id = 0x99c7;
	args->flags = KFD_IOC_ALLOC_MEM_FLAGS_VRAM|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_PUBLIC|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE;

	args->handle = 0x99c700000031;

	rc = do_ioctl(AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, args);
	printf("ioctl(-1, AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, {"
	       "va_addr=0x7fed28c00000, "
	       "size=0x2000000, "
	       "mmap_offset=0, "
	       "gpu_id=0x99c7, "
	       "flags=KFD_IOC_ALLOC_MEM_FLAGS_VRAM|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_PUBLIC|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE"
	       "}");
	if (rc >= 0)
		printf(" => {"
		       "handle=0x99c700000031"
		       "}");
	printf(") = %s\n", errstr);

	args->va_addr = 0x7fee5719e000;
	args->size = 0x1000;
	args->mmap_offset = 0x7fee5719e000;
	args->gpu_id = 0x99c7;
	args->flags = KFD_IOC_ALLOC_MEM_FLAGS_USERPTR|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE|KFD_IOC_ALLOC_MEM_FLAGS_COHERENT|KFD_IOC_ALLOC_MEM_FLAGS_UNCACHED;

	args->handle = 0x99c700000032;

	rc = do_ioctl(AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, args);
	printf("ioctl(-1, AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, {"
	       "va_addr=0x7fee5719e000, "
	       "size=0x1000, "
	       "mmap_offset=0x7fee5719e000, "
	       "gpu_id=0x99c7, "
	       "flags=KFD_IOC_ALLOC_MEM_FLAGS_USERPTR|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE|KFD_IOC_ALLOC_MEM_FLAGS_COHERENT|KFD_IOC_ALLOC_MEM_FLAGS_UNCACHED"
	       "}");
	if (rc >= 0)
		printf(" => {"
		       "handle=0x99c700000032"
		       "}");
	printf(") = %s\n", errstr);

	args->va_addr = 0x7fed4c200000;
	args->size = 0x100000;
	args->mmap_offset = 0x7fed4c200000;
	args->gpu_id = 0x99c7;
	args->flags = KFD_IOC_ALLOC_MEM_FLAGS_USERPTR|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE|KFD_IOC_ALLOC_MEM_FLAGS_COHERENT|KFD_IOC_ALLOC_MEM_FLAGS_UNCACHED;

	args->handle = 0x99c700000033;

	rc = do_ioctl(AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, args);
	printf("ioctl(-1, AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, {"
	       "va_addr=0x7fed4c200000, "
	       "size=0x100000, "
	       "mmap_offset=0x7fed4c200000, "
	       "gpu_id=0x99c7, "
	       "flags=KFD_IOC_ALLOC_MEM_FLAGS_USERPTR|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE|KFD_IOC_ALLOC_MEM_FLAGS_COHERENT|KFD_IOC_ALLOC_MEM_FLAGS_UNCACHED"
	       "}");
	if (rc >= 0)
		printf(" => {"
		       "handle=0x99c700000033"
		       "}");
	printf(") = %s\n", errstr);

	args->va_addr = 0x7fee57169000;
	args->size = 0x1000;
	args->mmap_offset = 0;
	args->gpu_id = 0x99c7;
	args->flags = KFD_IOC_ALLOC_MEM_FLAGS_GTT|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE|KFD_IOC_ALLOC_MEM_FLAGS_COHERENT|KFD_IOC_ALLOC_MEM_FLAGS_UNCACHED;

	args->handle = 0x99c700000034;

	rc = do_ioctl(AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, args);
	printf("ioctl(-1, AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, {"
	       "va_addr=0x7fee57169000, "
	       "size=0x1000, "
	       "mmap_offset=0, "
	       "gpu_id=0x99c7, "
	       "flags=KFD_IOC_ALLOC_MEM_FLAGS_GTT|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE|KFD_IOC_ALLOC_MEM_FLAGS_COHERENT|KFD_IOC_ALLOC_MEM_FLAGS_UNCACHED"
	       "}");
	if (rc >= 0)
		printf(" => {"
		       "handle=0x99c700000034"
		       "}");
	printf(") = %s\n", errstr);

	args->va_addr = 0x7fee57142000;
	args->size = 0x1000;
	args->mmap_offset = 0;
	args->gpu_id = 0x99c7;
	args->flags = KFD_IOC_ALLOC_MEM_FLAGS_VRAM|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE;

	args->handle = 0x99c700000035;

	rc = do_ioctl(AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, args);
	printf("ioctl(-1, AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, {"
	       "va_addr=0x7fee57142000, "
	       "size=0x1000, "
	       "mmap_offset=0, "
	       "gpu_id=0x99c7, "
	       "flags=KFD_IOC_ALLOC_MEM_FLAGS_VRAM|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE"
	       "}");
	if (rc >= 0)
		printf(" => {"
		       "handle=0x99c700000035"
		       "}");
	printf(") = %s\n", errstr);

	args->va_addr = 0x7fed27000000;
	args->size = 0x1aab000;
	args->mmap_offset = 0x7fed27000000;
	args->gpu_id = 0x99c7;
	args->flags = KFD_IOC_ALLOC_MEM_FLAGS_USERPTR|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE|KFD_IOC_ALLOC_MEM_FLAGS_COHERENT;

	args->handle = 0x99c700000036;

	rc = do_ioctl(AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, args);
	printf("ioctl(-1, AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, {"
	       "va_addr=0x7fed27000000, "
	       "size=0x1aab000, "
	       "mmap_offset=0x7fed27000000, "
	       "gpu_id=0x99c7, "
	       "flags=KFD_IOC_ALLOC_MEM_FLAGS_USERPTR|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE|KFD_IOC_ALLOC_MEM_FLAGS_COHERENT"
	       "}");
	if (rc >= 0)
		printf(" => {"
		       "handle=0x99c700000036"
		       "}");
	printf(") = %s\n", errstr);

	args->va_addr = 0x7fee57136000;
	args->size = 0x1000;
	args->mmap_offset = 0x7fee57136000;
	args->gpu_id = 0x99c7;
	args->flags = KFD_IOC_ALLOC_MEM_FLAGS_USERPTR|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE|KFD_IOC_ALLOC_MEM_FLAGS_COHERENT|KFD_IOC_ALLOC_MEM_FLAGS_UNCACHED;

	args->handle = 0x99c700000037;

	rc = do_ioctl(AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, args);
	printf("ioctl(-1, AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, {"
	       "va_addr=0x7fee57136000, "
	       "size=0x1000, "
	       "mmap_offset=0x7fee57136000, "
	       "gpu_id=0x99c7, "
	       "flags=KFD_IOC_ALLOC_MEM_FLAGS_USERPTR|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE|KFD_IOC_ALLOC_MEM_FLAGS_COHERENT|KFD_IOC_ALLOC_MEM_FLAGS_UNCACHED"
	       "}");
	if (rc >= 0)
		printf(" => {"
		       "handle=0x99c700000037"
		       "}");
	printf(") = %s\n", errstr);

	args->va_addr = 0x7fed38a00000;
	args->size = 0x100000;
	args->mmap_offset = 0x7fed38a00000;
	args->gpu_id = 0x99c7;
	args->flags = KFD_IOC_ALLOC_MEM_FLAGS_USERPTR|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE;

	args->handle = 0x99c700000038;

	rc = do_ioctl(AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, args);
	printf("ioctl(-1, AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, {"
	       "va_addr=0x7fed38a00000, "
	       "size=0x100000, "
	       "mmap_offset=0x7fed38a00000, "
	       "gpu_id=0x99c7, "
	       "flags=KFD_IOC_ALLOC_MEM_FLAGS_USERPTR|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE"
	       "}");
	if (rc >= 0)
		printf(" => {"
		       "handle=0x99c700000038"
		       "}");
	printf(") = %s\n", errstr);

	args->va_addr = 0x7fee57108000;
	args->size = 0x8000;
	args->mmap_offset = 0;
	args->gpu_id = 0x99c7;
	args->flags = KFD_IOC_ALLOC_MEM_FLAGS_VRAM|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_PUBLIC|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE;

	args->handle = 0x99c700000039;

	rc = do_ioctl(AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, args);
	printf("ioctl(-1, AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, {"
	       "va_addr=0x7fee57108000, "
	       "size=0x8000, "
	       "mmap_offset=0, "
	       "gpu_id=0x99c7, "
	       "flags=KFD_IOC_ALLOC_MEM_FLAGS_VRAM|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_PUBLIC|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE"
	       "}");
	if (rc >= 0)
		printf(" => {"
		       "handle=0x99c700000039"
		       "}");
	printf(") = %s\n", errstr);

	args->va_addr = 0x7fee570f8000;
	args->size = 0x8000;
	args->mmap_offset = 0x7fee570f8000;
	args->gpu_id = 0x99c7;
	args->flags = KFD_IOC_ALLOC_MEM_FLAGS_USERPTR|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE|KFD_IOC_ALLOC_MEM_FLAGS_COHERENT;

	args->handle = 0x99c70000003a;

	rc = do_ioctl(AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, args);
	printf("ioctl(-1, AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, {"
	       "va_addr=0x7fee570f8000, "
	       "size=0x8000, "
	       "mmap_offset=0x7fee570f8000, "
	       "gpu_id=0x99c7, "
	       "flags=KFD_IOC_ALLOC_MEM_FLAGS_USERPTR|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE|KFD_IOC_ALLOC_MEM_FLAGS_COHERENT"
	       "}");
	if (rc >= 0)
		printf(" => {"
		       "handle=0x99c70000003a"
		       "}");
	printf(") = %s\n", errstr);

	args->va_addr = 0x7fee57132000;
	args->size = 0x2000;
	args->mmap_offset = 0x7fee57132000;
	args->gpu_id = 0x99c7;
	args->flags = KFD_IOC_ALLOC_MEM_FLAGS_USERPTR|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE|KFD_IOC_ALLOC_MEM_FLAGS_COHERENT|KFD_IOC_ALLOC_MEM_FLAGS_UNCACHED;

	args->handle = 0x99c70000003b;

	rc = do_ioctl(AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, args);
	printf("ioctl(-1, AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, {"
	       "va_addr=0x7fee57132000, "
	       "size=0x2000, "
	       "mmap_offset=0x7fee57132000, "
	       "gpu_id=0x99c7, "
	       "flags=KFD_IOC_ALLOC_MEM_FLAGS_USERPTR|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE|KFD_IOC_ALLOC_MEM_FLAGS_COHERENT|KFD_IOC_ALLOC_MEM_FLAGS_UNCACHED"
	       "}");
	if (rc >= 0)
		printf(" => {"
		       "handle=0x99c70000003b"
		       "}");
	printf(") = %s\n", errstr);

	args->va_addr = 0x7fed26c00000;
	args->size = 0x201000;
	args->mmap_offset = 0x55c116ec1000;
	args->gpu_id = 0x99c7;
	args->flags = KFD_IOC_ALLOC_MEM_FLAGS_USERPTR|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE;

	args->handle = 0x99c70000003c;

	rc = do_ioctl(AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, args);
	printf("ioctl(-1, AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, {"
	       "va_addr=0x7fed26c00000, "
	       "size=0x201000, "
	       "mmap_offset=0x55c116ec1000, "
	       "gpu_id=0x99c7, "
	       "flags=KFD_IOC_ALLOC_MEM_FLAGS_USERPTR|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE"
	       "}");
	if (rc >= 0)
		printf(" => {"
		       "handle=0x99c70000003c"
		       "}");
	printf(") = %s\n", errstr);

	args->va_addr = 0x7fed26a00000;
	args->size = 0x100000;
	args->mmap_offset = 0x7fed26a00000;
	args->gpu_id = 0x99c7;
	args->flags = KFD_IOC_ALLOC_MEM_FLAGS_USERPTR|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE|KFD_IOC_ALLOC_MEM_FLAGS_COHERENT|KFD_IOC_ALLOC_MEM_FLAGS_UNCACHED;

	args->handle = 0x99c70000003d;

	rc = do_ioctl(AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, args);
	printf("ioctl(-1, AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, {"
	       "va_addr=0x7fed26a00000, "
	       "size=0x100000, "
	       "mmap_offset=0x7fed26a00000, "
	       "gpu_id=0x99c7, "
	       "flags=KFD_IOC_ALLOC_MEM_FLAGS_USERPTR|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE|KFD_IOC_ALLOC_MEM_FLAGS_COHERENT|KFD_IOC_ALLOC_MEM_FLAGS_UNCACHED"
	       "}");
	if (rc >= 0)
		printf(" => {"
		       "handle=0x99c70000003d"
		       "}");
	printf(") = %s\n", errstr);

	args->va_addr = 0x7fee57106000;
	args->size = 0x1000;
	args->mmap_offset = 0;
	args->gpu_id = 0x99c7;
	args->flags = KFD_IOC_ALLOC_MEM_FLAGS_GTT|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE|KFD_IOC_ALLOC_MEM_FLAGS_COHERENT|KFD_IOC_ALLOC_MEM_FLAGS_UNCACHED;

	args->handle = 0x99c70000003e;

	rc = do_ioctl(AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, args);
	printf("ioctl(-1, AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, {"
	       "va_addr=0x7fee57106000, "
	       "size=0x1000, "
	       "mmap_offset=0, "
	       "gpu_id=0x99c7, "
	       "flags=KFD_IOC_ALLOC_MEM_FLAGS_GTT|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE|KFD_IOC_ALLOC_MEM_FLAGS_COHERENT|KFD_IOC_ALLOC_MEM_FLAGS_UNCACHED"
	       "}");
	if (rc >= 0)
		printf(" => {"
		       "handle=0x99c70000003e"
		       "}");
	printf(") = %s\n", errstr);

	args->va_addr = 0x7fed26c00000;
	args->size = 0x201000;
	args->mmap_offset = 0x55c116ec1000;
	args->gpu_id = 0x99c7;
	args->flags = KFD_IOC_ALLOC_MEM_FLAGS_USERPTR|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE;

	args->handle = 0x99c70000003c;

	rc = do_ioctl(AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, args);
	printf("ioctl(-1, AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, {"
	       "va_addr=0x7fed26c00000, "
	       "size=0x201000, "
	       "mmap_offset=0x55c116ec1000, "
	       "gpu_id=0x99c7, "
	       "flags=KFD_IOC_ALLOC_MEM_FLAGS_USERPTR|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE"
	       "}");
	if (rc >= 0)
		printf(" => {"
		       "handle=0x99c70000003c"
		       "}");
	printf(") = %s\n", errstr);

	args->va_addr = 0x7fed24000000;
	args->size = 0x1401000;
	args->mmap_offset = 0x7fed255ff000;
	args->gpu_id = 0x99c7;
	args->flags = KFD_IOC_ALLOC_MEM_FLAGS_USERPTR|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE;

	args->handle = 0x99c70000003c;

	rc = do_ioctl(AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, args);
	printf("ioctl(-1, AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, {"
	       "va_addr=0x7fed24000000, "
	       "size=0x1401000, "
	       "mmap_offset=0x7fed255ff000, "
	       "gpu_id=0x99c7, "
	       "flags=KFD_IOC_ALLOC_MEM_FLAGS_USERPTR|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE"
	       "}");
	if (rc >= 0)
		printf(" => {"
		       "handle=0x99c70000003c"
		       "}");
	printf(") = %s\n", errstr);

	args->va_addr = 0x7fed24000000;
	args->size = 0x1401000;
	args->mmap_offset = 0x7fed255ff000;
	args->gpu_id = 0x99c7;
	args->flags = KFD_IOC_ALLOC_MEM_FLAGS_USERPTR|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE;

	args->handle = 0x99c70000003c;

	rc = do_ioctl(AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, args);
	printf("ioctl(-1, AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, {"
	       "va_addr=0x7fed24000000, "
	       "size=0x1401000, "
	       "mmap_offset=0x7fed255ff000, "
	       "gpu_id=0x99c7, "
	       "flags=KFD_IOC_ALLOC_MEM_FLAGS_USERPTR|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE"
	       "}");
	if (rc >= 0)
		printf(" => {"
		       "handle=0x99c70000003c"
		       "}");
	printf(") = %s\n", errstr);

	args->va_addr = 0x7fed26c00000;
	args->size = 0x201000;
	args->mmap_offset = 0x7fed255ff000;
	args->gpu_id = 0x99c7;
	args->flags = KFD_IOC_ALLOC_MEM_FLAGS_USERPTR|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE;

	args->handle = 0x99c70000003c;

	rc = do_ioctl(AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, args);
	printf("ioctl(-1, AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, {"
	       "va_addr=0x7fed26c00000, "
	       "size=0x201000, "
	       "mmap_offset=0x7fed255ff000, "
	       "gpu_id=0x99c7, "
	       "flags=KFD_IOC_ALLOC_MEM_FLAGS_USERPTR|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE"
	       "}");
	if (rc >= 0)
		printf(" => {"
		       "handle=0x99c70000003c"
		       "}");
	printf(") = %s\n", errstr);

	args->va_addr = 0x7fed24000000;
	args->size = 0x1401000;
	args->mmap_offset = 0x7fed255ff000;
	args->gpu_id = 0x99c7;
	args->flags = KFD_IOC_ALLOC_MEM_FLAGS_USERPTR|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE;

	args->handle = 0x99c70000003c;

	rc = do_ioctl(AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, args);
	printf("ioctl(-1, AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, {"
	       "va_addr=0x7fed24000000, "
	       "size=0x1401000, "
	       "mmap_offset=0x7fed255ff000, "
	       "gpu_id=0x99c7, "
	       "flags=KFD_IOC_ALLOC_MEM_FLAGS_USERPTR|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE"
	       "}");
	if (rc >= 0)
		printf(" => {"
		       "handle=0x99c70000003c"
		       "}");
	printf(") = %s\n", errstr);

	args->va_addr = 0x7fed26e00000;
	args->size = 0x100000;
	args->mmap_offset = 0x7fed26e00000;
	args->gpu_id = 0x99c7;
	args->flags = KFD_IOC_ALLOC_MEM_FLAGS_USERPTR|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE|KFD_IOC_ALLOC_MEM_FLAGS_COHERENT|KFD_IOC_ALLOC_MEM_FLAGS_UNCACHED;

	args->handle = 0x99c70000003c;

	rc = do_ioctl(AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, args);
	printf("ioctl(-1, AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, {"
	       "va_addr=0x7fed26e00000, "
	       "size=0x100000, "
	       "mmap_offset=0x7fed26e00000, "
	       "gpu_id=0x99c7, "
	       "flags=KFD_IOC_ALLOC_MEM_FLAGS_USERPTR|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE|KFD_IOC_ALLOC_MEM_FLAGS_COHERENT|KFD_IOC_ALLOC_MEM_FLAGS_UNCACHED"
	       "}");
	if (rc >= 0)
		printf(" => {"
		       "handle=0x99c70000003c"
		       "}");
	printf(") = %s\n", errstr);

	args->va_addr = 0x7fee57104000;
	args->size = 0x1000;
	args->mmap_offset = 0;
	args->gpu_id = 0x99c7;
	args->flags = KFD_IOC_ALLOC_MEM_FLAGS_GTT|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE|KFD_IOC_ALLOC_MEM_FLAGS_COHERENT|KFD_IOC_ALLOC_MEM_FLAGS_UNCACHED;

	args->handle = 0x99c70000003f;

	rc = do_ioctl(AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, args);
	printf("ioctl(-1, AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, {"
	       "va_addr=0x7fee57104000, "
	       "size=0x1000, "
	       "mmap_offset=0, "
	       "gpu_id=0x99c7, "
	       "flags=KFD_IOC_ALLOC_MEM_FLAGS_GTT|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE|KFD_IOC_ALLOC_MEM_FLAGS_COHERENT|KFD_IOC_ALLOC_MEM_FLAGS_UNCACHED"
	       "}");
	if (rc >= 0)
		printf(" => {"
		       "handle=0x99c70000003f"
		       "}");
	printf(") = %s\n", errstr);

	args->va_addr = 0x7fc390800000;
	args->size = 0x8000;
	args->mmap_offset = 0x7fc390800000;
	args->gpu_id = 0x99c7;
	args->flags = KFD_IOC_ALLOC_MEM_FLAGS_GTT|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE|KFD_IOC_ALLOC_MEM_FLAGS_COHERENT|KFD_IOC_ALLOC_MEM_FLAGS_UNCACHED;

	args->handle = 0x99c700000000;

	rc = do_ioctl(AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, args);
	printf("ioctl(-1, AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, {"
	       "va_addr=0x7fc390800000, "
	       "size=0x8000, "
	       "mmap_offset=0x7fc390800000, "
	       "gpu_id=0x99c7, "
	       "flags=KFD_IOC_ALLOC_MEM_FLAGS_GTT|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE|KFD_IOC_ALLOC_MEM_FLAGS_COHERENT|KFD_IOC_ALLOC_MEM_FLAGS_UNCACHED"
	       "}");
	if (rc >= 0)
		printf(" => {"
		       "handle=0x99c700000000"
		       "}");
	printf(") = %s\n", errstr);

	args->va_addr = 0x7fc390400000;
	args->size = 0x40000;
	args->mmap_offset = 0x7fc390400000;
	args->gpu_id = 0x99c7;
	args->flags = KFD_IOC_ALLOC_MEM_FLAGS_USERPTR|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE|KFD_IOC_ALLOC_MEM_FLAGS_COHERENT|KFD_IOC_ALLOC_MEM_FLAGS_UNCACHED;

	args->handle = 0x99c700000001;

	rc = do_ioctl(AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, args);
	printf("ioctl(-1, AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, {"
	       "va_addr=0x7fc390400000, "
	       "size=0x40000, "
	       "mmap_offset=0x7fc390400000, "
	       "gpu_id=0x99c7, "
	       "flags=KFD_IOC_ALLOC_MEM_FLAGS_USERPTR|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE|KFD_IOC_ALLOC_MEM_FLAGS_COHERENT|KFD_IOC_ALLOC_MEM_FLAGS_UNCACHED"
	       "}");
	if (rc >= 0)
		printf(" => {"
		       "handle=0x99c700000001"
		       "}");
	printf(") = %s\n", errstr);

	args->va_addr = 0x7fc390000000;
	args->size = 0x100000;
	args->mmap_offset = 0x7fc390000000;
	args->gpu_id = 0x99c7;
	args->flags = KFD_IOC_ALLOC_MEM_FLAGS_USERPTR|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE|KFD_IOC_ALLOC_MEM_FLAGS_COHERENT;

	args->handle = 0x99c700000002;

	rc = do_ioctl(AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, args);
	printf("ioctl(-1, AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, {"
	       "va_addr=0x7fc390000000, "
	       "size=0x100000, "
	       "mmap_offset=0x7fc390000000, "
	       "gpu_id=0x99c7, "
	       "flags=KFD_IOC_ALLOC_MEM_FLAGS_USERPTR|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE|KFD_IOC_ALLOC_MEM_FLAGS_COHERENT"
	       "}");
	if (rc >= 0)
		printf(" => {"
		       "handle=0x99c700000002"
		       "}");
	printf(") = %s\n", errstr);

	args->va_addr = 0x7fc38fc00000;
	args->size = 0x100000;
	args->mmap_offset = 0x7fc38fc00000;
	args->gpu_id = 0x99c7;
	args->flags = KFD_IOC_ALLOC_MEM_FLAGS_USERPTR|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE|KFD_IOC_ALLOC_MEM_FLAGS_COHERENT|KFD_IOC_ALLOC_MEM_FLAGS_UNCACHED;

	args->handle = 0x99c700000003;

	rc = do_ioctl(AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, args);
	printf("ioctl(-1, AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, {"
	       "va_addr=0x7fc38fc00000, "
	       "size=0x100000, "
	       "mmap_offset=0x7fc38fc00000, "
	       "gpu_id=0x99c7, "
	       "flags=KFD_IOC_ALLOC_MEM_FLAGS_USERPTR|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE|KFD_IOC_ALLOC_MEM_FLAGS_COHERENT|KFD_IOC_ALLOC_MEM_FLAGS_UNCACHED"
	       "}");
	if (rc >= 0)
		printf(" => {"
		       "handle=0x99c700000003"
		       "}");
	printf(") = %s\n", errstr);

	args->va_addr = 0x7fc38f800000;
	args->size = 0x1000;
	args->mmap_offset = 0x7fc38f800000;
	args->gpu_id = 0x99c7;
	args->flags = KFD_IOC_ALLOC_MEM_FLAGS_USERPTR|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE|KFD_IOC_ALLOC_MEM_FLAGS_COHERENT|KFD_IOC_ALLOC_MEM_FLAGS_UNCACHED;

	args->handle = 0x99c700000004;

	rc = do_ioctl(AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, args);
	printf("ioctl(-1, AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, {"
	       "va_addr=0x7fc38f800000, "
	       "size=0x1000, "
	       "mmap_offset=0x7fc38f800000, "
	       "gpu_id=0x99c7, "
	       "flags=KFD_IOC_ALLOC_MEM_FLAGS_USERPTR|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE|KFD_IOC_ALLOC_MEM_FLAGS_COHERENT|KFD_IOC_ALLOC_MEM_FLAGS_UNCACHED"
	       "}");
	if (rc >= 0)
		printf(" => {"
		       "handle=0x99c700000004"
		       "}");
	printf(") = %s\n", errstr);

	args->va_addr = 0x7fc38f400000;
	args->size = 0x2000;
	args->mmap_offset = 0x7fc38f400000;
	args->gpu_id = 0x99c7;
	args->flags = KFD_IOC_ALLOC_MEM_FLAGS_DOORBELL|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE|KFD_IOC_ALLOC_MEM_FLAGS_COHERENT;

	args->handle = 0x99c700000005;

	rc = do_ioctl(AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, args);
	printf("ioctl(-1, AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, {"
	       "va_addr=0x7fc38f400000, "
	       "size=0x2000, "
	       "mmap_offset=0x7fc38f400000, "
	       "gpu_id=0x99c7, "
	       "flags=KFD_IOC_ALLOC_MEM_FLAGS_DOORBELL|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE|KFD_IOC_ALLOC_MEM_FLAGS_COHERENT"
	       "}");
	if (rc >= 0)
		printf(" => {"
		       "handle=0x99c700000005"
		       "}");
	printf(") = %s\n", errstr);

	args->va_addr = 0x7fc38f000000;
	args->size = 0x40000;
	args->mmap_offset = 0x7fc38f000000;
	args->gpu_id = 0x99c7;
	args->flags = KFD_IOC_ALLOC_MEM_FLAGS_USERPTR|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE|KFD_IOC_ALLOC_MEM_FLAGS_COHERENT|KFD_IOC_ALLOC_MEM_FLAGS_UNCACHED;

	args->handle = 0x99c700000006;

	rc = do_ioctl(AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, args);
	printf("ioctl(-1, AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, {"
	       "va_addr=0x7fc38f000000, "
	       "size=0x40000, "
	       "mmap_offset=0x7fc38f000000, "
	       "gpu_id=0x99c7, "
	       "flags=KFD_IOC_ALLOC_MEM_FLAGS_USERPTR|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE|KFD_IOC_ALLOC_MEM_FLAGS_COHERENT|KFD_IOC_ALLOC_MEM_FLAGS_UNCACHED"
	       "}");
	if (rc >= 0)
		printf(" => {"
		       "handle=0x99c700000006"
		       "}");
	printf(") = %s\n", errstr);

	args->va_addr = 0x7fc38ec00000;
	args->size = 0x1000;
	args->mmap_offset = 0x7fc38ec00000;
	args->gpu_id = 0x99c7;
	args->flags = KFD_IOC_ALLOC_MEM_FLAGS_USERPTR|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE|KFD_IOC_ALLOC_MEM_FLAGS_COHERENT|KFD_IOC_ALLOC_MEM_FLAGS_UNCACHED;

	args->handle = 0x99c700000007;

	rc = do_ioctl(AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, args);
	printf("ioctl(-1, AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, {"
	       "va_addr=0x7fc38ec00000, "
	       "size=0x1000, "
	       "mmap_offset=0x7fc38ec00000, "
	       "gpu_id=0x99c7, "
	       "flags=KFD_IOC_ALLOC_MEM_FLAGS_USERPTR|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE|KFD_IOC_ALLOC_MEM_FLAGS_COHERENT|KFD_IOC_ALLOC_MEM_FLAGS_UNCACHED"
	       "}");
	if (rc >= 0)
		printf(" => {"
		       "handle=0x99c700000007"
		       "}");
	printf(") = %s\n", errstr);

	args->va_addr = 0x7fc38ce00000;
	args->size = 0x1c00000;
	args->mmap_offset = 0x7fc38ce00000;
	args->gpu_id = 0x99c7;
	args->flags = KFD_IOC_ALLOC_MEM_FLAGS_USERPTR|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE|KFD_IOC_ALLOC_MEM_FLAGS_COHERENT;

	args->handle = 0x99c700000008;

	rc = do_ioctl(AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, args);
	printf("ioctl(-1, AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, {"
	       "va_addr=0x7fc38ce00000, "
	       "size=0x1c00000, "
	       "mmap_offset=0x7fc38ce00000, "
	       "gpu_id=0x99c7, "
	       "flags=KFD_IOC_ALLOC_MEM_FLAGS_USERPTR|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE|KFD_IOC_ALLOC_MEM_FLAGS_COHERENT"
	       "}");
	if (rc >= 0)
		printf(" => {"
		       "handle=0x99c700000008"
		       "}");
	printf(") = %s\n", errstr);

	args->va_addr = 0x7fc38ca00000;
	args->size = 0x100000;
	args->mmap_offset = 0x7fc38ca00000;
	args->gpu_id = 0x99c7;
	args->flags = KFD_IOC_ALLOC_MEM_FLAGS_USERPTR|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE|KFD_IOC_ALLOC_MEM_FLAGS_COHERENT;

	args->handle = 0x99c700000009;

	rc = do_ioctl(AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, args);
	printf("ioctl(-1, AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, {"
	       "va_addr=0x7fc38ca00000, "
	       "size=0x100000, "
	       "mmap_offset=0x7fc38ca00000, "
	       "gpu_id=0x99c7, "
	       "flags=KFD_IOC_ALLOC_MEM_FLAGS_USERPTR|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE|KFD_IOC_ALLOC_MEM_FLAGS_COHERENT"
	       "}");
	if (rc >= 0)
		printf(" => {"
		       "handle=0x99c700000009"
		       "}");
	printf(") = %s\n", errstr);

	args->va_addr = 0x7fc38c600000;
	args->size = 0xf000;
	args->mmap_offset = 0x7fc38c600000;
	args->gpu_id = 0x99c7;
	args->flags = KFD_IOC_ALLOC_MEM_FLAGS_VRAM|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE|KFD_IOC_ALLOC_MEM_FLAGS_COHERENT;

	args->handle = 0x99c70000000a;

	rc = do_ioctl(AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, args);
	printf("ioctl(-1, AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, {"
	       "va_addr=0x7fc38c600000, "
	       "size=0xf000, "
	       "mmap_offset=0x7fc38c600000, "
	       "gpu_id=0x99c7, "
	       "flags=KFD_IOC_ALLOC_MEM_FLAGS_VRAM|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE|KFD_IOC_ALLOC_MEM_FLAGS_COHERENT"
	       "}");
	if (rc >= 0)
		printf(" => {"
		       "handle=0x99c70000000a"
		       "}");
	printf(") = %s\n", errstr);

	args->va_addr = 0x7fc37e800000;
	args->size = 0x67c4000;
	args->mmap_offset = 0x7fc37e800000;
	args->gpu_id = 0x99c7;
	args->flags = KFD_IOC_ALLOC_MEM_FLAGS_VRAM|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE|KFD_IOC_ALLOC_MEM_FLAGS_COHERENT;

	args->handle = 0x99c70000000b;

	rc = do_ioctl(AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, args);
	printf("ioctl(-1, AMDKFD_IOC_ALLOC_MEMORY_OF_GPU, {"
	       "va_addr=0x7fc37e800000, "
	       "size=0x67c4000, "
	       "mmap_offset=0x7fc37e800000, "
	       "gpu_id=0x99c7, "
	       "flags=KFD_IOC_ALLOC_MEM_FLAGS_VRAM|KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE|KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE|KFD_IOC_ALLOC_MEM_FLAGS_NO_SUBSTITUTE|KFD_IOC_ALLOC_MEM_FLAGS_COHERENT"
	       "}");
	if (rc >= 0)
		printf(" => {"
		       "handle=0x99c70000000b"
		       "}");
	printf(") = %s\n", errstr);
}

#ifdef INJECT_RETVAL
static void
test_print_amdkfd_ioc_map_memory_to_gpu(void)
{
	long rc;

	do_ioctl(AMDKFD_IOC_MAP_MEMORY_TO_GPU, 0);
	printf("ioctl(-1, AMDKFD_IOC_MAP_MEMORY_TO_GPU, NULL) = %s\n", errstr);

	TAIL_ALLOC_OBJECT_CONST_PTR(struct kfd_ioctl_map_memory_to_gpu_args, args);

	do_ioctl(AMDKFD_IOC_MAP_MEMORY_TO_GPU, (char *) args + 1);
	printf("ioctl(-1, AMDKFD_IOC_MAP_MEMORY_TO_GPU, %p) = %s\n",
	       (char *) args + 1, errstr);

	args->handle = 0x99c700000000;
	args->device_ids_array_ptr = 0x55c10af54e50;
	args->n_devices = 1;
	args->n_success = 1;

	args->n_success = 1;

	rc = do_ioctl(AMDKFD_IOC_MAP_MEMORY_TO_GPU, args);
	printf("ioctl(-1, AMDKFD_IOC_MAP_MEMORY_TO_GPU, {"
	       "handle=0x99c700000000, "
	       "device_ids_array_ptr=0x55c10af54e50, "
	       "n_devices=1, "
	       "n_success=1"
	       "}");
	if (rc >= 0)
		printf(" => {"
		       "n_success=1"
		       "}");
	printf(") = %s\n", errstr);

	args->handle = 0x99c700000000;
	args->device_ids_array_ptr = 0x55c10af557a0;
	args->n_devices = 1;
	args->n_success = 1;

	args->n_success = 1;

	rc = do_ioctl(AMDKFD_IOC_MAP_MEMORY_TO_GPU, args);
	printf("ioctl(-1, AMDKFD_IOC_MAP_MEMORY_TO_GPU, {"
	       "handle=0x99c700000000, "
	       "device_ids_array_ptr=0x55c10af557a0, "
	       "n_devices=1, "
	       "n_success=1"
	       "}");
	if (rc >= 0)
		printf(" => {"
		       "n_success=1"
		       "}");
	printf(") = %s\n", errstr);
}
#endif


#ifdef INJECT_RETVAL
static void
test_print_amdkfd_ioc_unmap_memory_from_gpu(void)
{
	long rc;

	do_ioctl(AMDKFD_IOC_UNMAP_MEMORY_FROM_GPU, 0);
	printf("ioctl(-1, AMDKFD_IOC_UNMAP_MEMORY_FROM_GPU, NULL) = %s\n", errstr);

	TAIL_ALLOC_OBJECT_CONST_PTR(struct kfd_ioctl_unmap_memory_from_gpu_args, args);

	do_ioctl(AMDKFD_IOC_UNMAP_MEMORY_FROM_GPU, (char *) args + 1);
	printf("ioctl(-1, AMDKFD_IOC_UNMAP_MEMORY_FROM_GPU, %p) = %s\n",
	       (char *) args + 1, errstr);

	args->handle = 0x99c700000000;
	args->device_ids_array_ptr = 0x55c10af557a0;
	args->n_devices = 1;
	args->n_success = 1;

	args->n_success = 1;

	rc = do_ioctl(AMDKFD_IOC_UNMAP_MEMORY_FROM_GPU, args);
	printf("ioctl(-1, AMDKFD_IOC_UNMAP_MEMORY_FROM_GPU, {"
	       "handle=0x99c700000000, "
	       "device_ids_array_ptr=0x55c10af557a0, "
	       "n_devices=1, "
	       "n_success=1"
	       "}");
	if (rc >= 0)
		printf(" => {"
		       "n_success=1"
		       "}");
	printf(") = %s\n", errstr);

	args->handle = 0x99c70000003c;
	args->device_ids_array_ptr = 0x55c116c303f0;
	args->n_devices = 1;
	args->n_success = 1;

	args->n_success = 1;

	rc = do_ioctl(AMDKFD_IOC_UNMAP_MEMORY_FROM_GPU, args);
	printf("ioctl(-1, AMDKFD_IOC_UNMAP_MEMORY_FROM_GPU, {"
	       "handle=0x99c70000003c, "
	       "device_ids_array_ptr=0x55c116c303f0, "
	       "n_devices=1, "
	       "n_success=1"
	       "}");
	if (rc >= 0)
		printf(" => {"
		       "n_success=1"
		       "}");
	printf(") = %s\n", errstr);
}
#endif

#ifdef INJECT_RETVAL
static void
test_print_amdkfd_ioc_free_memory_of_gpu(void)
{
	do_ioctl(AMDKFD_IOC_FREE_MEMORY_OF_GPU, 0);
	printf("ioctl(-1, AMDKFD_IOC_FREE_MEMORY_OF_GPU, NULL) = %s\n", errstr);

	TAIL_ALLOC_OBJECT_CONST_PTR(struct kfd_ioctl_free_memory_of_gpu_args, args);

	do_ioctl(AMDKFD_IOC_FREE_MEMORY_OF_GPU, (char *) args + 1);
	printf("ioctl(-1, AMDKFD_IOC_FREE_MEMORY_OF_GPU, %p) = %s\n",
	       (char *) args + 1, errstr);

	args->handle = 0x99c700000000;

	do_ioctl(AMDKFD_IOC_FREE_MEMORY_OF_GPU, args);
	printf("ioctl(-1, AMDKFD_IOC_FREE_MEMORY_OF_GPU, {"
	       "handle=0x99c700000000"
	       "}) = %s\n", errstr);

	args->handle = 0x99c70000003c;

	do_ioctl(AMDKFD_IOC_FREE_MEMORY_OF_GPU, args);
	printf("ioctl(-1, AMDKFD_IOC_FREE_MEMORY_OF_GPU, {"
	       "handle=0x99c70000003c"
	       "}) = %s\n", errstr);
}
#endif

#ifdef INJECT_RETVAL
static void
test_print_amdkfd_ioc_set_xnack_mode(void)
{
	long rc;

	do_ioctl(AMDKFD_IOC_SET_XNACK_MODE, 0);
	printf("ioctl(-1, AMDKFD_IOC_SET_XNACK_MODE, NULL) = %s\n", errstr);

	TAIL_ALLOC_OBJECT_CONST_PTR(struct kfd_ioctl_set_xnack_mode_args, args);

	do_ioctl(AMDKFD_IOC_SET_XNACK_MODE, (char *) args + 1);
	printf("ioctl(-1, AMDKFD_IOC_SET_XNACK_MODE, %p) = %s\n",
	       (char *) args + 1, errstr);

	args->xnack_enabled = 0;

	args->xnack_enabled = 0;

	rc = do_ioctl(AMDKFD_IOC_SET_XNACK_MODE, args);
	printf("ioctl(-1, AMDKFD_IOC_SET_XNACK_MODE, {"
	       "xnack_enabled=0"
	       "}");
	if (rc >= 0)
		printf(" => {"
		       "xnack_enabled=0"
		       "}");
	printf(") = %s\n", errstr);

}
#endif

static void
test_print_amdkfd_ioc_get_clock_counters(void)
{
	long rc;

	do_ioctl(AMDKFD_IOC_GET_CLOCK_COUNTERS, 0);
	printf("ioctl(-1, AMDKFD_IOC_GET_CLOCK_COUNTERS, NULL) = %s\n", errstr);

	TAIL_ALLOC_OBJECT_CONST_PTR(struct kfd_ioctl_get_clock_counters_args, args);

	do_ioctl(AMDKFD_IOC_GET_CLOCK_COUNTERS, (char *) args + 1);
	printf("ioctl(-1, AMDKFD_IOC_GET_CLOCK_COUNTERS, %p) = %s\n",
	       (char *) args + 1, errstr);

	args->gpu_id = 0x99c7;

	args->gpu_clock_counter = 0x28883c642718;
	args->cpu_clock_counter = 0x195473a53b34a;
	args->system_clock_counter = 0x1954b08ca72af;
	args->system_clock_freq = 1000000000;

	rc = do_ioctl(AMDKFD_IOC_GET_CLOCK_COUNTERS, args);
	printf("ioctl(-1, AMDKFD_IOC_GET_CLOCK_COUNTERS, {"
	       "gpu_id=0x99c7"
	       "}");
	if (rc >= 0)
		printf(" => {"
		       "gpu_clock_counter=0x28883c642718, "
		       "cpu_clock_counter=0x195473a53b34a, "
		       "system_clock_counter=0x1954b08ca72af, "
		       "system_clock_freq=1000000000"
		       "}");
	printf(") = %s\n", errstr);

}

static void
test_print_amdkfd_ioc_create_event(void)
{
	long rc;

	do_ioctl(AMDKFD_IOC_CREATE_EVENT, 0);
	printf("ioctl(-1, AMDKFD_IOC_CREATE_EVENT, NULL) = %s\n", errstr);

	TAIL_ALLOC_OBJECT_CONST_PTR(struct kfd_ioctl_create_event_args, args);

	do_ioctl(AMDKFD_IOC_CREATE_EVENT, (char *) args + 1);
	printf("ioctl(-1, AMDKFD_IOC_CREATE_EVENT, %p) = %s\n",
	       (char *) args + 1, errstr);

	args->event_type = KFD_IOC_EVENT_MEMORY;
	args->auto_reset = 0;
	args->node_id = 0;

	args->event_page_offset = 0;
	args->event_trigger_data = 0x40000000;
	args->event_id = 0x40000000;
	args->event_slot_index = 0;

	rc = do_ioctl(AMDKFD_IOC_CREATE_EVENT, args);
	printf("ioctl(-1, AMDKFD_IOC_CREATE_EVENT, {"
	       "event_type=KFD_IOC_EVENT_MEMORY, "
	       "auto_reset=0, "
	       "node_id=0"
	       "}");
	if (rc >= 0)
		printf(" => {"
		       "event_page_offset=0, "
		       "event_trigger_data=0x40000000, "
		       "event_id=0x40000000, "
		       "event_slot_index=0"
		       "}");
	printf(") = %s\n", errstr);

	args->event_type = KFD_IOC_EVENT_SIGNAL;
	args->auto_reset = 0x1;
	args->node_id = 0;

	args->event_page_offset = 0x8000000000000000;
	args->event_trigger_data = 0x1;
	args->event_id = 0x1;
	args->event_slot_index = 0x1;

	rc = do_ioctl(AMDKFD_IOC_CREATE_EVENT, args);
	printf("ioctl(-1, AMDKFD_IOC_CREATE_EVENT, {"
	       "event_type=KFD_IOC_EVENT_SIGNAL, "
	       "auto_reset=0x1, "
	       "node_id=0"
	       "}");
	if (rc >= 0)
		printf(" => {"
		       "event_page_offset=0x8000000000000000, "
		       "event_trigger_data=0x1, "
		       "event_id=0x1, "
		       "event_slot_index=0x1"
		       "}");
	printf(") = %s\n", errstr);
}

static void
test_print_amdkfd_ioc_set_scratch_backing_va(void)
{
	do_ioctl(AMDKFD_IOC_SET_SCRATCH_BACKING_VA, 0);
	printf("ioctl(-1, AMDKFD_IOC_SET_SCRATCH_BACKING_VA, NULL) = %s\n", errstr);

	TAIL_ALLOC_OBJECT_CONST_PTR(struct kfd_ioctl_set_scratch_backing_va_args, args);

	do_ioctl(AMDKFD_IOC_SET_SCRATCH_BACKING_VA, (char *) args + 1);
	printf("ioctl(-1, AMDKFD_IOC_SET_SCRATCH_BACKING_VA, %p) = %s\n",
	       (char *) args + 1, errstr);

	args->va_addr = 0x7fed5700;
	args->gpu_id = 0x99c7;

	do_ioctl(AMDKFD_IOC_SET_SCRATCH_BACKING_VA, args);
	printf("ioctl(-1, AMDKFD_IOC_SET_SCRATCH_BACKING_VA, {"
	       "va_addr=0x7fed5700, "
	       "gpu_id=0x99c7"
	       "}) = %s\n", errstr);

}

static void
test_print_amdkfd_ioc_set_trap_handler(void)
{
	do_ioctl(AMDKFD_IOC_SET_TRAP_HANDLER, 0);
	printf("ioctl(-1, AMDKFD_IOC_SET_TRAP_HANDLER, NULL) = %s\n", errstr);

	TAIL_ALLOC_OBJECT_CONST_PTR(struct kfd_ioctl_set_trap_handler_args, args);

	do_ioctl(AMDKFD_IOC_SET_TRAP_HANDLER, (char *) args + 1);
	printf("ioctl(-1, AMDKFD_IOC_SET_TRAP_HANDLER, %p) = %s\n",
	       (char *) args + 1, errstr);

	args->tba_addr = 0x7fee5826f000;
	args->tma_addr = 0;
	args->gpu_id = 0x99c7;

	do_ioctl(AMDKFD_IOC_SET_TRAP_HANDLER, args);
	printf("ioctl(-1, AMDKFD_IOC_SET_TRAP_HANDLER, {"
	       "tba_addr=0x7fee5826f000, "
	       "tma_addr=0, "
	       "gpu_id=0x99c7"
	       "}) = %s\n", errstr);

}

static void
test_print_amdkfd_ioc_get_tile_config(void)
{
	long rc;

	do_ioctl(AMDKFD_IOC_GET_TILE_CONFIG, 0);
	printf("ioctl(-1, AMDKFD_IOC_GET_TILE_CONFIG, NULL) = %s\n", errstr);

	TAIL_ALLOC_OBJECT_CONST_PTR(struct kfd_ioctl_get_tile_config_args, args);

	do_ioctl(AMDKFD_IOC_GET_TILE_CONFIG, (char *) args + 1);
	printf("ioctl(-1, AMDKFD_IOC_GET_TILE_CONFIG, %p) = %s\n",
	       (char *) args + 1, errstr);

	args->tile_config_ptr = 0x7ffde97613e0;
	args->macro_tile_config_ptr = 0x7ffde9761480;
	args->num_tile_configs = 32;
	args->num_macro_tile_configs = 16;
	args->gpu_id = 0x99c7;

	args->num_tile_configs = 32;
	args->num_macro_tile_configs = 16;
	args->gb_addr_config = 1092;
	args->num_banks = 0;
	args->num_ranks = 0;

	rc = do_ioctl(AMDKFD_IOC_GET_TILE_CONFIG, args);
	printf("ioctl(-1, AMDKFD_IOC_GET_TILE_CONFIG, {"
	       "tile_config_ptr=0x7ffde97613e0, "
	       "macro_tile_config_ptr=0x7ffde9761480, "
	       "num_tile_configs=32, "
	       "num_macro_tile_configs=16, "
	       "gpu_id=0x99c7"
	       "}");
	if (rc >= 0)
		printf(" => {"
		       "num_tile_configs=32, "
		       "num_macro_tile_configs=16, "
		       "gb_addr_config=1092, "
		       "num_banks=0, "
		       "num_ranks=0"
		       "}");
	printf(") = %s\n", errstr);

}

static void
test_print_amdkfd_ioc_create_queue(void)
{
	long rc;

	do_ioctl(AMDKFD_IOC_CREATE_QUEUE, 0);
	printf("ioctl(-1, AMDKFD_IOC_CREATE_QUEUE, NULL) = %s\n", errstr);

	TAIL_ALLOC_OBJECT_CONST_PTR(struct kfd_ioctl_create_queue_args, args);

	do_ioctl(AMDKFD_IOC_CREATE_QUEUE, (char *) args + 1);
	printf("ioctl(-1, AMDKFD_IOC_CREATE_QUEUE, %p) = %s\n",
	       (char *) args + 1, errstr);

	args->ring_base_address = 0x7fee58269000;
	args->ring_size = 0x1000;
	args->gpu_id = 0x99c7;
	args->queue_type = KFD_IOC_QUEUE_TYPE_COMPUTE_AQL;
	args->queue_percentage = 100;
	args->queue_priority = 7;
	args->eop_buffer_address = 0x7fee58265000;
	args->eop_buffer_size = 0x1000;
	args->ctx_save_restore_address = 0x7fed54800000;
	args->ctx_save_restore_size = 0x1a97000;
	args->ctl_stack_size = 0x7000;

	args->write_pointer_address = 0x7fee5826b038;
	args->read_pointer_address = 0x7fee5826b080;
	args->doorbell_offset = 0xe671c00000000000;
	args->queue_id = 0;

	rc = do_ioctl(AMDKFD_IOC_CREATE_QUEUE, args);
	printf("ioctl(-1, AMDKFD_IOC_CREATE_QUEUE, {"
	       "ring_base_address=0x7fee58269000, "
	       "ring_size=0x1000, "
	       "gpu_id=0x99c7, "
	       "queue_type=KFD_IOC_QUEUE_TYPE_COMPUTE_AQL, "
	       "queue_percentage=100, "
	       "queue_priority=7, "
	       "eop_buffer_address=0x7fee58265000, "
	       "eop_buffer_size=0x1000, "
	       "ctx_save_restore_address=0x7fed54800000, "
	       "ctx_save_restore_size=0x1a97000, "
	       "ctl_stack_size=0x7000"
	       "}");
	if (rc >= 0)
		printf(" => {"
		       "write_pointer_address=0x7fee5826b038, "
		       "read_pointer_address=0x7fee5826b080, "
		       "doorbell_offset=0xe671c00000000000, "
		       "queue_id=0"
		       "}");
	printf(") = %s\n", errstr);

	args->ring_base_address = 0x7fed26a00000;
	args->ring_size = 0x100000;
	args->gpu_id = 0x99c7;
	args->queue_type = KFD_IOC_QUEUE_TYPE_SDMA;
	args->queue_percentage = 100;
	args->queue_priority = 15;
	args->eop_buffer_address = 0;
	args->eop_buffer_size = 0;
	args->ctx_save_restore_address = 0;
	args->ctx_save_restore_size = 0;
	args->ctl_stack_size = 0;

	args->write_pointer_address = 0x7fee57106008;
	args->read_pointer_address = 0x7fee57106010;
	args->doorbell_offset = 0xe671c00000000800;
	args->queue_id = 0x2;

	rc = do_ioctl(AMDKFD_IOC_CREATE_QUEUE, args);
	printf("ioctl(-1, AMDKFD_IOC_CREATE_QUEUE, {"
	       "ring_base_address=0x7fed26a00000, "
	       "ring_size=0x100000, "
	       "gpu_id=0x99c7, "
	       "queue_type=KFD_IOC_QUEUE_TYPE_SDMA, "
	       "queue_percentage=100, "
	       "queue_priority=15, "
	       "eop_buffer_address=0, "
	       "eop_buffer_size=0, "
	       "ctx_save_restore_address=0, "
	       "ctx_save_restore_size=0, "
	       "ctl_stack_size=0"
	       "}");
	if (rc >= 0)
		printf(" => {"
		       "write_pointer_address=0x7fee57106008, "
		       "read_pointer_address=0x7fee57106010, "
		       "doorbell_offset=0xe671c00000000800, "
		       "queue_id=0x2"
		       "}");
	printf(") = %s\n", errstr);
}

static void
test_print_amdkfd_ioc_set_event(void)
{
	do_ioctl(AMDKFD_IOC_SET_EVENT, 0);
	printf("ioctl(-1, AMDKFD_IOC_SET_EVENT, NULL) = %s\n", errstr);

	TAIL_ALLOC_OBJECT_CONST_PTR(struct kfd_ioctl_set_event_args, args);

	do_ioctl(AMDKFD_IOC_SET_EVENT, (char *) args + 1);
	printf("ioctl(-1, AMDKFD_IOC_SET_EVENT, %p) = %s\n",
	       (char *) args + 1, errstr);

	args->event_id = 0x1;

	do_ioctl(AMDKFD_IOC_SET_EVENT, args);
	printf("ioctl(-1, AMDKFD_IOC_SET_EVENT, {"
	       "event_id=0x1"
	       "}) = %s\n", errstr);

	args->event_id = 0x1;

	do_ioctl(AMDKFD_IOC_SET_EVENT, args);
	printf("ioctl(-1, AMDKFD_IOC_SET_EVENT, {"
	       "event_id=0x1"
	       "}) = %s\n", errstr);
}

static void
test_print_amdkfd_ioc_destroy_event(void)
{
	do_ioctl(AMDKFD_IOC_DESTROY_EVENT, 0);
	printf("ioctl(-1, AMDKFD_IOC_DESTROY_EVENT, NULL) = %s\n", errstr);

	TAIL_ALLOC_OBJECT_CONST_PTR(struct kfd_ioctl_destroy_event_args, args);

	do_ioctl(AMDKFD_IOC_DESTROY_EVENT, (char *) args + 1);
	printf("ioctl(-1, AMDKFD_IOC_DESTROY_EVENT, %p) = %s\n",
	       (char *) args + 1, errstr);

	args->event_id = 0x2;

	do_ioctl(AMDKFD_IOC_DESTROY_EVENT, args);
	printf("ioctl(-1, AMDKFD_IOC_DESTROY_EVENT, {"
	       "event_id=0x2"
	       "}) = %s\n", errstr);

	args->event_id = 0x40000000;

	do_ioctl(AMDKFD_IOC_DESTROY_EVENT, args);
	printf("ioctl(-1, AMDKFD_IOC_DESTROY_EVENT, {"
	       "event_id=0x40000000"
	       "}) = %s\n", errstr);

}

static void
test_print_amdkfd_ioc_destroy_queue(void)
{
	do_ioctl(AMDKFD_IOC_DESTROY_QUEUE, 0);
	printf("ioctl(-1, AMDKFD_IOC_DESTROY_QUEUE, NULL) = %s\n", errstr);

	TAIL_ALLOC_OBJECT_CONST_PTR(struct kfd_ioctl_destroy_queue_args, args);

	do_ioctl(AMDKFD_IOC_DESTROY_QUEUE, (char *) args + 1);
	printf("ioctl(-1, AMDKFD_IOC_DESTROY_QUEUE, %p) = %s\n",
	       (char *) args + 1, errstr);

	args->queue_id = 0x1;

	do_ioctl(AMDKFD_IOC_DESTROY_QUEUE, args);
	printf("ioctl(-1, AMDKFD_IOC_DESTROY_QUEUE, {"
	       "queue_id=0x1"
	       "}) = %s\n", errstr);

	args->queue_id = 0;

	do_ioctl(AMDKFD_IOC_DESTROY_QUEUE, args);
	printf("ioctl(-1, AMDKFD_IOC_DESTROY_QUEUE, {"
	       "queue_id=0"
	       "}) = %s\n", errstr);

}

int
main(int argc, char *argv[])
{
#ifdef INJECT_RETVAL
	unsigned long num_skip;
	bool locked = false;

	if (argc < 2)
		error_msg_and_fail("Usage: %s NUM_SKIP", argv[0]);

	num_skip = strtoul(argv[1], NULL, 0);

	for (size_t i = 0; i < num_skip; i++) {
		long ret = ioctl(-1, AMDKFD_IOC_GET_VERSION, 0);

		printf("ioctl(-1, AMDKFD_IOC_GET_VERSION, NULL) = %s%s\n",
		       sprintrc(ret),
		       ret == INJECT_RETVAL ? " (INJECTED)" : "");

		if (ret != INJECT_RETVAL)
			continue;

		locked = true;
		break;
	}

	if (!locked)
		error_msg_and_fail("Hasn't locked on ioctl(-1"
				   ", AMDKFD_IOC_GET_VERSION, NULL) returning %d",
				   INJECT_RETVAL);
#endif

	test_print_amdkfd_ioc_create_queue();
	test_print_amdkfd_ioc_destroy_queue();
	test_print_amdkfd_ioc_set_memory_policy();
	test_print_amdkfd_ioc_get_clock_counters();

	test_print_amdkfd_ioc_create_event();
	test_print_amdkfd_ioc_destroy_event();
	test_print_amdkfd_ioc_set_event();

	test_print_amdkfd_ioc_set_scratch_backing_va();
	test_print_amdkfd_ioc_get_tile_config();
	test_print_amdkfd_ioc_set_trap_handler();
	test_print_amdkfd_ioc_acquire_vm();
	test_print_amdkfd_ioc_alloc_memory_of_gpu();

#ifdef INJECT_RETVAL
	test_print_amdkfd_ioc_get_version();
	test_print_amdkfd_ioc_get_process_apertures_new();
	test_print_amdkfd_ioc_free_memory_of_gpu();
	test_print_amdkfd_ioc_map_memory_to_gpu();
	test_print_amdkfd_ioc_unmap_memory_from_gpu();
	test_print_amdkfd_ioc_set_xnack_mode();
#endif

	puts("+++ exited with 0 +++");
	return 0;
}
