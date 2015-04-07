#include "defs.h"

#include "xlat/whence_codes.h"

/* Linux kernel has exactly one version of lseek:
 * fs/read_write.c::SYSCALL_DEFINE3(lseek, unsigned, fd, off_t, offset, unsigned, origin)
 * In kernel, off_t is always the same as (kernel's) long
 * (see include/uapi/asm-generic/posix_types.h),
 * which means that on x32 we need to use tcp->ext_arg[N] to get offset argument.
 * Use test/x32_lseek.c to test lseek decoding.
 */
#if defined(LINUX_MIPSN32) || defined(X32)
SYS_FUNC(lseek)
{
	long long offset;
	int whence;

	if (entering(tcp)) {
		printfd(tcp, tcp->u_arg[0]);
		offset = tcp->ext_arg[1];
		whence = tcp->u_arg[2];
		if (whence == SEEK_SET)
			tprintf(", %llu, ", offset);
		else
			tprintf(", %lld, ", offset);
		printxval(whence_codes, whence, "SEEK_???");
	}
	return RVAL_LUDECIMAL;
}
#else
SYS_FUNC(lseek)
{
	long offset;
	int whence;

	if (entering(tcp)) {
		printfd(tcp, tcp->u_arg[0]);
		offset = tcp->u_arg[1];
		whence = tcp->u_arg[2];
		if (whence == SEEK_SET)
			tprintf(", %lu, ", offset);
		else
			tprintf(", %ld, ", offset);
		printxval(whence_codes, whence, "SEEK_???");
	}
	return RVAL_UDECIMAL;
}
#endif

/* llseek syscall takes explicitly two ulong arguments hi, lo,
 * rather than one 64-bit argument for which LONG_LONG works
 * appropriate for the native byte order.
 *
 * See kernel's fs/read_write.c::SYSCALL_DEFINE5(llseek, ...)
 *
 * hi,lo are "unsigned longs" and combined exactly this way in kernel:
 * ((loff_t) hi << 32) | lo
 * Note that for architectures with kernel's long wider than userspace long
 * (such as x32), combining code will use *kernel's*, i.e. *wide* longs
 * for hi and lo. We would need to use tcp->ext_arg[N] on x32...
 * ...however, x32 (and x86_64) does not _have_ llseek syscall as such.
 */
SYS_FUNC(llseek)
{
	if (entering(tcp)) {
		printfd(tcp, tcp->u_arg[0]);
		if (tcp->u_arg[4] == SEEK_SET)
			tprintf(", %llu, ",
				((long long) tcp->u_arg[1]) << 32 |
				(unsigned long long) (unsigned) tcp->u_arg[2]);
		else
			tprintf(", %lld, ",
				((long long) tcp->u_arg[1]) << 32 |
				(unsigned long long) (unsigned) tcp->u_arg[2]);
	}
	else {
		long long off;
		if (syserror(tcp) || umove(tcp, tcp->u_arg[3], &off) < 0)
			tprintf("%#lx, ", tcp->u_arg[3]);
		else
			tprintf("[%llu], ", off);
		printxval(whence_codes, tcp->u_arg[4], "SEEK_???");
	}
	return 0;
}
