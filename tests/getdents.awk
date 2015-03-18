BEGIN {
	i = "[0-9]+"
	len = "[1-9]" i

	d_ino = "d_ino=" i
	d_off = "d_off=" i
	d_reclen = "d_reclen=" len
	d_name_1 = "d_name=\"\\.\""
	d_name_2 = "d_name=\"\\.\\.\""
	d_name_3 = "d_name=\"(A\\\\n){127}Z\""
	# Some older systems might not pass back d_type at all like Alpha.
	d_type_dir = "d_type=DT_(DIR|UNKNOWN)"
	d_type_reg = "d_type=DT_(REG|UNKNOWN)"

	dirent_1   = "\\{" d_ino ", " d_off ", " d_reclen ", " d_name_1 ", " d_type_dir "\\}"
	dirent_2   = "\\{" d_ino ", " d_off ", " d_reclen ", " d_name_2 ", " d_type_dir "\\}"
	dirent_3   = "\\{" d_ino ", " d_off ", " d_reclen ", " d_name_3 ", " d_type_reg "\\}"

	dirent64_1 = "\\{" d_ino ", " d_off ", " d_reclen ", " d_type_dir ", " d_name_1 "\\}"
	dirent64_2 = "\\{" d_ino ", " d_off ", " d_reclen ", " d_type_dir ", " d_name_2 "\\}"
	dirent64_3 = "\\{" d_ino ", " d_off ", " d_reclen ", " d_type_reg ", " d_name_3 "\\}"

	d_123 = dirent_1 " " dirent_2 " " dirent_3
	d_213 = dirent_2 " " dirent_1 " " dirent_3
	d_132 = dirent_1 " " dirent_3 " " dirent_2
	d_321 = dirent_3 " " dirent_2 " " dirent_1
	d_231 = dirent_2 " " dirent_3 " " dirent_1
	d_312 = dirent_3 " " dirent_1 " " dirent_2

	d64_123 = dirent64_1 " " dirent64_2 " " dirent64_3
	d64_213 = dirent64_2 " " dirent64_1 " " dirent64_3
	d64_132 = dirent64_1 " " dirent64_3 " " dirent64_2
	d64_321 = dirent64_3 " " dirent64_2 " " dirent64_1
	d64_231 = dirent64_2 " " dirent64_3 " " dirent64_1
	d64_312 = dirent64_3 " " dirent64_1 " " dirent64_2

	dents = "\\{(" d_123 "|" d_213 "|" d_132 "|" d_321 "|" d_231 "|" d_312 ")\\}"
	dents64 = "\\{(" d64_123 "|" d64_213 "|" d64_132 "|" d64_321 "|" d64_231 "|" d64_312 ")\\}"

	getdents   =   "getdents\\(" i ", " dents   ", " len "\\)"
	getdents64 = "getdents64\\(" i ", " dents64 ", " len "\\)"

	r[1] = "^(" getdents "|" getdents64 ") += " len "$"
	r[2] = "^getdents(64)?\\([0-9]+, \\{\\}, [1-9][0-9]+\\) += 0$"
	s[3] = "+++ exited with 0 +++"

	lines = 3
	fail = 0
}

@include "match.awk"
