SUBDIRS = fields users

gladedir = $(datadir)/glom/glade/

# We define GLOM_GLADEDIR so that we can get the installed .glade file at runtime.
# We define PREFIX, SYSCONFDIR, LIBDIR, and DATADIR for the GNOME_PROGRAM_STANDARD_PROPERTIES macro.
INCLUDES = $(GLOM_CFLAGS) $(PYTHON_INCLUDES) $(BOOST_CFLAGS) \
           -DGLOM_GLADEDIR=\""$(gladedir)/"\"

noinst_LIBRARIES = libmode_design.a
libmode_design_a_SOURCES = box_db_table_relationships.cc box_db_table_relationships.h \
                                                           dialog_fields.cc dialog_fields.h  \
                                                           dialog_design.cc dialog_design.h \
                                                           dialog_relationships.cc dialog_relationships.h

libmode_design_a_LIBADD = fields/libmode_design_fields.a \
                          users/libmode_design_users.a
