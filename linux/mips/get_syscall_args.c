#if defined LINUX_MIPSN64
	tcp->u_arg[0] = mips_REG_A0;
	tcp->u_arg[1] = mips_REG_A1;
	tcp->u_arg[2] = mips_REG_A2;
	tcp->u_arg[3] = mips_REG_A3;
	tcp->u_arg[4] = mips_REG_A4;
	tcp->u_arg[5] = mips_REG_A5;
#elif defined LINUX_MIPSN32
	tcp->u_arg[0] = tcp->ext_arg[0] = mips_REG_A0;
	tcp->u_arg[1] = tcp->ext_arg[1] = mips_REG_A1;
	tcp->u_arg[2] = tcp->ext_arg[2] = mips_REG_A2;
	tcp->u_arg[3] = tcp->ext_arg[3] = mips_REG_A3;
	tcp->u_arg[4] = tcp->ext_arg[4] = mips_REG_A4;
	tcp->u_arg[5] = tcp->ext_arg[5] = mips_REG_A5;
#elif defined LINUX_MIPSO32
	tcp->u_arg[0] = mips_REG_A0;
	tcp->u_arg[1] = mips_REG_A1;
	tcp->u_arg[2] = mips_REG_A2;
	tcp->u_arg[3] = mips_REG_A3;
	if (tcp->s_ent->nargs > 4) {
		umoven(tcp, mips_REG_SP + 4 * 4,
		       (tcp->s_ent->nargs - 4) * sizeof(tcp->u_arg[0]),
		       &tcp->u_arg[4]);
	}
#else
# error unsupported mips abi
#endif
