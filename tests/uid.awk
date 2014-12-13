BEGIN {
  ok = 0
  fail = 0
  r_uid = "(0|[1-9][0-9]*)"
  r_getuid = "^getuid" suffix "\\(\\)[[:space:]]+= " r_uid "$"
  r_setuid = "^/$"
  r_getresuid = "^/$"
  r_setreuid = "^/$"
  r_setresuid = "^/$"
  r_chown = "^/$"
  s_last = "/"
}

ok == 1 {
  fail = 1
  next
}

$0 == s_last {
  ok = 1
  next
}

{
  if (match($0, r_getuid, a)) {
    r_uid = a[1]
    r_setuid = "^setuid" suffix "\\(" r_uid "\\)[[:space:]]+= 0$"
    next
  }
  if (match($0, r_setuid)) {
    r_getresuid = "^getresuid" suffix "\\(\\[" r_uid "\\], \\[" r_uid "\\], \\[" r_uid "\\]\\)[[:space:]]+= 0$"
    next
  }
  if (match($0, r_getresuid)) {
    r_setreuid = "^setreuid" suffix "\\(-1, -1\\)[[:space:]]+= 0$"
    next
  }
  if (match($0, r_setreuid)) {
    r_setresuid = "^setresuid" suffix "\\(-1, " r_uid ", -1\\)[[:space:]]+= 0$"
    next
  }
  if (match($0, r_setresuid)) {
    r_chown = "^chown" suffix "\\(\".\", -1, -1\\)[[:space:]]+= 0$"
    next
  }
  if (match($0, r_chown)) {
    s_last = "+++ exited with 0 +++"
    next
  }
  next
}

END {
  if (fail) {
    print "Unexpected output after exit"
    exit 1
  }
  if (ok)
    exit 0
  if (r_setuid == "^/$") {
    print "getuid doesn't match"
    exit 1
  }
  if (r_getresuid == "^/$") {
    print "setuid doesn't match"
    exit 1
  }
  if (r_setreuid == "^/$") {
    print "getresuid doesn't match"
    exit 1
  }
  if (r_setresuid == "^/$") {
    print "setreuid doesn't match"
    exit 1
  }
  if (r_chown == "^/$") {
    print "setresuid doesn't match"
    exit 1
  }
  if (s_last == "/") {
    print "chown doesn't match"
    exit 1
  }
  print "The last line doesn't match"
  exit 1
}
