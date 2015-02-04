BEGIN {
	r[1] = "^getrandom\\(\"(\\\\x[0-9a-f][0-9a-f]){3}\", 3, 0\\) += 3$"
	r[2] = "^getrandom\\(\"(\\\\x[0-9a-f][0-9a-f]){3}\"\\.\\.\\., 4, GRND_NONBLOCK\\) += 4$"
	r[3] = "^getrandom\\(0x[[0-9a-f]+, 4, GRND_NONBLOCK\\|GRND_RANDOM\\|0x3000\\) += -1 "
	r[4] = "^\\+\\+\\+ exited with 0 \\+\\+\\+$"
	lines = 4
	fail = 0
}

NR > lines { exit 1 }

{
	if (match($0, r[NR]))
		next

	print "Line " NR " does not match."
	fail = 1
}

END {
	if (fail == 0 && NR != lines) {
		fail = 1
		print "Expected " lines " lines, found " NR " line(s)."
	}
	exit fail
}
