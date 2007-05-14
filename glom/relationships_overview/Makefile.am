gladedir = $(datadir)/glom/glade/

# We define GLOM_GLADEDIR so that we can get the installed .glade file at runtime.
INCLUDES = -I@top_srcdir@/ $(GLOM_CFLAGS) \
           -DGLOM_GLADEDIR=\""$(gladedir)/"\"

noinst_LIBRARIES = librelationshipsoverview.a
librelationshipsoverview_a_SOURCES = dialog_relationships_overview.h dialog_relationships_overview.cc
