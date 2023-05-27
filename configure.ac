AC_INIT([Fract Program], [0.1], [fract@pfeifer-syscon.de])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([1.11])
AM_SILENT_RULES([yes])
AX_CHECK_ENABLE_DEBUG([info], [DEBUG], [RELEASE], [IS-RELEASE])
AC_PROG_CXX
AC_PROG_CC
AX_CXX_COMPILE_STDCXX([20], [noext], [mandatory])
LT_INIT
PKG_CHECK_MODULES([GTKMM], [gtkmm-3.0])
PKG_CHECK_MODULES([GLIBMM], [glibmm-2.4 giomm-2.4])
PKG_CHECK_MODULES([GTHREAD], [gthread-2.0])
AM_CONDITIONAL([RELEASE],[test "x${enable_debug}" != "xyes"])
AX_CHECK_SED()
AC_CONFIG_FILES([
Makefile
res/Makefile
src/Makefile])
AC_OUTPUT
