## Copyright (c) 2009  Openismus GmbH  <http://www.openismus.com/>
##
## This file is part of Glom.
##
## Glom is free software: you can redistribute it and/or modify it
## under the terms of the GNU General Public License as published
## by the Free Software Foundation, either version 2 of the License,
## or (at your option) any later version.
##
## Glom is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
## See the GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program.  If not, see <http://www.gnu.org/licenses/>.

AC_INIT([Glom], [1.12.3], [http://bugzilla.gnome.org/enter_bug.cgi?product=Glom], [glom])
AC_PREREQ([2.60])

AC_CONFIG_SRCDIR([glom/main.cc])
AC_CONFIG_MACRO_DIR([macros])
AC_CONFIG_HEADERS([config.h glom/libglom/libglom_config.h])

AM_INIT_AUTOMAKE([1.10 check-news no-define nostdinc])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES])
AC_ARG_VAR([ACLOCAL_FLAGS], [aclocal flags, e.g. -I <macro dir>])

AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_CXX

dnl Maemo still ships with the ancient libtool 1.4
dnl LT_PREREQ([2.2.0])
dnl LT_INIT([disable-static win32-dll])
AC_DISABLE_STATIC
AC_LIBTOOL_WIN32_DLL
AC_PROG_LIBTOOL

# Used for the install directories for headers.  The same number is used in
# the library name, which must be kept in sync, but variables can't be used
# with that.
AC_SUBST([GLOM_ABI_VERSION], [1.12])
AC_DEFINE_UNQUOTED([GLOM_ABI_VERSION], ["$GLOM_ABI_VERSION"],
		   [Define to the Glom ABI version string.])
# Python modules can't be called glom-1.x, so we use underlines.
AC_DEFINE_UNQUOTED([GLOM_ABI_VERSION_UNDERLINED], ["AS_TR_SH([$GLOM_ABI_VERSION])"],
                   [Define to the Glom ABI version with '.' replaced by '_'.])

# Only check for libgettextpo on non-Maemo platforms.
AS_IF([test "x$glom_enable_maemo" = xyes],
      [
        # libgettext-po changed its API, changing the error handler struct
        # from po_error_handler to po_xerror_handler:
        AC_CACHE_CHECK([for po_xerror_handler], [glom_cv_has_po_xerror_handler],
               [AC_CHECK_MEMBER([struct po_xerror_handler.xerror],
                                [glom_cv_has_po_xerror_handler=yes],
                                [glom_cv_has_po_xerror_handler=no],
                                [#include <gettext-po.h>])])

        AS_IF([test "x$glom_cv_has_po_xerror_handler" = xyes],
          [AC_DEFINE([HAVE_GETTEXTPO_XERROR], [1],
          [Define if libgettextpo provides the new po_xerror_handler struct.])])
])

IT_PROG_INTLTOOL([0.35])
AC_SUBST([GETTEXT_PACKAGE], [glom])
AM_GNU_GETTEXT_VERSION([0.11])
AM_GLIB_GNU_GETTEXT
AC_DEFINE([GETTEXT_PACKAGE], [PACKAGE_TARNAME], [Define to the gettext package name.])

AC_MSG_CHECKING([whether to enable Windows specific options])
AS_CASE([$host_os], [mingw*], [glom_host_win32=yes], [glom_host_win32=no])
AC_MSG_RESULT([$glom_host_win32])

AS_IF([test "x$glom_host_win32" = xyes],
      [AC_CHECK_TOOL([WINDRES], [windres])])

AM_CONDITIONAL([HOST_WIN32], [test "x$glom_host_win32" = xyes])

# In most cases, you'll want all these options for the maemo build:
#   ./configure --prefix=/usr --enable-postgesql --enable-sqlite --enable-maemo;make all install
# And you might need to do:
#   export PYTHON=python2.5
#   export CXXFLAGS="-fno-exceptions"
AC_ARG_ENABLE([maemo],
              [AS_HELP_STRING([--enable-maemo],
                              [build with support for the Maemo platform
                               (implies --enable-client-only)])],
              [glom_enable_maemo=$enableval],
              [glom_enable_maemo=no])

AC_ARG_ENABLE([client-only],
              [AS_HELP_STRING([--enable-client-only],
                              [build without developer mode and self-hosting])],
              [glom_enable_client_only=$enableval],
              [glom_enable_client_only=$glom_enable_maemo])

# Bail out if --enable-maemo and --disable-client-only are used together.
AS_IF([test "x$glom_enable_maemo" = xyes && test "x$glom_enable_client_only" != xyes],
      [AC_MSG_ERROR([[Building with --enable-maemo also requires --enable-client-only.]])])

AM_CONDITIONAL([GLOM_ENABLE_MAEMO], [test "x$glom_enable_maemo" = xyes])
AM_CONDITIONAL([GLOM_ENABLE_CLIENT_ONLY], [test "x$glom_enable_client_only" = xyes])

AS_IF([test "x$glom_enable_maemo" = xyes],
      [AC_DEFINE([GLOM_ENABLE_MAEMO], [1],
                 [Define to enable support for the Maemo platform.])])
AS_IF([test "x$glom_enable_client_only" = xyes],
      [AC_DEFINE([GLOM_ENABLE_CLIENT_ONLY], [1],
                 [Define to disable support for self-hosting and developer mode.])])

AC_ARG_ENABLE([sqlite],
              [AS_HELP_STRING([--enable-sqlite],
                              [Allow creation of SQLite databases and opening
                               of documents using SQLite databases. This
                               disables some functionality and is intended
                               only for embedded use.])],
              [glom_enable_sqlite=$enableval],
              [glom_enable_sqlite=no])

AM_CONDITIONAL([GLOM_ENABLE_SQLITE], [test "x$glom_enable_sqlite" = xyes])

AS_IF([test "x$glom_enable_sqlite" != xyes],
      [AC_DEFINE([GLOM_ENABLE_SQLITE], [1],
                 [Whether to enable support for SQLite databases.])])

AC_ARG_ENABLE([postgresql],
              [AS_HELP_STRING([--disable-postgresql],
                              [do not build with support for PostgreSQL databases])],
              [glom_enable_postgresql=$enableval],
              [glom_enable_postgresql=yes])

AM_CONDITIONAL([GLOM_ENABLE_POSTGRESQL], [test "x$glom_enable_postgresql" = xyes])

AS_IF([test "x$glom_enable_postgresql" = xyes],
      [AC_DEFINE([GLOM_ENABLE_POSTGRESQL], [1],
                 [Whether to enable support for PostgreSQL databases.])])

AC_ARG_ENABLE([maemo-launcher],
              [AS_HELP_STRING([--enable-maemo-launcher],
                              [build with maemo-launcher support])],
              [glom_maemo_launcher=$enableval],
              [glom_maemo_launcher=no])

# Libraries used by libglom:
# We require libgda >= 4.0.4 because previous versions had case-sensitivity or fatal metastore problems.
# libgda >= 4.1.2 is also OK, but not 4.1.<2.
REQUIRED_LIBGLOM_LIBS='gthread-2.0 giomm-2.4 libxml++-2.6 pygda-4.0 >= 2.25.3 pygobject-2.0 >= 2.6.0 libgdamm-4.0 >= 3.99.14 libgda-4.0 >= 4.0.4 libgda-postgres-4.0'

AS_IF([test "x$glom_host_win32" != xyes],
      [REQUIRED_LIBGLOM_LIBS="$REQUIRED_LIBGLOM_LIBS libepc-1.0 >= 0.3.1 avahi-ui"])

# Libraries used by Glom:
REQUIRED_GLOM_LIBS="$REQUIRED_LIBGLOM_LIBS gtkmm-2.4 >= 2.14 gthread-2.0 gconfmm-2.6 libxml++-2.6 libxslt >= 1.1.10 goocanvasmm-1.0 >= 0.14.0"

# Do not require iso-codes in client-only mode, or on Windows:
# TODO: Package iso-codes for Windows?
AS_IF([test "x$glom_enable_client_only" != xyes && test "x$glom_host_win32" != xyes],
      [REQUIRED_GLOM_LIBS="$REQUIRED_GLOM_LIBS iso-codes"])

# Do not require gtksourceviewmm in client only mode
AS_IF([test "x$glom_enable_client_only" != xyes],
      [REQUIRED_GLOM_LIBS="$REQUIRED_GLOM_LIBS gtksourceviewmm-2.0"])

AS_IF([test "x$glom_enable_sqlite" = xyes],
      [REQUIRED_GLOM_LIBS="$REQUIRED_GLOM_LIBS libgda-sqlite-4.0"])
AS_IF([test "x$glom_enable_maemo" = xyes],
      [REQUIRED_GLOM_LIBS="$REQUIRED_GLOM_LIBS hildonmm hildon-fmmm libossomm osso-af-settings"
      MM_PKG_CONFIG_SUBST([OSSO_SERVICEDIR], [--variable=dbusservicedir osso-af-settings])
      MM_PKG_CONFIG_SUBST([OSSO_DESKTOPDIR], [--variable=desktopentrydir osso-af-settings])])
AS_IF([test "x$glom_maemo_launcher" = xyes],
      [REQUIRED_GLOM_LIBS="$REQUIRED_GLOM_LIBS maemo-launcher-app"])

PKG_CHECK_MODULES([LIBGLOM], [$REQUIRED_LIBGLOM_LIBS])
PKG_CHECK_MODULES([GLOM], [$REQUIRED_GLOM_LIBS])

# Flags for building the included libegg
PKG_CHECK_MODULES([EGG], [gthread-2.0 gtk+-2.0])

MM_PKG_CONFIG_SUBST([GLIB_GENMARSHAL], [--variable=glib_genmarshal glib-2.0])

# Get the location of the ISO-Codes (currencies, languages) files:
MM_PKG_CONFIG_SUBST([ISO_CODES_PREFIX], [--variable=prefix iso-codes])
AC_DEFINE_UNQUOTED([ISO_CODES_PREFIX], ["$ISO_CODES_PREFIX"],
                   [Define to the installation prefix of the iso-codes module.])

AC_LANG_ASSERT([C])
DK_ARG_ENABLE_WARNINGS([GLOM_WFLAGS],
                       [-Wall], [-Wall -Wextra],
                       [G GDK GDK_PIXBUF GTK])
AC_LANG([C++])
DK_ARG_ENABLE_WARNINGS([GLOM_WXXFLAGS],
		       [-Wall], [-Wall -Wextra -Wno-missing-field-initializers],
                       [G GDK GDK_PIXBUF GTK GLIBMM GTKMM])
AC_CHECK_FUNCS([strptime])

# Get the compiler and linker flags for embedding Python.
# To specify a particular python version set an environment variable.
# For instance: PYTHON=python2.5
MM_CHECK_MODULE_PYTHON

AC_ARG_ENABLE([update-mime-database],
              [AS_HELP_STRING([--disable-update-mime-database],
                              [do not run the update-mime-database utility
                               (mainly useful to package maintainers)])],
              [glom_update_mime_database=$enableval],
              [glom_update_mime_database=yes])

AM_CONDITIONAL([UPDATE_MIME_DATABASE], [test "x$glom_update_mime_database" != xno])

# Locate the directory containing the postgresql utilities, such as the
# postmaster executable, so we can self-host postgresql databases.
AC_ARG_WITH([postgres-utils],
            [AS_HELP_STRING([--with-postgres-utils=DIR],
                            [path to PostgreSQL utilities (overriding pg_config)])],
            [POSTGRES_UTILS_PATH=$withval])

# Path not needed on Windows
AS_IF([test "x$glom_host_win32" != xyes && test "x$glom_enable_client_only" != xyes],
      [AS_CASE([$POSTGRES_UTILS_PATH], [""|no|yes],
[
  POSTGRES_UTILS_PATH=`pg_config --bindir 2>&AS_MESSAGE_LOG_FD`
  AS_IF(["$POSTGRES_UTILS_PATH/pg_ctl" --version >/dev/null 2>&AS_MESSAGE_LOG_FD],,
        [AC_MSG_ERROR([[
The Postgres utilities could not be found. They are needed for
self-hosting of Glom databases. Please make sure that Postgres
is installed, and if necessary specify the correct directory
explicitly with the --with-postgres-utils option.
]])])])])
AC_DEFINE_UNQUOTED([POSTGRES_UTILS_PATH], ["$POSTGRES_UTILS_PATH"],
                   [Define to the location of the PostgreSQL utilities.])
AC_DEFINE_UNQUOTED([EXEEXT], ["$EXEEXT"],
                   [Define to the file extension of executables on the target.])

GNOME_DOC_INIT([0.9.0],,
  [AC_MSG_WARN([[gnome-doc-utils not found: documentation will not be built.]])])

AC_CONFIG_FILES([Makefile
                 docs/user-guide/Makefile
                 po/Makefile.in
                 glom.desktop.in
                 glom/libglom/glom-1.12.pc
                 win32/glom.iss])
AC_OUTPUT
