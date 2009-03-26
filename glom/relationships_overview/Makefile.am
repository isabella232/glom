gladedir = $(datadir)/glom/glade/

# We define GLOM_GLADEDIR so that we can get the installed .glade file at runtime.
AM_CPPFLAGS = -I@top_srcdir@/ -I@top_srcdir@/glom $(GLOM_CFLAGS) $(GLOM_WARNING_FLAGS) \
           -DGLOM_GLADEDIR=\""$(gladedir)/"\"

noinst_LIBRARIES = librelationshipsoverview.a
librelationshipsoverview_a_SOURCES = dialog_relationships_overview.h dialog_relationships_overview.cc \
  printoperation_relationshipsoverview.h printoperation_relationshipsoverview.cc canvas_group_dbtable.h canvas_group_dbtable.cc

