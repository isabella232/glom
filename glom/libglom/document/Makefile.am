AM_CPPFLAGS = -I@top_srcdir@/ -I@top_srcdir@/glom $(GLOM_CFLAGS)

h_sources = document_glom.h view.h
cc_sources = document_glom.cc

library_includedir=$(includedir)/glom-1.0/libglom/document
library_include_HEADERS = $(h_sources)

noinst_LTLIBRARIES = libdocument.la
libdocument_la_SOURCES = $(h_sources) $(cc_sources)
