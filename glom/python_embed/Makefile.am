INCLUDES = -I@top_srcdir@/ $(GLOM_CFLAGS) $(PYTHON_INCLUDES)

SUBDIRS = boost

noinst_LIBRARIES = libpython_embed.a
libpython_embed_a_SOURCES = glom_python.h glom_python.cc \
  py_glom_record.h py_glom_record.cc

#libpython_embed_a_LIBADD = boost/python/libpython_embed_boost_python.a
