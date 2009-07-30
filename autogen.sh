#! /bin/sh -e
test -n "$srcdir" || srcdir=`dirname "$0"`
test -n "$srcdir" || srcdir=.
(
  cd "$srcdir" &&
  gnome-doc-common --copy &&
  gnome-doc-prepare --automake --copy --force &&
  AUTOPOINT='intltoolize --automake --copy' ACLOCAL=aclocal-1.10 AUTOMAKE=automake-1.10 autoreconf --force --install
) || exit
test -n "$NOCONFIGURE" || "$srcdir/configure" "$@"
