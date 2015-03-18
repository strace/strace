# s[] is array of match strings
# r[] is array of match patterns

NR > lines { next }

{
	if (s[NR]) {
		if ($0 == s[NR])
			next
		print "Line " NR " does not match expected string: " s[NR]
	} else {
		if (match($0, r[NR]))
			next
		print "Line " NR " does not match expected pattern: " r[NR]
	}

	fail = 1
}

END {
	if (fail == 0 && NR != lines) {
		fail = 1
		print "Expected " lines " lines, found " NR " line(s)."
	}
	exit fail
}
