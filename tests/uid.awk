BEGIN {
  r_uint = "(0|[1-9][0-9]*)"
  regexp = "^getuid" suffix "\\(\\)[[:space:]]+= " r_uint "$"
  expected = "getuid"
  fail = 0
}

regexp == "" {
  fail = 1
  next
}

{
  if (match($0, regexp, a)) {
    if (expected == "getuid") {
      uid = a[1]
      expected = "setuid"
      regexp = "^setuid" suffix "\\(" uid "\\)[[:space:]]+= 0$"
    } else if (expected == "setuid") {
      expected = "getresuid"
      regexp = "^getresuid" suffix "\\(\\[" uid "\\], \\[" uid "\\], \\[" uid "\\]\\)[[:space:]]+= 0$"
    } else if (expected == "getresuid") {
      expected = "setreuid"
      regexp = "^setreuid" suffix "\\(-1, -1\\)[[:space:]]+= 0$"
    } else if (expected == "setreuid") {
      expected = "setresuid"
      regexp = "^setresuid" suffix "\\(" uid ", -1, -1\\)[[:space:]]+= 0$"
    } else if (expected == "setresuid") {
      expected = "fchown"
      regexp = "^fchown" suffix "\\(1, -1, -1\\)[[:space:]]+= 0$"
    } else if (expected == "fchown") {
      expected = "1st getgroups"
      regexp = "^getgroups" suffix "\\(0, NULL\\)[[:space:]]+= " r_uint "$"
    } else if (expected == "1st getgroups") {
      ngroups = a[1]
      if (ngroups == "0")
        list=""
      else if (ngroups == "1")
        list=r_uint
      else
        list=r_uint "(, " r_uint "){" (ngroups - 1) "}"
      expected = "2nd getgroups"
      regexp = "^getgroups" suffix "\\(" ngroups ", \\[" list "\\]\\)[[:space:]]+= " ngroups "$"
    } else if (expected == "2nd getgroups") {
      expected = "the last line"
      regexp = "^\\+\\+\\+ exited with 0 \\+\\+\\+$"
    } else if (expected == "the last line") {
      expected = "nothing"
      regexp = ""
    }
  }
}

END {
  if (fail) {
    print "Unexpected output after exit"
    exit 1
  }
  if (regexp == "")
    exit 0
  print "error: " expected " doesn't match"
  exit 1
}
