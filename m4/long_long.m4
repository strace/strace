dnl ### A macro to determine endianness of long long
AC_DEFUN([AC_LITTLE_ENDIAN_LONG_LONG],
[AC_MSG_CHECKING(for little endian long long)
AC_CACHE_VAL(ac_cv_have_little_endian_long_long,
[AC_RUN_IFELSE([AC_LANG_SOURCE([[
int main () {
	union {
		long long ll;
		int l [2];
	} u;
	u.ll = 0x12345678;
	if (u.l[0] == 0x12345678)
		return 0;
	return 1;
}
]])],[ac_cv_have_little_endian_long_long=yes],[ac_cv_have_little_endian_long_long=no],[
if test "x$ac_cv_c_bigendian" = "xyes"; then
	ac_cv_have_little_endian_long_long=no
else
	ac_cv_have_little_endian_long_long=yes
fi
])])
AC_MSG_RESULT($ac_cv_have_little_endian_long_long)
if test "$ac_cv_have_little_endian_long_long" = yes
then
	AC_DEFINE([HAVE_LITTLE_ENDIAN_LONG_LONG], 1,
[Define if long long is little-endian.])
fi
])
