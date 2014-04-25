#!/bin/sh

convert() {
	sed -n '/^\(static \)\?const struct xlat '"$n"'\[\] = {$/,/^};$/{
		s/^[[:space:]]*XLAT(\([^)]\+\)).*/\1/p
		s/^[[:space:]]*{[[:space:]]*(\?\(1<<[^),[:space:]]\+\).*/\1/p
		s/.*not NULL-terminated.*/#unterminated/p
		s/^\([[:space:]]*{.*\)/\1/p
		s/^\t*\( *[/*].*\)/\1/p}' "$f" >> xlat/"$n".in
	sed -i '/^\(static \)\?const struct xlat '"$n"'\[\] = {$/,/^};$/c #include "xlat/'"$n"'.h"' "$f"
}

for f; do
	for n in $(sed -n 's/^\(static \)\?const struct xlat \([a-z0-9_]\+\)\[\] = {$/\2/p' "$f"); do
		case "$n" in
			cacheflush_flags|struct_user_offsets) # skip
				;;
			ioprio_class|ioprio_who|mtd_mode_options|personality_options|syslog_action_type|ubi_volume_props|ubi_volume_types)
				echo '#unconditional' > xlat/"$n".in
				convert
				;;
			*)
				> xlat/"$n".in
				convert
				;;
		esac
	done
done
