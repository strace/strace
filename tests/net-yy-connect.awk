BEGIN {
  lines = 5
  fail = 0

  r_i = "[1-9][0-9]*"
  r_port = "[1-9][0-9][0-9][0-9]+"
  r_localhost = "127\\.0\\.0\\.1"
  r_connect = "^connect\\(0<TCP:\\[" r_i "\\]>, \\{sa_family=AF_INET, sin_port=htons\\((" r_port ")\\), sin_addr=inet_addr\\(\"" r_localhost "\"\\)\\}, " r_i ") += 0$"
}

NR == 1 && /^socket\(PF_INET, SOCK_STREAM, IPPROTO_IP\) += 0$/ {next}

NR == 2 {
  if (match($0, r_connect, a)) {
    port_r = a[1]
    r_send = "^send\\(0<TCP:\\[" r_localhost ":(" r_port ")->" r_localhost ":" port_r "\\]>, \"data\", 4, MSG_DONTROUTE\\) += 4$"
    r_sendto = "^sendto\\(0<TCP:\\[" r_localhost ":(" r_port ")->" r_localhost ":" port_r "\\]>, \"data\", 4, MSG_DONTROUTE, NULL, 0\\) += 4$"
    next
  }
}

NR == 3 {
  if (r_send != "" && (match($0, r_send, a) || match($0, r_sendto, a))) {
    port_l = a[1]
    r_close = "^close\\(0<TCP:\\[" r_localhost ":" port_l "->" r_localhost ":" port_r "\\]>\\) += 0$"
    next
  }
}

NR == 4 {if (r_close != "" && match($0, r_close)) next}

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
