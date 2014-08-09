# rt_sigaction on ALPHA has 5 args: sig, act, oact, sigsetsize, restorer.
# rt_sigaction on SPARC has 5 args: sig, act, oact, restorer, sigsetsize.
# rt_sigaction on other architectures has 4 args: sig, act, oact, sigsetsize.
# Some architectures have SA_RESTORER, some don't;
# in particular, SPARC has and ALPHA hasn't.
#
# There are two regexps for each test:
# the 1st is for any architecture with SA_RESTORER, including SPARC;
# the 2nd is for any architecture without SA_RESTORER, including ALPHA;
# the 3rd is for any architecture without SA_RESTORER and swapped args.

BEGIN {
	lines = 5
	fail = 0
}

# Test 1.
NR == 1 && /^rt_sigaction\(SIGUSR2, {SIG_IGN, \[HUP INT\], SA_RESTORER\|SA_RESTART, 0x[0-9a-f]+}, {SIG_DFL, \[\], 0}, (0x[0-9a-f]+, )?(4|8|16)\) = 0$/ {next}
NR == 1 && /^rt_sigaction\(SIGUSR2, {SIG_IGN, \[HUP INT\], SA_RESTART}, {SIG_DFL, \[\], 0}, (4|8|16)(, 0x[0-9a-f]+)?\) = 0$/ {next}
NR == 1 && /^rt_sigaction\(SIGUSR2, {SIG_IGN, \[HUP INT\], SA_RESTART}, {SIG_DFL, \[\], 0}, 0x[0-9a-f]+, (4|8|16)\) = 0$/ {next}

# Test 2.
NR == 2 && /^rt_sigaction\(SIGUSR2, {0x[0-9a-f]+, \[QUIT TERM\], SA_RESTORER\|SA_SIGINFO, 0x[0-9a-f]+}, {SIG_IGN, \[HUP INT\], SA_RESTORER\|SA_RESTART, 0x[0-9a-f]+}, (0x[0-9a-f]+, )?(4|8|16)\) = 0$/ {next}
NR == 2 && /^rt_sigaction\(SIGUSR2, {0x[0-9a-f]+, \[QUIT TERM\], SA_SIGINFO}, {SIG_IGN, \[HUP INT\], SA_RESTART}, (4|8|16)(, 0x[0-9a-f]+)?\) = 0$/ {next}
NR == 2 && /^rt_sigaction\(SIGUSR2, {0x[0-9a-f]+, \[QUIT TERM\], SA_SIGINFO}, {SIG_IGN, \[HUP INT\], SA_RESTART}, 0x[0-9a-f]+, (4|8|16)\) = 0$/ {next}

# Test 3.
NR == 3 && /^rt_sigaction\(SIGUSR2, {SIG_DFL, \[\], SA_RESTORER, 0x[0-9a-f]+}, {0x[0-9a-f]+, \[QUIT TERM\], SA_RESTORER\|SA_SIGINFO, 0x[0-9a-f]+}, (0x[0-9a-f]+, )?(4|8|16)\) = 0$/ {next}
NR == 3 && /^rt_sigaction\(SIGUSR2, {SIG_DFL, \[\], 0}, {0x[0-9a-f]+, \[QUIT TERM\], SA_SIGINFO}, (4|8|16)(, 0x[0-9a-f]+)?\) = 0$/ {next}
NR == 3 && /^rt_sigaction\(SIGUSR2, {SIG_DFL, \[\], 0}, {0x[0-9a-f]+, \[QUIT TERM\], SA_SIGINFO}, 0x[0-9a-f]+, (4|8|16)\) = 0$/ {next}

# Test 4.
NR == 4 && /^rt_sigaction\(SIGUSR2, {SIG_DFL, ~\[HUP( ((RT|SIGRT)[^] ]+|[3-9][0-9]|1[0-9][0-9]))*\], SA_RESTORER, 0x[0-9a-f]+}, {SIG_DFL, \[\], SA_RESTORER, 0x[0-9a-f]+}, (0x[0-9a-f]+, )?(4|8|16)\) = 0$/ {next}
NR == 4 && /^rt_sigaction\(SIGUSR2, {SIG_DFL, ~\[HUP( ((RT|SIGRT)[^] ]+|[3-9][0-9]|1[0-9][0-9]))*\], 0}, {SIG_DFL, \[\], 0}, (4|8|16)(, 0x[0-9a-f]+)?\) = 0$/ {next}
NR == 4 && /^rt_sigaction\(SIGUSR2, {SIG_DFL, ~\[HUP( ((RT|SIGRT)[^] ]+|[3-9][0-9]|1[0-9][0-9]))*\], 0}, {SIG_DFL, \[\], 0}, 0x[0-9a-f]+, (4|8|16)\) = 0$/ {next}

# The last line.
NR == lines && /^\+\+\+ exited with 0 \+\+\+$/ {next}

{
  print "Line " NR " does not match: " $0
  fail=1
}

END {
  if (NR != lines) {
    print "Expected " lines " lines, found " NR " line(s)."
    print ""
    exit 1
  }
  if (fail) {
    print ""
    exit 1
  }
}
