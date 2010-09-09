dnl ### A macro to set gcc warning flags.
define(AC_WARNFLAGS,
[AC_SUBST(WARNFLAGS)
if test -z "$WARNFLAGS"
then
	if test -n "$GCC"
	then
		# If we're using gcc we want warning flags.
		WARNFLAGS=-Wall
	fi
fi
])
