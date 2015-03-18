BEGIN {
	n1[1] = "SIG_IGN, \\[HUP INT\\], SA_RESTORER\\|SA_RESTART, 0x[0-9a-f]+"
	n2[1] = "SIG_IGN, \\[HUP INT\\], SA_RESTART"

	n1[2] = "0x[0-9a-f]+, \\[QUIT TERM\\], SA_RESTORER\\|SA_SIGINFO, 0x[0-9a-f]+"
	n2[2] = "0x[0-9a-f]+, \\[QUIT TERM\\], SA_SIGINFO"

	n1[3] = "SIG_DFL, \\[\\], SA_RESTORER, 0x[0-9a-f]+"
	n2[3] = "SIG_DFL, \\[\\], 0"

	n1[4] = "SIG_DFL, ~\\[HUP( ((RT|SIGRT)[^] ]+|[3-9][0-9]|1[0-9][0-9]))*\\], SA_RESTORER, 0x[0-9a-f]+"
	n2[4] = "SIG_DFL, ~\\[HUP( ((RT|SIGRT)[^] ]+|[3-9][0-9]|1[0-9][0-9]))*\\], 0"

	o1[1] = o2[1] = "SIG_DFL, \\[\\], 0"

	for (i = 2; i < 5; i++) {
		o1[i] = n1[i - 1]
		o2[i] = n2[i - 1]
	}

	a1 = "(0x[0-9a-f]+, )?(4|8|16)"
	a2 = "(4|8|16)(, 0x[0-9a-f]+)?"
	a3 = "0x[0-9a-f]+, (4|8|16)"

	for (i = 1; i < 5; i++) {
		r[i] = "^rt_sigaction\\(SIGUSR2, (" \
			"\\{" n1[i] "\\}, \\{" o1[i] "\\}, " a1 "|" \
			"\\{" n2[i] "\\}, \\{" o2[i] "\\}, " a2 "|" \
			"\\{" n2[i] "\\}, \\{" o2[i] "\\}, " a3 ")\\) = 0$"
	}
	s[5] = "+++ exited with 0 +++"

	lines = 5
	fail = 0
}

@include "match.awk"
