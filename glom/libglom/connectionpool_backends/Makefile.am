AM_CPPFLAGS = \
	-I@top_srcdir@/ $(GLOM_CFLAGS) \
	-DEXEEXT=\""$(EXEEXT)"\"

noinst_LTLIBRARIES = libconnectionpool_backends.la
libconnectionpool_backends_la_SOURCES = \
	postgres.h postgres.cc \
	postgres_central.h postgres_central.cc \
	sqlite.h sqlite.cc

if !GLOM_ENABLE_CLIENT_ONLY
libconnectionpool_backends_la_SOURCES += \
	postgres_self.h postgres_self.cc
endif
