AM_CPPFLAGS = -I@top_srcdir@/ -I@top_srcdir@/glom $(LIBGLOM_CFLAGS) $(GLOM_WARNING_FLAGS)

noinst_LTLIBRARIES = libdata_structure_layout_reportparts.la

h_sources = layoutitem_groupby.h \
                              layoutitem_summary.h \
                              layoutitem_fieldsummary.h \
                              layoutitem_header.h \
                              layoutitem_footer.h \
                              layoutitem_verticalgroup.h

cc_sources = layoutitem_groupby.cc \
                              layoutitem_summary.cc \
                              layoutitem_fieldsummary.cc \
                              layoutitem_header.cc \
                              layoutitem_footer.cc \
                              layoutitem_verticalgroup.cc

library_includedir=$(includedir)/glom-1.0/libglom/data_structure/layout/report_parts
library_include_HEADERS = $(h_sources)

libdata_structure_layout_reportparts_la_SOURCES = $(h_sources) $(cc_sources)
