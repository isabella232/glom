#! /bin/sh -e
test -n "$srcdir" || srcdir=`dirname "$0"`
test -n "$srcdir" || srcdir=.
(
  cd "$srcdir" &&
  gnome-doc-common --copy && # From gnome-common
  gnome-doc-prepare --automake --copy --force && #From gnome-doc-utis
  mm-common-prepare --copy --force && # From mm-common
  autopoint --force &&
  AUTOPOINT='intltoolize --automake --copy' autoreconf --force --install
) || exit
test -n "$NOCONFIGURE" || "$srcdir/configure" --enable-maintainer-mode "$@"
