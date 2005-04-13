INCLUDES = -I@top_srcdir@/ $(GLOM_CFLAGS) $(PYTHON_INCLUDES)

SUBDIRS =

libpython_embed.a : libglom.la

noinst_LIBRARIES = libpython_embed.a
libpython_embed_a_SOURCES = glom_python.h glom_python.cc
libpython_embed_a_LIBADD = .libs/libglom.a

# Build and install the python module:
pkgpyexec_LTLIBRARIES = libglom.la
libglom_la_LDFLAGS = $(common_ldflags) -export-symbols-regex initpyglomrecord
#libglom_la_LIBADD = $(GDA_LIBS)
libglom_la_SOURCES = py_glom_record.cc py_glom_record.h


#libpython_embed_a_LIBADD = boost/python/libpython_embed_boost_python.a
