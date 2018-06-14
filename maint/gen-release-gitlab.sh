#!/bin/sh -efu

cat <<'EOF'
Downloads
=========

EOF

set +f
set -- strace-*.tar.xz*
set -f
for f; do
	printf '[%s](/uploads/%s/%s)\n' "$f" "..." "$f"
done

cat <<'EOF'
**Please ignore so called "Source code" links provided by gitlab, they are useless**.

EOF

"$(dirname "$0")"/gen-tag-message.sh |
	sed 's/\([^[:space:]]\)\*/\1\\*/g'
