SUBDIRS = 

gladedir = $(datadir)/glom/glade/

# We define GLOM_GLADEDIR so that we can get the installed .glade file at runtime.
# We define PREFIX, SYSCONFDIR, LIBDIR, and DATADIR for the GNOME_PROGRAM_STANDARD_PROPERTIES macro.
AM_CPPFLAGS = -I@top_srcdir@/ -I@top_srcdir@/glom $(GLOM_CFLAGS) $(GLOM_WARNING_FLAGS) $(PYTHON_INCLUDES) \
           -DGLOM_GLADEDIR=\""$(gladedir)/"\"

noinst_LIBRARIES = libmode_design_users.a
libmode_design_users_a_SOURCES = dialog_users_list.cc dialog_users_list.h \
                                                           dialog_groups_list.cc dialog_groups_list.h \
                                                           dialog_user.cc dialog_user.h \
                                                           dialog_new_group.cc dialog_new_group.h \
                                                           dialog_choose_user.cc dialog_choose_user.h
