SUBDIRS = data_structure document mode_data mode_design mode_find navigation utility_widgets

gladedir = $(datadir)/glom/glade/

# We define GLOM_GLADEDIR so that we can get the installed .glade file at runtime.
# We define PREFIX, SYSCONFDIR, LIBDIR, and DATADIR for the GNOME_PROGRAM_STANDARD_PROPERTIES macro.
INCLUDES = $(GLOM_CFLAGS) \
           -DGLOM_GLADEDIR=\""$(gladedir)/"\" \
           -DLOCALEDIR=\""$(datadir)/locale"\" \
           -DPREFIX=\""$(prefix)"\" \
           -DSYSCONFDIR=\""$(sysconfdir)"\" \
           -DLIBDIR=\""$(libdir)"\" \
           -DDATADIR=\""$(datadir)"\"

bin_PROGRAMS = glom

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
               dialog_invalid_data.h dialog_invalid_data.cc \
               sharedptr.h

glom_LDADD = document/libdocument.a \
             data_structure/libdata_structure.a \
             mode_data/libmode_data.a \
             mode_find/libmode_find.a \
             mode_design/libmode_design.a \
             mode_design/fields/libmode_design_fields.a \
             navigation/libnavigation.a \
             utility_widgets/libutility_widgets.a \
             utility_widgets/adddel/libutility_widgets_adddel.a \
			 utility_widgets/adddel/eggcolumnchooser/libutility_widgets_adddel_eggcolumnchooserdialog.a \
             $(GLOM_LIBS)

#Install the .dtd file:
glomdatadir = $(pkgdatadir)
glomdata_DATA = glom_document.dtd

# Install the glade file:
# The glade prefix of glade_DATA seems to cause it to be installed in <appname>/glade/
# Because that's how gladedir is defined.
glade_DATA = glom.glade

EXTRA_DIST = glom_document.dtd $(glade_DATA)
