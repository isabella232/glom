AM_CPPFLAGS = \
	-I@top_srcdir@/ $(GLOM_CFLAGS) \
	-DEXEEXT=\""$(EXEEXT)"\"

noinst_LTLIBRARIES = libconnectionpool_backends.la
libconnectionpool_backends_la_SOURCES = \
	postgres_central.h postgres_central.cc \
	postgres_self.h postgres_self.cc
