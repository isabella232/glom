SUBDIRS = report_parts

AM_CPPFLAGS = -I@top_srcdir@/ $(GLOM_CFLAGS)

noinst_LTLIBRARIES = libdata_structure_layout.la


h_sources = layoutitem.h \
                              layoutgroup.h\
                              layoutitem_field.h \
                              layoutitem_portal.h \
                              layoutitem_calendarportal.h \
                              layoutitem_notebook.h \
                              layoutitem_button.h \
                              layoutitem_text.h \
                              layoutitem_image.h \
                              layoutitem_line.h \
                              usesrelationship.h \
                              fieldformatting.h \
                              custom_title.h \
                              layoutitem_placeholder.h

cc_sources = layoutitem.cc \
                              layoutgroup.cc \
                              layoutitem_field.cc \
                              layoutitem_portal.cc \
                              layoutitem_calendarportal.cc \
                              layoutitem_notebook.cc \
                              layoutitem_button.cc \
                              layoutitem_text.cc \
                              layoutitem_image.cc \
                              layoutitem_line.cc \
                              usesrelationship.cc \
                              fieldformatting.cc \
                              custom_title.cc \
                              layoutitem_placeholder.cc

library_includedir=$(includedir)/glom-1.0/libglom/data_structure/layout
library_include_HEADERS = $(h_sources)

libdata_structure_layout_la_SOURCES = $(h_sources) $(cc_sources)

