#/bin/sh -e

for x in $(find ${srcdir}/examples/ -name "*.glom")
do
  tests/test_document_load_and_save $x ${srcdir}/glom/glom_document.dtd || exit 1
done
