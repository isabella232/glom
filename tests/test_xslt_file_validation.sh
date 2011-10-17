#/bin/sh -e

for x in $(find ${srcdir}/examples/ -name "*.glom")
do
  # echo Validating $x
  # Note that there is no DTD for XSL because DTD is not capable enough.
  xmllint --noout $x || exit 1
done
