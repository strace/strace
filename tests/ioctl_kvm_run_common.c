/*
 * Check decoding of KVM_* commands of ioctl syscall using /dev/kvm API.
 * Based on kvmtest.c from https://lwn.net/Articles/658512/
 *
 * kvmtest.c author: Josh Triplett <josh@joshtriplett.org>
 * Copyright (c) 2015 Intel Corporation
 * Copyright (c) 2017-2021 The strace developers.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "tests.h"

#if defined HAVE_LINUX_KVM_H				\
 && defined HAVE_STRUCT_KVM_CPUID2			\
 && defined HAVE_STRUCT_KVM_REGS			\
 && defined HAVE_STRUCT_KVM_SREGS			\
 && defined HAVE_STRUCT_KVM_USERSPACE_MEMORY_REGION	\
 &&(defined __x86_64__ || defined __i386__)

# include <fcntl.h>
# include <stdint.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <sys/ioctl.h>
# include <sys/mman.h>
# include <unistd.h>
# include <linux/kvm.h>

# ifndef KVM_MAX_CPUID_ENTRIES
#  define KVM_MAX_CPUID_ENTRIES 80
# endif

# include "xmalloc.h"
# include "xlat.h"
# include "xlat/kvm_cpuid_flags.h"

static int
kvm_ioctl(int fd, unsigned long cmd, const char *cmd_str, void *arg)
{
	int rc = ioctl(fd, cmd, arg);
	if (rc < 0)
		perror_msg_and_skip("%s", cmd_str);
	return rc;
}

# define KVM_IOCTL(fd_, cmd_, arg_)	\
	kvm_ioctl((fd_), (cmd_), #cmd_, (arg_))

static const char dev[] = "/dev/kvm";
static const char vm_dev[] = "anon_inode:kvm-vm";
static char vcpu_dev[] = "anon_inode:kvm-vcpu:0";
static size_t page_size;

extern const char code[];
extern const unsigned short code_size;

__asm__(
	".type code, @object		\n"
	"code:				\n"
	"	mov $0xd80003f8, %edx	\n"
	"	mov $'\n', %al		\n"
	"	out %al, (%dx)		\n"
	"	hlt			\n"
	".size code, . - code		\n"
	".type code_size, @object	\n"
	"code_size:			\n"
	"	.short . - code		\n"
	".size code_size, . - code_size	\n"
	);

static void
print_kvm_segment(const struct kvm_segment *seg)
{
	printf("{base=%#jx, limit=%u, selector=%u, type=%u, present=%u, "
	       "dpl=%u, db=%u, s=%u, l=%u, g=%u, avl=%u}",
	       (uintmax_t) seg->base, seg->limit, seg->selector, seg->type,
	       seg->present, seg->dpl, seg->db, seg->s, seg->l, seg->g,
	       seg->avl);
}

static void
print_kvm_sregs(const struct kvm_sregs *sregs)
{
	printf("{cs=");
	print_kvm_segment(&sregs->cs);
# if VERBOSE
	printf(", ds=");
	print_kvm_segment(&sregs->ds);
	printf(", es=");
	print_kvm_segment(&sregs->es);
	printf(", fs=");
	print_kvm_segment(&sregs->fs);
	printf(", gs=");
	print_kvm_segment(&sregs->gs);
	printf(", ss=");
	print_kvm_segment(&sregs->ss);
	printf(", tr=");
	print_kvm_segment(&sregs->tr);
	printf(", ldt=");
	print_kvm_segment(&sregs->ldt);
	printf(", gdt={base=%#jx, limit=%u}, idt={base=%#jx, limit=%u}, "
	      "cr0=%llu, cr2=%llu, cr3=%llu, cr4=%llu, cr8=%llu, efer=%llu, "
	      "apic_base=%#jx", (uintmax_t) sregs->gdt.base, sregs->gdt.limit,
	      (uintmax_t) sregs->idt.base, sregs->idt.limit, sregs->cr0,
	      sregs->cr2, sregs->cr3, sregs->cr4, sregs->cr8, sregs->efer,
	      (uintmax_t)sregs->apic_base);
	printf(", interrupt_bitmap=[");
	for (size_t i = 0; i < ARRAY_SIZE(sregs->interrupt_bitmap); i++) {
		if (i)
			printf(", ");
		printf("%#jx", (uintmax_t) sregs->interrupt_bitmap[i]);
	}
	printf("]");
# else
	printf(", ...");
# endif
	printf("}");
}

static void
print_kvm_regs(const struct kvm_regs *regs)
{
	printf("{rax=%#jx", (uintmax_t) regs->rax);
# if VERBOSE
	printf(", rbx=%#jx, rcx=%#jx, rdx=%#jx, rsi=%#jx, rdi=%#jx",
	       (uintmax_t) regs->rbx, (uintmax_t) regs->rcx,
	       (uintmax_t) regs->rdx, (uintmax_t) regs->rsi,
	       (uintmax_t) regs->rdi);
# else
	printf(", ...");
# endif
	printf(", rsp=%#jx, rbp=%#jx", (uintmax_t) regs->rsp,
	       (uintmax_t) regs->rbp);
# if VERBOSE
	printf(", r8=%#jx, r9=%#jx, r10=%#jx, r11=%#jx, r12=%#jx, r13=%#jx"
	       ", r14=%#jx, r15=%#jx",
	       (uintmax_t) regs->r8, (uintmax_t) regs->r9,
	       (uintmax_t) regs->r10, (uintmax_t) regs->r11,
	       (uintmax_t) regs->r12, (uintmax_t) regs->r13,
	       (uintmax_t) regs->r14, (uintmax_t) regs->r15);
# else
	printf(", ...");
# endif
	printf(", rip=%#jx, rflags=%#jx}", (uintmax_t) regs->rip,
	       (uintmax_t) regs->rflags);
}

# define need_print_KVM_RUN 1

static void
print_KVM_RUN(const int fd, const char *const dev, const unsigned int reason);

static void
run_kvm(const int vcpu_fd, struct kvm_run *const run, const size_t mmap_size,
	void *const mem)
{
	/* Initialize CS to point at 0, via a read-modify-write of sregs. */
	struct kvm_sregs sregs;
	KVM_IOCTL(vcpu_fd, KVM_GET_SREGS, &sregs);
	printf("ioctl(%d<%s>, KVM_GET_SREGS, ", vcpu_fd, vcpu_dev);
	print_kvm_sregs(&sregs);
	printf(") = 0\n");

	sregs.cs.base = 0;
	sregs.cs.selector = 0;
	KVM_IOCTL(vcpu_fd, KVM_SET_SREGS, &sregs);
	printf("ioctl(%d<%s>, KVM_SET_SREGS, ", vcpu_fd, vcpu_dev);
	print_kvm_sregs(&sregs);
	printf(") = 0\n");

	/*
	 * Initialize registers: instruction pointer for our code, addends,
	 * and initial flags required by x86 architecture.
	 */
	struct kvm_regs regs = {
		.rip = page_size,
		.rax = 2,
		.rbx = 2,
		.rflags = 0x2,
	};
	KVM_IOCTL(vcpu_fd, KVM_SET_REGS, &regs);
	printf("ioctl(%d<%s>, KVM_SET_REGS, ", vcpu_fd, vcpu_dev);
	print_kvm_regs(&regs);
	printf(") = 0\n");

	/* Copy the code */
	memcpy(mem, code, code_size);

	const char *p = "\n";

	/* Repeatedly run code and handle VM exits. */
	for (;;) {
		KVM_IOCTL(vcpu_fd, KVM_RUN, NULL);
		print_KVM_RUN(vcpu_fd, vcpu_dev, run->exit_reason);

		switch (run->exit_reason) {
		case KVM_EXIT_HLT:
			if (p)
				error_msg_and_fail("premature KVM_EXIT_HLT");
			return;
		case KVM_EXIT_IO:
			if (run->io.direction == KVM_EXIT_IO_OUT
			    && run->io.size == 1
			    && run->io.port == 0x03f8
			    && run->io.count == 1
			    && run->io.data_offset < mmap_size
			    && p && *p == ((char *) run)[run->io.data_offset])
				p = NULL;
			else
				error_msg_and_fail("unhandled KVM_EXIT_IO");
			break;
		case KVM_EXIT_MMIO:
			error_msg_and_fail("Got an unexpected MMIO exit:"
					   " phys_addr %#llx,"
					   " data %02x %02x %02x %02x"
						" %02x %02x %02x %02x,"
					   " len %u, is_write %hhu",
					   (unsigned long long) run->mmio.phys_addr,
					   run->mmio.data[0], run->mmio.data[1],
					   run->mmio.data[2], run->mmio.data[3],
					   run->mmio.data[4], run->mmio.data[5],
					   run->mmio.data[6], run->mmio.data[7],
					   run->mmio.len, run->mmio.is_write);
		case KVM_EXIT_FAIL_ENTRY:
			error_msg_and_fail("Got an unexpected FAIL_ENTRY exit:"
					   " hardware_entry_failure_reason %" PRI__x64,
					   run->fail_entry.hardware_entry_failure_reason);

		default:
			error_msg_and_fail("exit_reason = %#x",
					   run->exit_reason);
		}
	}
}

static int
vcpu_dev_should_have_cpuid(int fd)
{
	int r = 0;
	char *proc = xasprintf("/proc/self/fd/%u", fd);
	char buf[sizeof(vcpu_dev)];

	if (readlink(proc, buf, sizeof(buf)) == sizeof(buf) - 1
	    && (memcmp(buf, vcpu_dev, sizeof(buf) - 1) == 0))
		r = 1;
	free(proc);
	return r;
}

static void
print_cpuid_ioctl(int fd, const char *fd_dev,
		  const char *ioctl_name, const struct kvm_cpuid2 *cpuid)
{
	printf("ioctl(%d<%s>, %s, {nent=%u, entries=[",
	       fd, fd_dev, ioctl_name, cpuid->nent);
# if VERBOSE
	for (size_t i = 0; i < cpuid->nent; i++) {
		if (i)
			printf(", ");
		printf("{function=%#x, index=%#x, flags=",
		       cpuid->entries[i].function, cpuid->entries[i].index);
		printflags(kvm_cpuid_flags, cpuid->entries[i].flags,
			   "KVM_CPUID_FLAG_???");
		printf(", eax=%#x, ebx=%#x, ecx=%#x, edx=%#x}",
		       cpuid->entries[i].eax, cpuid->entries[i].ebx,
		       cpuid->entries[i].ecx, cpuid->entries[i].edx);
	}
# else
	if (cpuid->nent)
		printf("...");
# endif
	printf("]}) = 0\n");
}

int
main(void)
{
	skip_if_unavailable("/proc/self/fd/");

	int kvm = open(dev, O_RDWR);
	if (kvm < 0)
		perror_msg_and_skip("open: %s", dev);

	/* Make sure we have the stable version of the API */
	int ret = KVM_IOCTL(kvm, KVM_GET_API_VERSION, 0);
	if (ret != KVM_API_VERSION)
		error_msg_and_skip("KVM_GET_API_VERSION returned %d"
				   ", KVM_API_VERSION is %d",
				   kvm, KVM_API_VERSION);
	printf("ioctl(%d<%s>, KVM_GET_API_VERSION, 0) = %d\n",
	       kvm, dev, ret);

	ret = KVM_IOCTL(kvm, KVM_CHECK_EXTENSION,
			(void *) (uintptr_t) KVM_CAP_USER_MEMORY);
	printf("ioctl(%d<%s>, KVM_CHECK_EXTENSION, KVM_CAP_USER_MEMORY) = %d\n",
	       kvm, dev, ret);

	int vm_fd = KVM_IOCTL(kvm, KVM_CREATE_VM, 0);
	printf("ioctl(%d<%s>, KVM_CREATE_VM, 0) = %d<%s>\n",
	       kvm, dev, vm_fd, vm_dev);

	/* Allocate one aligned page of guest memory to hold the code. */
	page_size = get_page_size();
	void *const mem = mmap(NULL, page_size, PROT_READ | PROT_WRITE,
				  MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	if (mem == MAP_FAILED)
		perror_msg_and_fail("mmap page");

	/* Map it to the second page frame (to avoid the real-mode IDT at 0). */
	struct kvm_userspace_memory_region region = {
		.slot = 0,
		.guest_phys_addr = page_size,
		.memory_size = page_size,
		.userspace_addr = (uintptr_t) mem,
	};
	KVM_IOCTL(vm_fd, KVM_SET_USER_MEMORY_REGION, &region);
	printf("ioctl(%d<%s>, KVM_SET_USER_MEMORY_REGION"
	       ", {slot=0, flags=0, guest_phys_addr=%#lx, memory_size=%lu"
	       ", userspace_addr=%p}) = 0\n", vm_fd, vm_dev,
	       (unsigned long) page_size, (unsigned long) page_size, mem);

	int vcpu_fd = KVM_IOCTL(vm_fd, KVM_CREATE_VCPU, NULL);
	if (!vcpu_dev_should_have_cpuid(vcpu_fd)) {
		/*
		 * This is an older kernel that doesn't place a cpuid
		 * at the end of the dentry associated with vcpu_fd.
		 * Trim the cpuid part of vcpu_dev like:
		 * "anon_inode:kvm-vcpu:0" -> "anon_inode:kvm-vcpu"
		 */
		vcpu_dev[strlen (vcpu_dev) - 2] = '\0';
# ifdef KVM_NO_CPUID_CALLBACK
		KVM_NO_CPUID_CALLBACK;
# endif
	}

	printf("ioctl(%d<%s>, KVM_CREATE_VCPU, 0) = %d<%s>\n",
	       vm_fd, vm_dev, vcpu_fd, vcpu_dev);

	/* Map the shared kvm_run structure and following data. */
	ret = KVM_IOCTL(kvm, KVM_GET_VCPU_MMAP_SIZE, NULL);
	struct kvm_run *run;
	if (ret < (int) sizeof(*run))
		error_msg_and_fail("KVM_GET_VCPU_MMAP_SIZE returned %d < %d",
				   ret, (int) sizeof(*run));
	printf("ioctl(%d<%s>, KVM_GET_VCPU_MMAP_SIZE, 0) = %d\n",
	       kvm, dev, ret);

	const size_t mmap_size = (ret + page_size - 1) & -page_size;
	run = mmap(NULL, mmap_size, PROT_READ | PROT_WRITE,
		   MAP_SHARED, vcpu_fd, 0);
	if (run == MAP_FAILED)
		perror_msg_and_fail("mmap vcpu");

	size_t cpuid_nent = KVM_MAX_CPUID_ENTRIES;
	struct kvm_cpuid2 *cpuid = tail_alloc(sizeof(*cpuid) +
					      cpuid_nent *
					      sizeof(*cpuid->entries));

	cpuid->nent = 0;
	ioctl(kvm, KVM_GET_SUPPORTED_CPUID, cpuid);
	printf("ioctl(%d<%s>, KVM_GET_SUPPORTED_CPUID, %p) = -1 E2BIG (%m)\n",
	       kvm, dev, cpuid);

	cpuid->nent = cpuid_nent;

	KVM_IOCTL(kvm, KVM_GET_SUPPORTED_CPUID, cpuid);
	print_cpuid_ioctl(kvm, dev, "KVM_GET_SUPPORTED_CPUID", cpuid);

	struct kvm_cpuid2 cpuid_tmp = { .nent = 0 };
	KVM_IOCTL(vcpu_fd, KVM_SET_CPUID2, &cpuid_tmp);
	printf("ioctl(%d<%s>, KVM_SET_CPUID2, {nent=%u, entries=[]}) = 0\n",
	       vcpu_fd, vcpu_dev, cpuid_tmp.nent);

	KVM_IOCTL(vcpu_fd, KVM_SET_CPUID2, cpuid);
	print_cpuid_ioctl(vcpu_fd, vcpu_dev, "KVM_SET_CPUID2", cpuid);

	ioctl(vcpu_fd, KVM_SET_CPUID2, NULL);
	printf("ioctl(%d<%s>, KVM_SET_CPUID2, NULL) = -1 EFAULT (%m)\n",
	       vcpu_fd, vcpu_dev);

	run_kvm(vcpu_fd, run, mmap_size, mem);

	puts("+++ exited with 0 +++");
	return 0;
}

#else /* !HAVE_LINUX_KVM_H */

SKIP_MAIN_UNDEFINED("HAVE_LINUX_KVM_H && HAVE_STRUCT_KVM_CPUID2 && "
		    "HAVE_STRUCT_KVM_REGS && HAVE_STRUCT_KVM_SREGS && "
		    "HAVE_STRUCT_KVM_USERSPACE_MEMORY_REGION && "
		    "(__x86_64__ || __i386__)")

# define need_print_KVM_RUN 0

#endif
