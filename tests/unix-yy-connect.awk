BEGIN {
  lines = 5
  fail = 0

  inode = "?"

  r_i = "[1-9][0-9]*"
  r_close0 = "^close\\(0<UNIX:[" r_i ",\"local-stream\"]>\\) += 0$"
  r_connect = "^connect\\(1<UNIX:\\[(" r_i ")\\]>, {sa_family=AF_LOCAL, sun_path=\"local-stream\"}, " r_i "\\) += 0$"
  r_close1 = "^/$"
}

NR == 1 && /^socket\(PF_LOCAL, SOCK_STREAM, 0\) += 1$/ {next}
NR == 2 {if (match($0, r_close0)) next}
NR == 3 {
  if (match($0, r_connect, a)) {
    inode = a[1]
    r_close1 = "^close\\(1<UNIX:\\[(" r_i ")->" r_i "\\]>\\) += 0$"
    next
  }
}
NR == 4 {if (match($0, r_close1, a) && a[1] == inode) {next}}

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
