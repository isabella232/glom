#/bin/sh -e

for x in $(find ${srcdir}/examples/ -name "*.glom")
do
  # echo Validating $x
  xmllint --noout --dtdvalid ${srcdir}/glom/glom_document.dtd $x || exit 1
done
