AM_CPPFLAGS = -I@top_srcdir@/ -I@top_srcdir@/glom $(GLOM_CFLAGS) $(GLOM_WARNING_FLAGS) $(PYTHON_INCLUDES) \
           -DGLOM_ABI_VERSION=\""$(GLOM_ABI_VERSION)\"" \
           -DGLOM_ABI_VERSION_UNDERLINED=\""$(GLOM_ABI_VERSION_UNDERLINED)\""

SUBDIRS = python_module

noinst_LIBRARIES = libpython_embed.a
libpython_embed_a_SOURCES = glom_python.h glom_python.cc
#libpython_embed_a_LIBADD = python_module/libglom.a


#libpython_embed_a_LIBADD = boost/python/libpython_embed_boost_python.a
