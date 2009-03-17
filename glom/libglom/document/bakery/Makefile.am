SUBDIRS = view

AM_CPPFLAGS = -I@top_srcdir@/ -I@top_srcdir@/glom $(GLOM_CFLAGS)

h_sources = Document.h Document_XML.h
cc_sources = Document.cc Document_XML.cc

noinst_LTLIBRARIES = libdocument_bakery.la
libdocument_bakery_la_SOURCES = $(h_sources) $(cc_sources)

 




