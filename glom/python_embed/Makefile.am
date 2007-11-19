AM_CPPFLAGS = -I@top_srcdir@/ $(GLOM_CFLAGS) $(PYTHON_INCLUDES)

SUBDIRS = python_module

noinst_LIBRARIES = libpython_embed.a
libpython_embed_a_SOURCES = glom_python.h glom_python.cc
#libpython_embed_a_LIBADD = python_module/libglom.a


#libpython_embed_a_LIBADD = boost/python/libpython_embed_boost_python.a
