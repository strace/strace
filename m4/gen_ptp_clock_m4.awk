/^struct (strace_ptp_[^[:space:]]+)[[:space:]]+{/ {
	match($0, /^struct strace_(ptp_[^[:space:]]+)[[:space:]]+{/, a)

	struct_name = a[1]
	prefix = "struct " struct_name

	in_struct = 1
	next
}

/^};/ {
	in_struct = 0
	next
}

(in_struct == 1) {
	if (match($0, /^[[:space:]]+([^;\[\]]+[[:space:]]+)+([^[:space:]\[\];]+)(\[[^;]*\])?;$/, a)) {
		print "\t\t" prefix "." a[2] ","
	}
}

