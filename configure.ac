AC_INIT([Fract Program], [0.1], [fract@pfeifer-syscon.de])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([config.h])
dnl use -no-portablitity to stop complaining about $(shell... in res Makefile.am
dnl i dont get it, as the shell is everywhere, turtle power!

AM_INIT_AUTOMAKE([1.11 -Wno-portability])

AM_SILENT_RULES([yes])

AX_CHECK_ENABLE_DEBUG([info], [DEBUG], [RELEASE], [IS-RELEASE])
AC_PROG_CXX
AC_PROG_CC
dnl we depend on c++20 in just one place (glglobe),
dnl (did choose it for consistency across libs and programs)
dnl so if you happen to find yourself in the need to use
dnl a older compiler, c++17 may work as well.
AX_CXX_COMPILE_STDCXX([20], [noext], [optional])
LT_INIT
AC_CANONICAL_HOST
AC_MSG_NOTICE([host_os $host_os])
PKG_CHECK_MODULES([GTKMM], [gtkmm-3.0])
PKG_CHECK_MODULES([GLIBMM], [glibmm-2.4 giomm-2.4])
PKG_CHECK_MODULES([GTHREAD], [gthread-2.0])
AM_CONDITIONAL([RELEASE],[test "x${enable_debug}" != "xyes"])
if test "$host_os" = "mingw32" ; then
    AC_SUBST(EXTRA_LDFLAGS, "-mwindows")
fi
AC_CONFIG_FILES([
Makefile
res/Makefile
src/Makefile])
AC_OUTPUT
