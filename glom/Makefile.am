SUBDIRS = data_structure document mode_data mode_design mode_find navigation utility_widgets

gladedir = $(datadir)/glom/glade/

# We define GLOM_GLADEDIR so that we can get the installed .glade file at runtime.
# We define PREFIX, SYSCONFDIR, LIBDIR, and DATADIR for the GNOME_PROGRAM_STANDARD_PROPERTIES macro.
INCLUDES = $(GLOM_CFLAGS) $(PYTHON_INCLUDES) \
           -DGLOM_GLADEDIR=\""$(gladedir)/"\" \
           -DLOCALEDIR=\""$(datadir)/locale"\" \
           -DPREFIX=\""$(prefix)"\" \
           -DSYSCONFDIR=\""$(sysconfdir)"\" \
           -DLIBDIR=\""$(libdir)"\" \
           -DDATADIR=\""$(datadir)"\"

bin_PROGRAMS = glom test_pyembed

glom_SOURCES = main.cc \
               application.cc application.h \
               appstate.cc appstate.h \
               base_db.h base_db.cc \
               box_db.h box_db.cc \
               box_db_table.h box_db_table.cc \
               dialog_glom.h dialog_glom.cc \
               frame_glom.h frame_glom.cc \
               notebook_glom.h notebook_glom.cc \
               connectionpool.h connectionpool.cc \
               dialog_connection.h dialog_connection.cc \
               dialog_invalid_data.h dialog_invalid_data.cc \
               dialog_new_database.h dialog_new_database.cc \
               glom_python.h glom_python.cc \
               sharedptr.h

glom_LDADD = document/libdocument.a \
             data_structure/libdata_structure.a \
             data_structure/layout/libdata_structure_layout.a \
             mode_data/libmode_data.a \
             mode_find/libmode_find.a \
             mode_design/libmode_design.a \
             mode_design/fields/libmode_design_fields.a \
             navigation/libnavigation.a \
             utility_widgets/libutility_widgets.a \
             utility_widgets/adddel/libutility_widgets_adddel.a \
             utility_widgets/adddel/eggcolumnchooser/libutility_widgets_adddel_eggcolumnchooserdialog.a \
             utility_widgets/db_adddel/libutility_widgets_db_adddel.a \  
             $(GLOM_LIBS) $(PYTHON_LDFLAGS) -lutil

#libutil seems to be a fairly standard GNU library, and python needs it for openpty() and forkpty().


test_pyembed_SOURCES = test_pyembed.cc
test_pyembed_LDADD = $(GLOM_LIBS) $(PYTHON_LDFLAGS) -lutil


#Install the .dtd file:
glomdatadir = $(pkgdatadir)
glomdata_DATA = glom_document.dtd

# Install the glade file:
# The glade prefix of glade_DATA seems to cause it to be installed in <appname>/glade/
# Because that's how gladedir is defined.
glade_DATA = glom.glade

EXTRA_DIST = glom_document.dtd $(glade_DATA)


