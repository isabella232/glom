SUBDIRS = view

AM_CPPFLAGS = -I@top_srcdir@/ -I@top_srcdir@/glom $(LIBGLOM_CFLAGS) $(GLOM_WARNING_FLAGS)

h_sources = document.h document_xml.h
cc_sources = document.cc document_xml.cc

library_includedir=$(includedir)/glom-1.0/libglom/document/bakery
library_include_HEADERS = $(h_sources)

noinst_LTLIBRARIES = libdocument_bakery.la
libdocument_bakery_la_SOURCES = $(h_sources) $(cc_sources)

 




