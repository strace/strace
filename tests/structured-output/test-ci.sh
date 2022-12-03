#!/bin/sh

CC=gcc STACKTRACE=no TARGET=x86 ./ci/run-build-and-tests.sh

# an example of log for a failed test 'tampering-notes' would be in:
# $(ROOTDIR)/strace-5.19.0.40.1e75c/_build/sub/tests/tampering-notes.log
