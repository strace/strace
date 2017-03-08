strace - the linux syscall tracer
=================================

This is [strace](https://strace.io) -- a diagnostic, debugging and instructional userspace utility with a traditional command-line interface for Linux.  It is used to monitor and tamper with interactions between processes and the Linux kernel, which include system calls, signal deliveries, and changes of process state.  The operation of strace is made possible by the kernel feature known as [ptrace](http://man7.org/linux/man-pages/man2/ptrace.2.html).

strace is released under a Berkeley-style license at the request of Paul Kranenburg; see the file [COPYING](COPYING) for details.

See the file [NEWS](NEWS) for information on what has changed in recent versions.

Please read the file [INSTALL-git](INSTALL-git.md) for installation instructions.

The user discussion and development of strace take place on [the strace mailing list](https://lists.sourceforge.net/lists/listinfo/strace-devel) -- everyone is welcome to post bug reports, feature requests, comments and patches to strace-devel@lists.sourceforge.net.  The mailing list archives are available at https://sourceforge.net/p/strace/mailman/strace-devel/  and other archival sites.

[![Build Status](https://travis-ci.org/strace/strace.svg?branch=master)](https://travis-ci.org/strace/strace) [![Code Coverage](https://codecov.io/github/strace/strace/coverage.svg?branch=master)](https://codecov.io/github/strace/strace?branch=master)
