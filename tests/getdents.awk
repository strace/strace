BEGIN {
  lines = 3
  fail = 0

  i = "[0-9]+"
  len = "[1-9]" i

  d_ino = "d_ino=" i
  d_off = "d_off=" i
  d_reclen = "d_reclen=" len
  d_name1 = "d_name=\"\\.\""
  d_name2 = "d_name=\"\\.\\.\""
  d_type = "d_type=DT_DIR"

  d1_1 = "{" d_ino ", " d_off ", " d_reclen ", " d_name1 ", " d_type "}"
  d1_2 = "{" d_ino ", " d_off ", " d_reclen ", " d_name2 ", " d_type "}"
  d2_1 = "{" d_ino ", " d_off ", " d_reclen ", " d_type ", " d_name1 "}"
  d2_2 = "{" d_ino ", " d_off ", " d_reclen ", " d_type ", " d_name2 "}"

  getdents   =   "^getdents\\(" i ", {(" d1_1 " " d1_2 "|" d1_2 " " d1_1 ")}, " len "\\) += " len "$"
  getdents64 = "^getdents64\\(" i ", {(" d2_1 " " d2_2 "|" d2_2 " " d2_1 ")}, " len "\\) += " len "$"
}

NR == 1 {if (match($0, getdents) || match($0, getdents64)) next}

NR == 2 && /^getdents(64)?\([0-9]+, \{\}, [1-9][0-9]+\) += 0$/ {next}

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
