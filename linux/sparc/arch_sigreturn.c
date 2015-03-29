long fp = sparc_regs.u_regs[U_REG_FP] + sizeof(struct sparc_stackf);
struct {
	struct pt_regs si_regs;
	int si_mask;
	void *fpu_save;
	long insns[2] ATTRIBUTE_ALIGNED(8);
	unsigned int extramask[NSIG / 8 / sizeof(int) - 1];
} frame;

if (umove(tcp, fp, &frame) < 0) {
	tprintf("{mask=%#lx}", fp);
} else {
	unsigned int mask[NSIG / 8 / sizeof(int)];

	mask[0] = frame.si_mask;
	memcpy(mask + 1, frame.extramask, sizeof(frame.extramask));
	tprintsigmask_addr("{mask=", mask);
	tprints("}");
}
