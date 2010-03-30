#/bin/sh -e

for x in $(find ${srcdir}/glom/ -name "*.glade")
do
  # echo glade_toplevels_instantiation $x
  tests/glade_toplevels_instantiation $x || exit 1
done
