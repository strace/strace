BEGIN {
  lines = 8
  fail = 0

  inode_listen = "?"
  inode_accepted = "?"
  inode_peer = "?"

  r_i = "[1-9][0-9]*"
  r_bind = "^bind\\(0<UNIX:\\[(" r_i ")\\]>, {sa_family=AF_LOCAL, sun_path=\"local-stream\"}, " r_i "\\) += 0$"
  r_listen = "^/$"
  r_getsockname = "^/$"
  r_accept = "^/$"
  r_close0 = "^/$"
  r_close1 = "^/$"
}

NR == 1 && /^socket\(PF_LOCAL, SOCK_STREAM, 0\) += 0$/ {next}

NR == 2 {
  if (match($0, r_bind, a)) {
    inode_listen = a[1]
    r_listen = "^listen\\(0<UNIX:\\[" inode_listen ",\"local-stream\"\\]>, 5\\) += 0$"
    r_getsockname = "^getsockname\\(0<UNIX:\\[" inode_listen ",\"local-stream\"\\]>, {sa_family=AF_LOCAL, sun_path=\"local-stream\"}, \\[" r_i "\\]\\) += 0$"
    r_accept = "^accept\\(0<UNIX:\\[" inode_listen ",\"local-stream\"\\]>, {sa_family=AF_LOCAL, NULL}, \\[" r_i "\\]\\) += 1<UNIX:\\[(" r_i ")->(" r_i "),\"local-stream\"\\]>"
    next
  }
}

NR == 3 {if (match($0, r_listen)) next}

NR == 4 {if (match($0, r_getsockname)) next}

NR == 5 {
  if (match($0, r_accept, a)) {
    inode_accepted = a[1]
    inode_peer = a[2]
    print inode_accepted
    r_close0 = "^close\\(0<UNIX:\\[" inode_listen ",\"local-stream\"\\]>\\) += 0$"
    r_close1 = "^close\\(1<UNIX:\\[" inode_accepted ",\"local-stream\"\\]>\\) += 0$"
    next
  }
}

NR == 6 {if (match($0, r_close0)) next}
NR == 7 {if (match($0, r_close1)) next}

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
