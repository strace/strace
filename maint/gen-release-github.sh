#!/bin/sh -efu

"$(dirname "$0")"/gen-tag-message.sh |
	sed 's/\([^[:space:]]\)\*/\1\\*/g'

cat <<'EOF'

Downloads
=========

**Please ignore so called "Source code" links provided by github above, they are useless**.
EOF
