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
    switch (expected) {
      case "getuid":
        uid = a[1]
        expected = "setuid"
        regexp = "^setuid" suffix "\\(" uid "\\)[[:space:]]+= 0$"
        next
      case "setuid":
        expected = "getresuid"
        regexp = "^getresuid" suffix "\\(\\[" uid "\\], \\[" uid "\\], \\[" uid "\\]\\)[[:space:]]+= 0$"
        next
      case "getresuid":
        expected = "setreuid"
        regexp = "^setreuid" suffix "\\(-1, -1\\)[[:space:]]+= 0$"
        next
      case "setreuid":
        expected = "setresuid"
        regexp = "^setresuid" suffix "\\(-1, " uid ", -1\\)[[:space:]]+= 0$"
        next
      case "setresuid":
        expected = "chown"
        regexp = "^chown" suffix "\\(\".\", -1, -1\\)[[:space:]]+= 0$"
        next
      case "chown":
        expected = "1st getgroups"
        regexp = "^getgroups" suffix "\\(0, NULL\\)[[:space:]]+= " r_uint "$"
        next
      case "1st getgroups":
        ngroups = a[1]
        switch (ngroups) {
          case "0": list=""; break
          case "1": list=r_uint; break
          default: list=r_uint "(, " r_uint "){" (ngroups - 1) "}"
        }
        expected = "2nd getgroups"
        regexp = "^getgroups" suffix "\\(" ngroups ", \\[" list "\\]\\)[[:space:]]+= " ngroups "$"
        next
      case "2nd getgroups":
        expected = "the last line"
        regexp = "^\\+\\+\\+ exited with 0 \\+\\+\\+$"
        next
      case "the last line":
        expected = "nothing"
        regexp = ""
        next
    }
  }
  next
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
