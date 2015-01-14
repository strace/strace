BEGIN {
  fail = 0
  lines = 3
  cap = "(0|CAP_[A-Z_]+(\\|CAP_[A-Z_]+)*|CAP_[A-Z_]+(\\|CAP_[A-Z_]+){37}\\|0xffffffc0)"
  capget = "^capget\\(\\{_LINUX_CAPABILITY_VERSION_3, 0\\}, \\{" cap ", " cap ", " cap "\\}\\) = 0$"
}

NR == 1 {if (match($0, capget)) next}

NR == 2 && $0 == "capset({_LINUX_CAPABILITY_VERSION_3, 0}, {CAP_DAC_OVERRIDE|CAP_WAKE_ALARM, CAP_DAC_READ_SEARCH|CAP_BLOCK_SUSPEND, 0}) = -1 EPERM (Operation not permitted)" {next}

NR == lines && $0 == "+++ exited with 0 +++" {next}

{
  print "Line " NR " does not match."
  fail = 1
  exit 1
}

END {
  if (fail == 0 && NR != lines) {
    print "Expected " lines " lines, found " NR " line(s)."
    exit 1
  }
}
