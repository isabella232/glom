SUBDIRS = view

AM_CPPFLAGS = -I@top_srcdir@/ -I@top_srcdir@/glom $(GLOM_CFLAGS)

h_sources = document.h document_xml.h
cc_sources = document.cc document_xml.cc

noinst_LTLIBRARIES = libdocument_bakery.la
libdocument_bakery_la_SOURCES = $(h_sources) $(cc_sources)

 




