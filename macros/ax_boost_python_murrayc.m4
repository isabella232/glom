# ===========================================================================
#         http://www.nongnu.org/autoconf-archive/ax_boost_python.html
# ===========================================================================
#
# SYNOPSIS
#
#   AX_BOOST_PYTHON_MURRAYC (based on AX_BOOST_PYTHON)
#
# DESCRIPTION
#
#   This macro checks to see if the Boost.Python library is installed. It
#   also attempts to guess the currect library name using several attempts.
#   It tries to build the library name using a user supplied name or suffix
#   and then just the raw library.
#
#   If the library is found, HAVE_BOOST_PYTHON is defined and
#   BOOST_PYTHON_LIBS is set to the name of the library.
#
#   This macro calls AC_SUBST(BOOST_PYTHON_LIBS).
#
#   In order to ensure that the Python headers are specified on the include
#   path, this macro requires AX_PYTHON to be called.
#
# LICENSE
#
#   Copyright (c) 2008 Michael Tindal
#
#   This program is free software; you can redistribute it and/or modify it
#   under the terms of the GNU General Public License as published by the
#   Free Software Foundation; either version 2 of the License, or (at your
#   option) any later version.
#
#   This program is distributed in the hope that it will be useful, but
#   WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
#   Public License for more details.
#
#   You should have received a copy of the GNU General Public License along
#   with this program. If not, see <http://www.gnu.org/licenses/>.
#
#   As a special exception, the respective Autoconf Macro's copyright owner
#   gives unlimited permission to copy, distribute and modify the configure
#   scripts that are the output of Autoconf when processing the Macro. You
#   need not follow the terms of the GNU General Public License when using
#   or distributing such scripts, even though portions of the text of the
#   Macro appear in them. The GNU General Public License (GPL) does govern
#   all other use of the material that constitutes the Autoconf Macro.
#
#   This special exception to the GPL applies to versions of the Autoconf
#   Macro released by the Autoconf Archive. When you make and distribute a
#   modified version of the Autoconf Macro, you may extend this special
#   exception to the GPL to apply to your modified version as well.

#serial 7
#With large changes by murrayc

#Note that this previously said it was checking for the library, but it's techically the both the headers and library that it looks for. murrayc
AC_DEFUN([AX_BOOST_PYTHON_MURRAYC],
[AC_REQUIRE([MM_CHECK_MODULE_PYTHON])dnl
AC_CACHE_CHECK(whether the Boost::Python headers are available,
ac_cv_boost_python,
[AC_LANG_SAVE
 AC_LANG_CPLUSPLUS
 CPPFLAGS_SAVE=$CPPFLAGS
 # Hacked to use the output of MM_CHECK_MODULE_PYTHON() instead of the outout of AX_PYTHON, which we don't use. murrayc
 # if test x$PYTHON_INCLUDE_DIR != x; then
 #  CPPFLAGS=-I$PYTHON_INCLUDE_DIR $CPPFLAGS
 #fi
 if test x$PYTHON_CPPFLAGS != x; then
   #Note that this expects boost/ to be at some top-level such as /usr/include/
   #We couldn't check for anything else anyway because there's no pkg-config file to tell us where it is
   CPPFLAGS=$PYTHON_CPPFLAGS $CPPFLAGS
 fi
 AC_COMPILE_IFELSE(AC_LANG_PROGRAM([[
 #include <boost/python/module.hpp>
 using namespace boost::python;
 BOOST_PYTHON_MODULE(test) { throw "Boost::Python test."; }]],
 			   [[return 0;]]),
  			   ac_cv_boost_python=yes, ac_cv_boost_python=no)
 AC_LANG_RESTORE
 CPPFLAGS=$CPPFLAGS_SAVE
])
if test "$ac_cv_boost_python" = "yes"; then
  AC_DEFINE(HAVE_BOOST_PYTHON,,[define if the Boost::Python headers and library are available])
  ax_python_lib=boost_python
  AC_ARG_WITH([boost-python],AS_HELP_STRING([--with-boost-python],[specify the boost python shared library to use. For instance, --with-boost-python=boost_python-py25. Defaults to boost-python. If you use this then you should probably set PYTHON too, to avoid using multiple python versions.]),
  [if test "x$with_boost_python" != "xno"; then
     ax_python_lib=$with_boost_python
     ax_boost_python_lib=boost_python-$with_boost_python
   fi])
  for ax_lib in $ax_python_lib $ax_boost_python_lib boost_python; do
    #This previously checked for an exit() function in the boost::python library.
    #That was strange and apparently no longer works. murrayc.
    #AC_CHECK_LIB($ax_lib, exit, [BOOST_PYTHON_LIB=$ax_lib break])
    AC_LANG_SAVE
    AC_LANG_CPLUSPLUS
    #Note that this requires PYTHON_CPPFLAGS and PYTHON_LIBS from MM_CHECK_MODULE_PYTHON()
    SAVED_CPPFLAGS=$CPPFLAGS
    CPPFLAGS=$PYTHON_CPPFLAGS $CPPFLAGS
    SAVED_LDFLAGS=$LDFLAGS
    LDFLAGS="$LDFLAGS $PYTHON_LIBS -l$ax_lib"
    AC_LINK_IFELSE(
      [AC_LANG_PROGRAM([#include <boost/python.hpp>],
        [boost::python::object test_object])],
      #Note that this was previously BOOST_PYTHON_LIB, but I renamed it because people expect *LIBS. murrayc.
      [BOOST_PYTHON_LIBS="-l$ax_lib"])
    LDFLAGS=$SAVED_LDFLAGS
    CPPFLAGS=$SAVED_CPPFLAGS
    AC_LANG_RESTORE
  done
  if test x$BOOST_PYTHON_LIBS != x; then
    AC_MSG_RESULT([boost::python shared library found: $BOOST_PYTHON_LIBS])
  fi
  #TODO: Figure out how to do a simple if else:
  if test x$BOOST_PYTHON_LIBS == x; then
    AC_MSG_ERROR([boost::python shared library not found])
  fi
  AC_SUBST(BOOST_PYTHON_LIBS)
fi
])dnl
