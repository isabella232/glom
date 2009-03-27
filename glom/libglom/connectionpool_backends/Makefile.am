AM_CPPFLAGS = \
	-I@top_srcdir@/ -I@top_srcdir@/glom $(LIBGLOM_CFLAGS) $(GLOM_WARNING_FLAGS) \
	-DEXEEXT=\""$(EXEEXT)"\"

noinst_LTLIBRARIES = libconnectionpool_backends.la
libconnectionpool_backends_la_SOURCES = \
	backend.h backend.cc


if GLOM_ENABLE_POSTGRESQL
libconnectionpool_backends_la_SOURCES += \
	postgres.h postgres.cc \
	postgres_central.h postgres_central.cc

if !GLOM_ENABLE_CLIENT_ONLY
libconnectionpool_backends_la_SOURCES += \
	postgres_self.h postgres_self.cc
endif

endif


if GLOM_ENABLE_SQLITE
libconnectionpool_backends_la_SOURCES += \
	sqlite.h sqlite.cc
endif
