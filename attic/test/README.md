This directory contains some corner case demo files.  Most of them are
quite old and probably will be of little interest to the casual reader.
For automated tests, see [../../tests](../../tests) directory.

To run a demo:
* Run make
* Run resulting executable(s) under strace
* Check strace output and/or program's output and exitcode

To add a new demo:
* Add its .c source to this dir
* Add it to "all" and "clean" targets in Makefile
* Add it to .gitignore file

Please spend some time making your demo understandable.
For example, it may print an explanation how it should be used
(which strace options to use, and what to look for in strace output).

If possible, make it so that your demo detects error/bug
it is intended to demonstrate, and prints error message and exits with 1
if the bug is detected, instead of relying on user to peruse strace output.
