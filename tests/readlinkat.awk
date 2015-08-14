BEGIN {
	pathname = "\\\\x72\\\\x65\\\\x61\\\\x64\\\\x6c\\\\x69\\\\x6e\\\\x6b\\\\x61\\\\x74\\\\x2e\\\\x6c\\\\x69\\\\x6e\\\\x6b"
	buf = "\\\\x72\\\\x65\\\\x61"
	r[1] = "^readlinkat\\(AT_FDCWD, \"" pathname "\", \"" buf "\"\\.\\.\\., 31\\) += 12"
	r[2] = "^\\+\\+\\+ exited with 0 \\+\\+\\+$"
	lines = 2
	fail = 0
}

@include "match.awk"
