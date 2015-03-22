if (tcp->currpers == 1) {
	if (check_errno && is_negated_errno(aarch64_regs.regs[0])) {
		tcp->u_rval = -1;
		tcp->u_error = -aarch64_regs.regs[0];
	} else {
		tcp->u_rval = aarch64_regs.regs[0];
	}
} else {
#include "arm/get_error.c"
}
