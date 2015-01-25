BEGIN {
  lines = 3
  fail = 0

  i = "[0-9]+"
  len = "[1-9]" i

  d_ino = "d_ino=" i
  d_off = "d_off=" i
  d_reclen = "d_reclen=" len
  d_name_1 = "d_name=\"\\.\""
  d_name_2 = "d_name=\"\\.\\.\""
  d_name_3 = "d_name=\"(A\\\\n){127}Z\""
  d_type_dir = "d_type=DT_DIR"
  d_type_reg = "d_type=DT_REG"

  dirent_1   = "\\{" d_ino ", " d_off ", " d_reclen ", " d_name_1 ", " d_type_dir "\\}"
  dirent_2   = "\\{" d_ino ", " d_off ", " d_reclen ", " d_name_2 ", " d_type_dir "\\}"
  dirent_3   = "\\{" d_ino ", " d_off ", " d_reclen ", " d_name_3 ", " d_type_reg "\\}"

  dirent64_1 = "\\{" d_ino ", " d_off ", " d_reclen ", " d_type_dir ", " d_name_1 "\\}"
  dirent64_2 = "\\{" d_ino ", " d_off ", " d_reclen ", " d_type_dir ", " d_name_2 "\\}"
  dirent64_3 = "\\{" d_ino ", " d_off ", " d_reclen ", " d_type_reg ", " d_name_3 "\\}"

  dents   = "\\{(" dirent_1   " " dirent_2   "|" dirent_2   " " dirent_1   ") " dirent_3   "\\}"
  dents64 = "\\{(" dirent64_1 " " dirent64_2 "|" dirent64_2 " " dirent64_1 ") " dirent64_3 "\\}"

  getdents   =   "^getdents\\(" i ", " dents   ", " len "\\) += " len "$"
  getdents64 = "^getdents64\\(" i ", " dents64 ", " len "\\) += " len "$"
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
