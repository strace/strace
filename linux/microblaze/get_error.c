if (check_errno && is_negated_errno(microblaze_r3)) {
	tcp->u_rval = -1;
	tcp->u_error = -microblaze_r3;
} else {
	tcp->u_rval = microblaze_r3;
}
