BEGIN {
	cap = "(0|CAP_[A-Z_]+(\\|CAP_[A-Z_]+)*|CAP_[A-Z_]+(\\|CAP_[A-Z_]+){37}\\|0xffffffc0)"
	r[1] = "^capget\\(\\{_LINUX_CAPABILITY_VERSION_3, 0\\}, \\{" cap ", " cap ", " cap "\\}\\) = 0$"
	capset_data = "{CAP_DAC_OVERRIDE|CAP_WAKE_ALARM, CAP_DAC_READ_SEARCH|CAP_BLOCK_SUSPEND, 0}"
	s[2] = "capset({_LINUX_CAPABILITY_VERSION_3, 0}, " capset_data ") = -1 EPERM (Operation not permitted)"
	s[3] = "+++ exited with 0 +++"

	lines = 3
	fail = 0
}

@include "match.awk"
