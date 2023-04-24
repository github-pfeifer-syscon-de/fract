# ===========================================================================
#
# ===========================================================================
#
# SYNOPSIS
#
#   AX_CHECK_SED()
#
# DESCRIPTION
#
#   Check for the presence of sed, fail if it cannot be found.
#     and define @regex_cmd@ for use within Makefile.am
#
# LICENSE
#
#   Copyright (c) 2021 rpf <ax_check_sed@pfeifer-syscon.de>
#
#   Copying and distribution of this file, with or without modification, are
#   permitted in any medium without royalty provided the copyright notice
#   and this notice are preserved.

#serial 1

AC_DEFUN([AX_CHECK_SED],[
    AC_CHECK_PROGS(regex_cmd, sed)
    if test x$regex_cmd = "x" ; then
        AC_MSG_ERROR([sed is required to build the data files.])
    fi
])
