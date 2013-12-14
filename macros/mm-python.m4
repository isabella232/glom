## Copyright (c) 2009  Openismus GmbH  <http://www.openismus.com/>
##
## This file is part of mm-autofu.
##
## mm-autofu is free software: you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published
## by the Free Software Foundation, either version 2 of the License,
## or (at your option) any later version.
##
## mm-autofu is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with mm-autofu.  If not, see <http://www.gnu.org/licenses/>.

#serial 20090721

## _MM_PYTHON_SYSCONFIG(expression, [action-if-succeeded], [action-if-failed])
##
m4_define([_MM_PYTHON_SYSCONFIG],
[dnl
mm_val=`$PYTHON -c "dnl
[import sys; from distutils import sysconfig; sys.stdout.write]dnl
([sysconfig.]$1 or '')" 2>&AS_MESSAGE_LOG_FD`
AS_IF([test "[$]?" -eq 0 && test "x$mm_val" != x], [$2], [$3])[]dnl
])

## MM_CHECK_MODULE_PYTHON
##
## Check whether Python is installed, and determine the include path
## and libraries needed for linking a C or C++ program with Python.
## The resulting configuration is stored in the PYTHON_CPPFLAGS and
## PYTHON_LIBS substitution variables.
##
## Note: We should document why we use this instead of AX_PYTHON().
##
AC_DEFUN([MM_CHECK_MODULE_PYTHON],
[dnl
AC_REQUIRE([AM_PATH_PYTHON])[]dnl
dnl
AC_ARG_VAR([PYTHON_CPPFLAGS], [compiler include flags for Python])[]dnl
AC_ARG_VAR([PYTHON_LIBS], [linker flags for Python])[]dnl
dnl
AC_MSG_CHECKING([for Python headers])
AS_IF([test "x$PYTHON_CPPFLAGS" = x],
[
  _MM_PYTHON_SYSCONFIG([[get_python_inc()]], [PYTHON_CPPFLAGS="-I$mm_val"])
  _MM_PYTHON_SYSCONFIG([[get_python_inc(True)]],
  [test "x$PYTHON_CPPFLAGS" = "x-I$mm_val" || PYTHON_CPPFLAGS="$PYTHON_CPPFLAGS -I$mm_val"])
])
AC_MSG_RESULT([$PYTHON_CPPFLAGS])

AC_MSG_CHECKING([for Python libraries])
AS_IF([test "x$PYTHON_LIBS" = x],
[
  _MM_PYTHON_SYSCONFIG([[get_config_var('LIBS')]], [PYTHON_LIBS=$mm_val])
  set X
dnl On Windows the library is in libs/, not in lib/, so check there as well:
  _MM_PYTHON_SYSCONFIG([[EXEC_PREFIX]], [set "[$]@" "$mm_val/lib" "$mm_val/libs" "$mm_val/lib64" "$mm_val/lib/i386-linux-gnu" "$mm_val/lib/x86_64-linux-gnu"])
  _MM_PYTHON_SYSCONFIG([[PREFIX]],      [set "[$]@" "$mm_val/lib" "$mm_val/libs" "$mm_val/lib64" "$mm_val/lib/i386-linux-gnu" "$mm_val/lib/x86_64-linux-gnu"])
  _MM_PYTHON_SYSCONFIG([[get_python_lib(True, True)]],  [set "[$]@" "$mm_val/config" "$mm_val"])
  _MM_PYTHON_SYSCONFIG([[get_python_lib(False, True)]], [set "[$]@" "$mm_val/config" "$mm_val"])
  shift
  mm_pylib=python$PYTHON_VERSION
  mm_pylib_win=`echo "$mm_pylib" | sed 's/\.//g'`

  for mm_dir
  do
    AS_IF([test -f "$mm_dir/lib$mm_pylib.so" || \
           test -f "$mm_dir/lib$mm_pylib.a"],
          [AS_CASE([$mm_dir], [[/usr/lib|/usr/lib64]],
                   [PYTHON_LIBS="$PYTHON_LIBS -l$mm_pylib"; break],
                   [PYTHON_LIBS="$PYTHON_LIBS -L$mm_dir -l$mm_pylib"; break])],
          [test -f "$mm_dir/lib$mm_pylib_win.dll.a" || \
           test -f "$mm_dir/lib$mm_pylib_win.a" || \
           test -f "$mm_dir/$mm_pylib_win.lib"],
          [PYTHON_LIBS="$PYTHON_LIBS -L$mm_dir -l$mm_pylib_win"; break])
  done
])
AC_MSG_RESULT([$PYTHON_LIBS])

mm_save_CPPFLAGS=$CPPFLAGS
mm_save_LIBS=$LIBS
CPPFLAGS="$CPPFLAGS $PYTHON_CPPFLAGS"
LIBS="$LIBS $PYTHON_LIBS"
AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <Python.h>]],
                                [[(void) PyImport_ImportModule((char*)"sys");]])],,
               [AC_MSG_FAILURE([[Failed to compile test program for Python embedding.]])])
CPPFLAGS=$mm_save_CPPFLAGS
LIBS=$mm_save_LIBS[]dnl
])
