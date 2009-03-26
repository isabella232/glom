SUBDIRS = 

gladedir = $(datadir)/glom/glade/

# We define GLOM_GLADEDIR so that we can get the installed .glade file at runtime.
# We define PREFIX, SYSCONFDIR, LIBDIR, and DATADIR for the GNOME_PROGRAM_STANDARD_PROPERTIES macro.
AM_CPPFLAGS = -I@top_srcdir@/ -I@top_srcdir@/glom $(GLOM_CFLAGS) $(GLOM_WARNING_FLAGS) $(PYTHON_INCLUDES) \
           -DGLOM_GLADEDIR=\""$(gladedir)/"\"


h_sources = view.h viewbase.h view_composite.h 
cc_sources = view.cc viewbase.cc view_composite.cc 

noinst_LTLIBRARIES = libdocument_bakery_view.la
libdocument_bakery_view_la_SOURCES = $(h_sources) $(cc_sources)

