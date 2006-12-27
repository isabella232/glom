INCLUDES = -I@top_srcdir@/ $(GLOM_CFLAGS) $(PYTHON_INCLUDES)

SUBDIRS = data_structure document python_embed

lib_LTLIBRARIES = libglom.la
libglom_la_SOURCES = appstate.h appstate.cc \
                     calcinprogress.h calcinprogress.cc \
                     connectionpool.h connectionpool.cc \
                     utils.h utils.cc \
                     sharedptr.h \
                     standard_table_prefs_fields.h

libglom_la_LDFLAGS = -no-undefined

libglom_la_LIBADD  = document/libdocument.la \
                     data_structure/libdata_structure.la \
                     data_structure/layout/libdata_structure_layout.la \
                     data_structure/layout/report_parts/libdata_structure_layout_reportparts.la \
                     python_embed/libglom_python_embed.la \
                     $(GLOM_LIBS) $(PYTHON_LDFLAGS) -lutil


noinst_PROGRAMS = test
test_SOURCES = test_sharedptr_layoutitem.cc
test_LDADD =  libglom.la \
              $(GLOM_LIBS) -lutil
                   
