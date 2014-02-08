# rt_sigaction on ALPHA has 5 args: sig, act, oact, sigsetsize, restorer.
# rt_sigaction on SPARC has 5 args: sig, act, oact, restorer, sigsetsize.
# rt_sigaction on other architectures has 4 args: sig, act, oact, sigsetsize.
# Some architectures have SA_RESTORER, some don't;
# in particular, SPARC has and ALPHA hasn't.
#
# There are two regexps for each test:
# the 1st is for any architecture with SA_RESTORER, including SPARC;
# the 2nd is for any architecture without SA_RESTORER, including ALPHA.

# Test 1.
NR == 1 && /^rt_sigaction\(SIGUSR2, {SIG_IGN, \[HUP INT\], SA_RESTORER\|SA_RESTART, 0x[0-9a-f]+}, {SIG_DFL, \[\], 0}, (0x[0-9a-f]+, )?(4|8|16)\) = 0$/ {next}
NR == 1 && /^rt_sigaction\(SIGUSR2, {SIG_IGN, \[HUP INT\], SA_RESTART}, {SIG_DFL, \[\], 0}, (4|8|16)(, 0x[0-9a-f]+)?\) = 0$/ {next}

# Test 2.
NR == 2 && /^rt_sigaction\(SIGUSR2, {0x[0-9a-f]+, \[QUIT TERM\], SA_RESTORER\|SA_SIGINFO, 0x[0-9a-f]+}, {SIG_IGN, \[HUP INT\], SA_RESTORER\|SA_RESTART, 0x[0-9a-f]+}, (0x[0-9a-f]+, )?(4|8|16)\) = 0$/ {next}
NR == 2 && /^rt_sigaction\(SIGUSR2, {0x[0-9a-f]+, \[QUIT TERM\], SA_SIGINFO}, {SIG_IGN, \[HUP INT\], SA_RESTART}, (4|8|16)(, 0x[0-9a-f]+)?\) = 0$/ {next}

# Test 3.
NR == 3 && /^rt_sigaction\(SIGUSR2, {SIG_DFL, \[\], SA_RESTORER, 0x[0-9a-f]+}, {0x[0-9a-f]+, \[QUIT TERM\], SA_RESTORER\|SA_SIGINFO, 0x[0-9a-f]+}, (0x[0-9a-f]+, )?(4|8|16)\) = 0$/ {next}
NR == 3 && /^rt_sigaction\(SIGUSR2, {SIG_DFL, \[\], 0}, {0x[0-9a-f]+, \[QUIT TERM\], SA_SIGINFO}, (4|8|16)(, 0x[0-9a-f]+)?\) = 0$/ {next}

# The last line.
NR == 4 && /^\+\+\+ exited with 0 \+\+\+$/ {next}

{
  print "Line " NR " does not match: " $0
  exit 1
}

END {
  if (NR != 4) {
    print "Expected 4 lines, found " NR " line(s)."
    exit 1
  }
}
