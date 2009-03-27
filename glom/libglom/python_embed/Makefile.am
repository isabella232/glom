SUBDIRS =

AM_CPPFLAGS = -I@top_srcdir@/ -I@top_srcdir@/glom  $(LIBGLOM_CFLAGS) $(GLOM_WARNING_FLAGS) $(PYTHON_INCLUDES)

noinst_LTLIBRARIES = libglom_python_embed.la
libglom_python_embed_la_SOURCES = py_glom_record.h py_glom_record.cc \
                                 py_glom_related.h py_glom_related.cc \
                                 py_glom_relatedrecord.h py_glom_relatedrecord.cc \
                                 pygdavalue_conversions.h pygdavalue_conversions.c



