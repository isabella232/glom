SUBDIRS = connectionpool_backends data_structure document python_embed

gladedir = $(datadir)/glom/glade/
glomxsltdir = $(datadir)/glom/xslt/
glom_examples_dir = $(pkgdatadir)/doc/examples/
glom_icon_dir = $(datadir)/icons/hicolor/48x48/apps

# We define GLOM_GLADEDIR so that we can get the installed .glade file at runtime.
# We define PREFIX, SYSCONFDIR, LIBDIR, and DATADIR for the GNOME_PROGRAM_STANDARD_PROPERTIES macro.
AM_CPPFLAGS = -I@top_srcdir@/ -I@top_srcdir@/glom $(LIBGLOM_CFLAGS) $(GLOM_WARNING_FLAGS) $(PYTHON_INCLUDES) \
           -DGLOM_GLADEDIR=\""$(gladedir)/"\" \
           -DLOCALEDIR=\""$(datadir)/locale"\" \
           -DPREFIX=\""$(prefix)"\" \
           -DSYSCONFDIR=\""$(sysconfdir)"\" \
           -DLIBDIR=\""$(libdir)"\" \
           -DDATADIR=\""$(datadir)"\" \
           -DEXEEXT=\""$(EXEEXT)"\" \
           -DGLOM_XSLTDIR=\""$(glomxsltdir)/"\" \
           -DGLOM_EXAMPLES_DIR=\""$(glom_examples_dir)"\" \
           -DGLOM_ICON_DIR=\""$(glom_icon_dir)"\"

# These are installed, for use by other applications/utilties:
h_sources_public = libglom_config.h \
                     init.h \
                     sharedptr.h \
                     standard_table_prefs_fields.h \
                     appstate.h
                     
h_sources = $(h_sources_public) \
                     calcinprogress.h \
                     connectionpool.h \
                     spawn_with_feedback.h \
                     utils.h \
                     gst-package.h

cc_sources = appstate.cc \
                     init.cc \
                     calcinprogress.cc \
                     connectionpool.cc \
                     spawn_with_feedback.cc \
                     utils.cc \
                     gst-package.c

library_includedir=$(includedir)/glom-$(GLOM_ABI_VERSION)/libglom
library_include_HEADERS = $(h_sources_public)

lib_LTLIBRARIES = libglom-1.12.la
libglom_1_12_la_SOURCES = $(h_sources) $(cc_sources)

libglom_1_12_la_LDFLAGS = -no-undefined

# Link libglom statically on maemo and windows for now.
if GLOM_ENABLE_MAEMO
libglom_1_12_la_LDFLAGS += -static
endif

libglom_1_12_la_LIBADD  = document/libdocument.la \
                     document/bakery/libdocument_bakery.la \
                     document/bakery/view/libdocument_bakery_view.la \
                     connectionpool_backends/libconnectionpool_backends.la \
                     data_structure/libdata_structure.la \
                     data_structure/layout/libdata_structure_layout.la \
                     data_structure/layout/report_parts/libdata_structure_layout_reportparts.la \
                     python_embed/libglom_python_embed.la \
                     $(LIBGLOM_LIBS) $(PYTHON_LDFLAGS)

if !WIN32
libglom_1_12_la_LIBADD += -lutil
endif

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = glom-1.0.pc



#Small test programs:
noinst_PROGRAMS = test_sharedptr_layoutitem test_connectionpool test_document

test_sharedptr_layoutitem_SOURCES = test_sharedptr_layoutitem.cc
test_sharedptr_layoutitem_LDADD =  libglom-1.12.la \
              $(LIBGLOM_LIBS)

test_connectionpool_SOURCES = test_connectionpool.cc
test_connectionpool_LDADD =  libglom-1.12.la \
              $(LIBGLOM_LIBS)

test_document_SOURCES = test_document.cc
test_document_LDADD =  libglom-1.12.la \
              $(LIBGLOM_LIBS)

if !WIN32
test_sharedptr_layoutitem_LDADD += -lutil
test_connectionpool_LDADD += -lutil
endif

