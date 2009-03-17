SUBDIRS = 

gladedir = $(datadir)/glom/glade/

# We define GLOM_GLADEDIR so that we can get the installed .glade file at runtime.
# We define PREFIX, SYSCONFDIR, LIBDIR, and DATADIR for the GNOME_PROGRAM_STANDARD_PROPERTIES macro.
AM_CPPFLAGS = -I@top_srcdir@/ -I@top_srcdir@/glom $(GLOM_CFLAGS) $(PYTHON_INCLUDES) \
           -DGLOM_GLADEDIR=\""$(gladedir)/"\"


h_sources = View.h ViewBase.h View_Composite.h 
cc_sources = View.cc ViewBase.cc View_Composite.cc 

noinst_LTLIBRARIES = libdocument_bakery_view.la
libdocument_bakery_view_la_SOURCES = $(h_sources) $(cc_sources)

