SUBDIRS = layout

AM_CPPFLAGS = -I@top_srcdir@/ -I@top_srcdir@/glom $(LIBGLOM_CFLAGS) $(GLOM_WARNING_FLAGS)

h_sources = field.h \
                              foundset.h \
                              relationship.h \
                              fieldtypes.h \
                              tableinfo.h \
                              glomconversions.h \
                              privileges.h \
                              groupinfo.h \
                              numeric_format.h \
                              iso_codes.h \
                              system_prefs.h \
                              print_layout.h \
                              report.h \
                              translatable_item.h \
                              parameternamegenerator.h
cc_sources = field.cc \
                              foundset.cc \
                              relationship.cc \
                              fieldtypes.cc \
                              tableinfo.cc \
                              glomconversions.cc \
                              privileges.cc \
                              groupinfo.cc \
                              numeric_format.cc \
                              iso_codes.cc \
                              system_prefs.cc \
                              print_layout.cc \
                              report.cc \
                              translatable_item.cc \
                              parameternamegenerator.cc

library_includedir=$(includedir)/glom-1.0/libglom/data_structure
library_include_HEADERS = $(h_sources)

noinst_LTLIBRARIES = libdata_structure.la
libdata_structure_la_SOURCES = $(h_sources) $(cc_sources)

 




