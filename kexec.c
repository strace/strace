#include "defs.h"

#include "xlat/kexec_load_flags.h"
#include "xlat/kexec_arch_values.h"

#ifndef KEXEC_ARCH_MASK
# define KEXEC_ARCH_MASK 0xffff0000
#endif
#ifndef KEXEC_SEGMENT_MAX
# define KEXEC_SEGMENT_MAX 16
#endif

static void
print_seg(const unsigned long *seg)
{
	tprints("{");
	printaddr(seg[0]);
	tprintf(", %lu, ", seg[1]);
	printaddr(seg[2]);
	tprintf(", %lu}", seg[3]);
}

static void
print_kexec_segments(struct tcb *tcp, const unsigned long addr,
		     const unsigned long len)
{
	unsigned long seg[4];
	const size_t sizeof_seg = ARRAY_SIZE(seg) * current_wordsize;
	unsigned int i;

	if (!len) {
		tprints("[]");
		return;
	}

	if (len > KEXEC_SEGMENT_MAX) {
		printaddr(addr);
		return;
	}

	if (umove_ulong_array_or_printaddr(tcp, addr, seg, ARRAY_SIZE(seg)))
		return;

	tprints("[");
	print_seg(seg);

	for (i = 1; i < len; ++i) {
		tprints(", ");
		if (umove_ulong_array_or_printaddr(tcp,
						   addr + i * sizeof_seg,
						   seg, ARRAY_SIZE(seg)))
			break;
		print_seg(seg);
	}

	tprints("]");
}

SYS_FUNC(kexec_load)
{
	unsigned long n;

	/* entry, nr_segments */
	printaddr(tcp->u_arg[0]);
	tprintf(", %lu, ", tcp->u_arg[1]);

	/* segments */
	print_kexec_segments(tcp, tcp->u_arg[2], tcp->u_arg[1]);
	tprints(", ");

	/* flags */
	n = tcp->u_arg[3];
	printxval(kexec_arch_values, n & KEXEC_ARCH_MASK, "KEXEC_ARCH_???");
	n &= ~KEXEC_ARCH_MASK;
	if (n) {
		tprints("|");
		printflags(kexec_load_flags, n, "KEXEC_???");
	}

	return RVAL_DECODED;
}
