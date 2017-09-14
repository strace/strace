#!/bin/sh

. "${srcdir=.}/init.sh"

SCRIPTFILE=lua-script.lua

run_strace_with_script()
{
	cat > "$SCRIPTFILE" || fail_ "cannot write $SCRIPTFILE"
	run_strace -l "$SCRIPTFILE" "$@"
}
