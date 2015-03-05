BEGIN {
  lines = 6
  fail = 0
  addrlen = length(addr) + 3

  r_i = "[1-9][0-9]*"
  r_close_listen = "^close\\(0<UNIX:[" r_i ",\"" addr "\"]>\\) += 0$"
  r_connect = "^connect\\(1<UNIX:\\[(" r_i ")\\]>, \\{sa_family=AF_(LOCAL|UNIX|FILE), sun_path=\"" addr "\"\\}, " addrlen "\\) += 0$"
}

NR == 1 && /^socket\(PF_(LOCAL|UNIX|FILE), SOCK_STREAM, 0\) += 1$/ {next}

NR == 2 {if (match($0, r_close_listen)) next}

NR == 3 {
  if (match($0, r_connect, a)) {
    inode = a[1]
    r_close_connected = "^close\\(1<UNIX:\\[(" r_i ")->" r_i "\\]>\\) += 0$"
    next
  }
}

NR == 4 && /^--- SIGUSR1 / {next}

NR == 5 {
  if (inode != "" && r_close_connected != "" && match($0, r_close_connected, a) && a[1] == inode) {
    next
  }
}

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
