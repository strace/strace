BEGIN {
  lines = 8
  fail = 0

  addrlen = length(addr) + 3
  r_i = "[1-9][0-9]*"
  r_bind = "^bind\\(0<UNIX:\\[(" r_i ")\\]>, \\{sa_family=AF_(LOCAL|UNIX|FILE), sun_path=\"" addr "\"\\}, " addrlen "\\) += 0$"
}

NR == 1 && /^socket\(PF_(LOCAL|UNIX|FILE), SOCK_STREAM, 0\) += 0$/ {next}

NR == 2 {
  if (match($0, r_bind, a)) {
    inode_listen = a[1]
    r_listen = "^listen\\(0<UNIX:\\[" inode_listen ",\"" addr "\"\\]>, 5\\) += 0$"
    r_getsockname = "^getsockname\\(0<UNIX:\\[" inode_listen ",\"" addr "\"\\]>, \\{sa_family=AF_(LOCAL|UNIX|FILE), sun_path=\"" addr "\"\\}, \\[" addrlen "\\]\\) += 0$"
    r_accept = "^accept\\(0<UNIX:\\[" inode_listen ",\"" addr "\"\\]>, \\{sa_family=AF_(LOCAL|UNIX|FILE), NULL\\}, \\[2\\]\\) += 1<UNIX:\\[(" r_i ")->(" r_i "),\"" addr "\"\\]>"
    next
  }
}

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
