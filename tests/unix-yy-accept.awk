#!/bin/gawk
#
# Copyright (c) 2014 Masatake YAMATO <yamato@redhat.com>
# Copyright (c) 2014-2015 Dmitry V. Levin <ldv@altlinux.org>
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. The name of the author may not be used to endorse or promote products
#    derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
# OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
# IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
# NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
# THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

BEGIN {
  lines = 8
  fail = 0

  addrlen = length(addr) + 3
  r_i = "[1-9][0-9]*"
  r_socket = "^socket\\(PF_(LOCAL|UNIX|FILE), SOCK_STREAM, 0\\) += 0<UNIX:\\[(" r_i ")\\]>$"
}

NR == 1 {
  if (match($0, r_socket, a)) {
    inode_listen = a[2]
    r_bind = "^bind\\(0<UNIX:\\[" inode_listen "\\]>, \\{sa_family=AF_(LOCAL|UNIX|FILE), sun_path=\"" addr "\"\\}, " addrlen "\\) += 0$"
    r_listen = "^listen\\(0<UNIX:\\[" inode_listen ",\"" addr "\"\\]>, 5\\) += 0$"
    r_getsockname = "^getsockname\\(0<UNIX:\\[" inode_listen ",\"" addr "\"\\]>, \\{sa_family=AF_(LOCAL|UNIX|FILE), sun_path=\"" addr "\"\\}, \\[" addrlen "\\]\\) += 0$"
    r_accept = "^accept\\(0<UNIX:\\[" inode_listen ",\"" addr "\"\\]>, \\{sa_family=AF_(LOCAL|UNIX|FILE), NULL\\}, \\[2\\]\\) += 1<UNIX:\\[(" r_i ")->(" r_i "),\"" addr "\"\\]>"
    next
  }
}

NR == 2 {if (r_bind != "" && match($0, r_bind)) next}

NR == 3 {if (r_listen != "" && match($0, r_listen)) next}

NR == 4 {if (r_getsockname != "" && match($0, r_getsockname)) next}

NR == 5 {
  if (r_accept != "" && match($0, r_accept, a)) {
    inode_accepted = a[2]
    inode_peer = a[3]
    r_close_listen = "^close\\(0<UNIX:\\[" inode_listen ",\"" addr "\"\\]>\\) += 0$"
    r_close_accepted = "^close\\(1<UNIX:\\[" inode_accepted ",\"" addr "\"\\]>\\) += 0$"
    next
  }
}

NR == 6 {if (r_close_listen != "" && match($0, r_close_listen)) next}
NR == 7 {if (r_close_accepted != "" && match($0, r_close_accepted)) next}

NR == lines && $0 == "+++ exited with 0 +++" {next}

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
