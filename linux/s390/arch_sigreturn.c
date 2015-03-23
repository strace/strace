long mask[NSIG / 8 / sizeof(long)];
const long addr = *s390_frame_ptr + __SIGNAL_FRAMESIZE;

if (umove(tcp, addr, &mask) < 0) {
	tprintf("{mask=%#lx}", addr);
} else {
#ifdef S390
	long v = mask[0];
	mask[0] = mask[1];
	mask[1] = v;
#endif
	tprintsigmask_addr("{mask=", mask);
	tprints("}");
}
