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
print_seg(const long seg_buf, const unsigned long seg_bufsz,
	  const long seg_mem, const unsigned long seg_memsz)
{
	tprints("{");
	printaddr(seg_buf);
	tprintf(", %lu, ", seg_bufsz);
	printaddr(seg_mem);
	tprintf(", %lu}", seg_memsz);
}

static void
print_kexec_segments(struct tcb *tcp, const unsigned long addr,
		     const unsigned long len)
{
#if SUPPORTED_PERSONALITIES > 1
	union {
		struct { u_int32_t buf, bufsz, mem, memsz; } seg32;
		struct { u_int64_t buf, bufsz, mem, memsz; } seg64;
	} seg;
# define sizeof_seg \
	(current_wordsize == 4 ? sizeof(seg.seg32) : sizeof(seg.seg64))
# define seg_buf \
	(current_wordsize == 4 ? (uint64_t) seg.seg32.buf : seg.seg64.buf)
# define seg_bufsz \
	(current_wordsize == 4 ? (uint64_t) seg.seg32.bufsz : seg.seg64.bufsz)
# define seg_mem \
	(current_wordsize == 4 ? (uint64_t) seg.seg32.mem : seg.seg64.mem)
# define seg_memsz \
	(current_wordsize == 4 ? (uint64_t) seg.seg32.memsz : seg.seg64.memsz)
#else
	struct {
		void *buf;
		size_t bufsz;
		void *mem;
		size_t memsz;
	} seg;
# define sizeof_seg sizeof(seg)
# define seg_buf seg.buf
# define seg_bufsz seg.bufsz
# define seg_mem seg.mem
# define seg_memsz seg.memsz
#endif
	unsigned int i;

	if (!len) {
		tprints("[]");
		return;
	}

	if (len > KEXEC_SEGMENT_MAX) {
		printaddr(addr);
		return;
	}

	if (umoven_or_printaddr(tcp, addr, sizeof_seg, &seg))
		return;

	tprints("[");
	print_seg((unsigned long) seg_buf, seg_bufsz,
		  (unsigned long) seg_mem, seg_memsz);

	for (i = 1; i < len; ++i) {
		tprints(", ");
		if (umoven_or_printaddr(tcp, addr + i * sizeof_seg,
					sizeof_seg, &seg))
			break;
		print_seg((unsigned long) seg_buf, seg_bufsz,
			  (unsigned long) seg_mem, seg_memsz);
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
