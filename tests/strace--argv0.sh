#! /bin/sh

# Test the command-line option  --argv0=string  to strace.
STRACE=./strace  # the executable to test

# Output from strace on fd 2 (stderr) goes to fd 1 (stdout)
# while output from "ls" is discarded onto /dev/null.
# In this case, the typical expected output from strace is:
#    execve("/usr/bin/ls", ["ls"], 0x7ffd60359170 /* 45 vars */) = 0
#    +++ exited with 0 +++
# That text is piped into grep, which checks argv[0] for "ls".

$STRACE -e trace=execve             ls  2>&1  1>/dev/null  |\
      	grep --silent 'execve([^,]*, \["ls"],'  \
&& \
# Now do the same thing, except test the new command-line option
# which substitutes an arbitrary string for  argv[0].
$STRACE -e trace=execve --argv0=foo ls  2>&1  1>/dev/null  |\
      	grep --silent 'execve([^,]*, \["foo"],'

