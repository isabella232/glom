#/bin/sh -e

for x in $(find ${srcdir}/glom/ -name "*.glade")
do
  # echo Validating $x
  # TODO: Is there a Glade DTD?
  xmllint --noout $x || exit 1
done
