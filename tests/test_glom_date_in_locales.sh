#/bin/sh -e

locales=("en_US.UTF-8" "en_GB.UTF-8" "en_CA.UTF-8" "de_DE.UTF-8" "fr_FR.UTF-8" "hu_HU.UTF-8")

for x in "${locales[@]}"
do
  echo testing with LANG="$x"
  export LANG="$x"
  ${buildir}glom --debug-date-check || exit 1
done
