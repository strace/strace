if (x86_io.iov_len != sizeof(i386_regs)) {
	/* x86-64 or x32 ABI */
	tcp->u_arg[0] = x86_64_regs.rdi;
	tcp->u_arg[1] = x86_64_regs.rsi;
	tcp->u_arg[2] = x86_64_regs.rdx;
	tcp->u_arg[3] = x86_64_regs.r10;
	tcp->u_arg[4] = x86_64_regs.r8;
	tcp->u_arg[5] = x86_64_regs.r9;
#ifdef X32
	tcp->ext_arg[0] = x86_64_regs.rdi;
	tcp->ext_arg[1] = x86_64_regs.rsi;
	tcp->ext_arg[2] = x86_64_regs.rdx;
	tcp->ext_arg[3] = x86_64_regs.r10;
	tcp->ext_arg[4] = x86_64_regs.r8;
	tcp->ext_arg[5] = x86_64_regs.r9;
#endif
} else {
	/* i386 ABI */
	/* Zero-extend from 32 bits */
	/* Use widen_to_long(tcp->u_arg[N]) in syscall handlers
	 * if you need to use *sign-extended* parameter.
	 */
	tcp->u_arg[0] = (long)(uint32_t)i386_regs.ebx;
	tcp->u_arg[1] = (long)(uint32_t)i386_regs.ecx;
	tcp->u_arg[2] = (long)(uint32_t)i386_regs.edx;
	tcp->u_arg[3] = (long)(uint32_t)i386_regs.esi;
	tcp->u_arg[4] = (long)(uint32_t)i386_regs.edi;
	tcp->u_arg[5] = (long)(uint32_t)i386_regs.ebp;
}
