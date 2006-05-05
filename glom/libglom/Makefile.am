INCLUDES = -I@top_srcdir@/ $(GLOM_CFLAGS) $(PYTHON_INCLUDES)

SUBDIRS = data_structure document python_embed

lib_LTLIBRARIES = libglom.la
libglom_la_SOURCES = appstate.h appstate.cc \
                     calcinprogress.h calcinprogress.cc \
                     connectionpool.h connectionpool.cc \
                     utils.h utils.cc \
                     sharedptr.h \
                     standard_table_prefs_fields.h

libglom_la_LIBADD  = document/libdocument.la \
                     data_structure/libdata_structure.la \
                     data_structure/layout/libdata_structure_layout.la \
                     data_structure/layout/report_parts/libdata_structure_layout_reportparts.la \
                     python_embed/libglom_python_embed.la
   
libglom_la_LDFLAGS = $(common_ldflags) \
                     $(GLOM_LIBS) $(PYTHON_LDFLAGS) -lutil
                   
#libglom_la__LDFLAGS =


#libglom_la_LIBADD = boost/python/libpython_embed_boost_python.a


