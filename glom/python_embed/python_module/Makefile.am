INCLUDES = -I@top_srcdir@/ $(GLOM_CFLAGS) $(PYTHON_INCLUDES)

SUBDIRS =

#This seems to stop libtool from complaining that glom.la is not named libglom.a
common_ldflags = -module -avoid-version

# Build and install the python module:
pyexec_LTLIBRARIES = glom.la
glom_la_LDFLAGS = $(common_ldflags) -export-symbols-regex initglom

glom_la_LIBADD = $(top_builddir)/glom/libglom/libglom.la \
                 $(GLOM_LIBS) $(PYTHON_LDFLAGS) -lutil

glom_la_SOURCES = py_glom_module.cc py_glom_module.h
