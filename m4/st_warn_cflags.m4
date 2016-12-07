AC_DEFUN([st_WARN_CFLAGS], [dnl
gl_WARN_ADD([-Wall])
gl_WARN_ADD([-Wempty-body])
gl_WARN_ADD([-Wformat-security])
gl_WARN_ADD([-Wignored-qualifiers])
gl_WARN_ADD([-Winit-self])
gl_WARN_ADD([-Wlogical-op])
gl_WARN_ADD([-Wmissing-parameter-type])
gl_WARN_ADD([-Wnested-externs])
gl_WARN_ADD([-Wold-style-declaration])
gl_WARN_ADD([-Wold-style-definition])
gl_WARN_ADD([-Wsign-compare])
gl_WARN_ADD([-Wtype-limits])
gl_WARN_ADD([-Wwrite-strings])
AC_ARG_ENABLE([gcc-Werror],
  [AS_HELP_STRING([--enable-gcc-Werror], [turn on gcc's -Werror option])],
  [case $enableval in
     yes) gl_WARN_ADD([-Werror]) ;;
     no)  ;;
     *)   AC_MSG_ERROR([bad value $enableval for gcc-Werror option]) ;;
   esac]
)
AC_SUBST([WARN_CFLAGS])
])
