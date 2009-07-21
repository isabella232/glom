SUBDIRS = bakery

AM_CPPFLAGS = -I@top_srcdir@/ -I@top_srcdir@/glom $(LIBGLOM_CFLAGS) $(GLOM_WARNING_FLAGS)

h_sources = document.h view.h
cc_sources = document.cc

library_includedir=$(includedir)/glom-$(GLOM_ABI_VERSION)/libglom/document
library_include_HEADERS = $(h_sources)

noinst_LTLIBRARIES = libdocument.la
libdocument_la_SOURCES = $(h_sources) $(cc_sources)
