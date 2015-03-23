/*
 * On i386, sigcontext is followed on stack by struct fpstate
 * and after it an additional u32 extramask which holds
 * upper half of the mask.
 */
struct {
	uint32_t struct_sigcontext_padding1[20];
	uint32_t oldmask;
	uint32_t struct_sigcontext_padding2;
	uint32_t struct_fpstate_padding[156];
	uint32_t extramask;
} frame;

if (umove(tcp, *i386_esp_ptr, &frame) < 0) {
	tprintf("{mask=%#lx}", (unsigned long) *i386_esp_ptr);
} else {
	uint32_t mask[2] = { frame.oldmask, frame.extramask };
	tprintsigmask_addr("{mask=", mask);
	tprints("}");
}
