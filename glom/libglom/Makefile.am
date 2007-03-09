SUBDIRS = data_structure document python_embed

gladedir = $(datadir)/glom/glade/
glomxsltdir = $(datadir)/glom/xslt/
glom_examples_dir = $(pkgdatadir)/doc/examples/
glom_icon_dir = $(datadir)/icons/hicolor/48x48/apps

# We define GLOM_GLADEDIR so that we can get the installed .glade file at runtime.
# We define PREFIX, SYSCONFDIR, LIBDIR, and DATADIR for the GNOME_PROGRAM_STANDARD_PROPERTIES macro.
INCLUDES = -I@top_srcdir@/ $(GLOM_CFLAGS) $(PYTHON_INCLUDES) \
           -DGLOM_GLADEDIR=\""$(gladedir)/"\" \
           -DLOCALEDIR=\""$(datadir)/locale"\" \
           -DPREFIX=\""$(prefix)"\" \
           -DSYSCONFDIR=\""$(sysconfdir)"\" \
           -DLIBDIR=\""$(libdir)"\" \
           -DDATADIR=\""$(datadir)"\" \
           -DGLOM_XSLTDIR=\""$(glomxsltdir)/"\" \
           -DGLOM_EXAMPLES_DIR=\""$(glom_examples_dir)"\" \
           -DGLOM_ICON_DIR=\""$(glom_icon_dir)"\"

lib_LTLIBRARIES = libglom.la
libglom_la_SOURCES = appstate.h appstate.cc \
                     calcinprogress.h calcinprogress.cc \
                     connectionpool.h connectionpool.cc \
                     dialog_progress_creating.h dialog_progress_creating.cc \
                     spawn_with_feedback.h spawn_with_feedback.cc \
                     utils.h utils.cc \
                     sharedptr.h \
                     standard_table_prefs_fields.h \
                     avahi_publisher.h avahi_publisher.cc \
		     gst-package.h gst-package.c

libglom_la_LDFLAGS = -no-undefined

libglom_la_LIBADD  = document/libdocument.la \
                     data_structure/libdata_structure.la \
                     data_structure/layout/libdata_structure_layout.la \
                     data_structure/layout/report_parts/libdata_structure_layout_reportparts.la \
                     python_embed/libglom_python_embed.la \
                     $(GLOM_LIBS) $(PYTHON_LDFLAGS) -lutil


noinst_PROGRAMS = test_sharedptr_layoutitem test_avahi_publisher
test_sharedptr_layoutitem_SOURCES = test_sharedptr_layoutitem.cc
test_sharedptr_layoutitem_LDADD =  libglom.la \
              $(GLOM_LIBS) -lutil

test_avahi_publisher_SOURCES = test_avahi_publisher.cc
test_avahi_publisher_LDADD =  libglom.la \
              $(GLOM_LIBS) -lutil
                   
