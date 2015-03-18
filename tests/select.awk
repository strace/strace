BEGIN {
	r[1] = "^p?select6?\\(2, \\[0 1\\], \\[0 1\\], \\[0 1\\], NULL(, 0)?\\) += 1 \\(\\)$"
	r[2] = "^p?select6?\\(-1, NULL, 0x[0-9a-f]+, NULL, NULL(, 0)?\\) += -1 "
	r[3] = "^p?select6?\\(1025, \\[0\\], \\[\\], NULL, \\{0, 100(000)?\\}(, 0)?\\) += 0 \\(Timeout\\)$"
	s[4] = "+++ exited with 0 +++"

	lines = 4
	fail = 0
}

@include "match.awk"
