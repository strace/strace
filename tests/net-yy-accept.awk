BEGIN {
  lines = 9
  fail = 0

  r_i = "[1-9][0-9]*"
  r_port = "[1-9][0-9][0-9][0-9]+"
  r_localhost = "127\\.0\\.0\\.1"
  r_bind = "^bind\\(0<TCP:\\[(" r_i ")\\]>, \\{sa_family=AF_INET, sin_port=htons\\(0\\), sin_addr=inet_addr\\(\"" r_localhost "\"\\)\\}, " r_i "\\) += 0$"
  r_getsockname = "^getsockname\\(0<TCP:\\[" r_localhost ":(" r_port ")\\]>, \\{sa_family=AF_INET, sin_port=htons\\((" r_port ")\\), sin_addr=inet_addr\\(\"" r_localhost "\"\\)\\}, \\[" r_i "\\]\\) += 0$"
}

NR == 1 && /^socket\(PF_INET, SOCK_STREAM, IPPROTO_IP\) += 0$/ {next}

NR == 2 {
  if (match($0, r_bind, a)) {
    inode = a[1]
    r_listen = "^listen\\(0<TCP:\\[" inode "\\]>, 5\\) += 0$"
    next
  }
}

NR == 3 {if (r_listen != "" && match($0, r_listen)) next}

NR == 4 {
  if (match($0, r_getsockname, a) && a[1] == a[2]) {
    port_l = a[1]
    r_accept = "^accept\\(0<TCP:\\[" r_localhost ":" port_l "\\]>, \\{sa_family=AF_INET, sin_port=htons\\((" r_port ")\\), sin_addr=inet_addr\\(\"" r_localhost "\"\\)\\}, \\[" r_i "\\]\\) += 1<TCP:\\[" r_localhost ":" port_l "->" r_localhost ":(" r_port ")\\]>$"
    r_close0 = "^close\\(0<TCP:\\[" r_localhost ":" port_l "\\]>) += 0$"
    next
  }
}

NR == 5 {
  if (r_accept != "" && match($0, r_accept, a) && a[1] == a[2]) {
    port_r = a[1]
    r_recv = "^recv\\(1<TCP:\\[" r_localhost ":" port_l "->" r_localhost ":" port_r "\\]>, \"data\", 5, MSG_WAITALL\\) += 4$"
    r_recvfrom = "^recvfrom\\(1<TCP:\\[" r_localhost ":" port_l "->" r_localhost ":" port_r "\\]>, \"data\", 5, MSG_WAITALL, NULL, NULL\\) += 4$"
    r_close1 = "^close\\(1<TCP:\\[" r_localhost ":" port_l "->" r_localhost ":" port_r "\\]>) += 0$"
    next
  }
}

NR == 6 {if (r_close0 != "" && match($0, r_close0)) next}

NR == 7 {if (r_recv != "" && (match($0, r_recv) || match($0, r_recvfrom))) next}

NR == 8 {if (r_close1 != "" && match($0, r_close1)) next}

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
