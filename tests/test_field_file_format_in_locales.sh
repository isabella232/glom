#/bin/sh -e

# This test checks that creating from examples works in non-English locales.
# This would break if libgda uses locale-dependent code to convert between numbers
# and internal text, for instance in GdaNumeric, as it has in the past.
#
# This test requires these locales to be installed and configured.
# That might be a problem on some systems, so feel free to use a patch to edit this file or disable it altogether.
#
# These are chosen based on problems found previously,
# and the ones with good translations shown here: http://l10n.gnome.org/module/glom/
# TODO: Get a list from po/*.po ?
#
# On debian/Ubuntu do this: 
#  sudo apt-get install language-pack-de language-pack-es language-pack-fi language-pack-fr language-pack-hu language-pack-it language-pack-pt language-pack-sl language-pack-da language-pack-cz language-pack-nb language-pack-se
#
# These are apparently not available on Fedora:  "da_DK.UTF-8" "cs_CZ.UTF-8" "nb_NO.UTF-8" "sv_SE.UTF-8"
locales=("en_US.UTF-8" "en_GB.UTF-8" "en_CA.UTF-8" "de_DE.UTF-8" "es_ES.UTF-8" "fi_FI.UTF-8" "fr_FR.UTF-8" "hu_HU.UTF-8" "it_IT.UTF-8" "pt_PT.UTF-8" "pt_BR.UTF-8" "sl_SI.UTF-8" "da_DK.UTF-8" "cs_CZ.UTF-8" "nb_NO.UTF-8" "sv_SE.UTF-8")

for x in "${locales[@]}"
do
  echo testing with LANG and LANGUAGE="$x"
  export LANG="$x"
  export LANGUAGE="$x"
  export LC_TIME="$x"
  tests/test_field_file_format || exit 1
done
