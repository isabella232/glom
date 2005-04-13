INCLUDES = -I@top_srcdir@/ $(GLOM_CFLAGS) $(PYTHON_INCLUDES)

SUBDIRS =

# Build and install the python module:
pkgpyexec_LTLIBRARIES = libglom.la
libglom_la_LDFLAGS = $(common_ldflags) -export-symbols-regex initpyglomrecord
#libglom_la_LIBADD = $(GDA_LIBS)
libglom_la_SOURCES = py_glom_record.cc py_glom_record.h
