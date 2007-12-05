SUBDIRS = libglom mode_data mode_find navigation utility_widgets python_embed reports

if !GLOM_ENABLE_CLIENT_ONLY
SUBDIRS += layout_item_dialogs mode_design translation relationships_overview
endif

gladedir = $(datadir)/glom/glade
glomxsltdir = $(datadir)/glom/xslt/
glom_examples_dir = $(pkgdatadir)/doc/examples/
glom_icon_dir = $(datadir)/icons/hicolor/48x48/apps

bin_PROGRAMS = glom
noinst_PROGRAMS = test_pyembed

# We define GLOM_GLADEDIR so that we can get the installed .glade file at runtime.
# We define PREFIX, SYSCONFDIR, LIBDIR, and DATADIR for the GNOME_PROGRAM_STANDARD_PROPERTIES macro.
AM_CPPFLAGS = -I@top_srcdir@/ $(GLOM_CFLAGS) $(PYTHON_INCLUDES) \
           -DGLOM_GLADEDIR=\""$(gladedir)/"\" \
           -DLOCALEDIR=\""$(datadir)/locale"\" \
           -DPREFIX=\""$(prefix)"\" \
           -DSYSCONFDIR=\""$(sysconfdir)"\" \
           -DLIBDIR=\""$(libdir)"\" \
           -DDATADIR=\""$(datadir)"\" \
           -DGLOM_XSLTDIR=\""$(glomxsltdir)/"\" \
           -DGLOM_EXAMPLES_DIR=\""$(glom_examples_dir)"\" \
           -DGLOM_ICON_DIR=\""$(glom_icon_dir)"\"

glom_SOURCES = main.cc \
               application.cc application.h \
               base_db.h base_db.cc \
               box_db.h box_db.cc \
               box_db_table.h box_db_table.cc \
               glom_postgres.h glom_postgres.cc \
               glom_privs.h glom_privs.cc \
               dialog_glom.h dialog_glom.cc \
               frame_glom.h frame_glom.cc \
               notebook_glom.h notebook_glom.cc \
               combobox_fields.h combobox_fields.cc \
               combobox_relationship.h combobox_relationship.cc \
               dialog_connection.h dialog_connection.cc \
               dialog_invalid_data.h dialog_invalid_data.cc \
               filechooser_export.h filechooser_export.cc \
               box_reports.h box_reports.cc \
               xsl_utils.h xsl_utils.cc

if !GLOM_ENABLE_CLIENT_ONLY
glom_SOURCES += dialog_new_self_hosted_connection.h \
                dialog_new_self_hosted_connection.cc \
                dialog_database_preferences.h \
                dialog_database_preferences.cc \
                printoperation_printlayout.h \
		printoperation_printlayout.cc
endif

# Note that the order in which libraries are linked seems to be important.
glom_LDADD = reports/libreports.a \
             mode_data/libmode_data.a \
             mode_find/libmode_find.a

if !GLOM_ENABLE_CLIENT_ONLY
glom_LDADD += layout_item_dialogs/liblayoutitemdialogs.a \
              mode_design/libmode_design.a \
              mode_design/fields/libmode_design_fields.a \
              mode_design/print_layouts/libmode_design_print_layouts.a \
              mode_design/users/libmode_design_users.a \
              mode_design/script_library/libmode_design_script_library.a
endif

glom_LDADD += navigation/libnavigation.a \
              utility_widgets/libutility_widgets.a \
              utility_widgets/adddel/libutility_widgets_adddel.a \
              utility_widgets/adddel/eggcolumnchooser/libutility_widgets_adddel_eggcolumnchooserdialog.a \
              utility_widgets/db_adddel/libutility_widgets_db_adddel.a \
              utility_widgets/canvas/libutility_widgets_canvas.a \
              utility_widgets/libutility_widgets.a

if !GLOM_ENABLE_CLIENT_ONLY
glom_LDADD += translation/libtranslation.a \
              relationships_overview/librelationshipsoverview.a \
              utility_widgets/canvas/libutility_widgets_canvas.a
endif

glom_LDADD += python_embed/libpython_embed.a \
              $(top_builddir)/glom/libglom/libglom.la \
              $(top_builddir)/glom/mode_data/libmode_data.a \
              $(GLOM_LIBS) $(PYTHON_LDFLAGS)

if !WIN32
glom_LDADD += -lutil
endif

glom_LDADD += -lgettextpo

             #python_embed/python_module/glom.la

#             python_embed/boost/config/libpython_embed_boost_config.a 
#             python_embed/boost/python/libpython_embed_boost_python.a 
#             python_embed/boost/python/detail/libpython_embed_boost_python_detail.a 
#             python_embed/boost/mpl/libpython_embed_boost_mpl.a 
#             python_embed/boost/mpl/aux_/libpython_embed_boost_mpl_aux.a 

#libutil seems to be a fairly standard GNU library, and python needs it for openpty() and forkpty().


test_pyembed_SOURCES = test_pyembed.cc
test_pyembed_LDADD = $(GLOM_LIBS) $(PYTHON_LDFLAGS)

if !WIN32
test_pyembed_LDADD += -lutil
endif


#Install the .dtd file:
glomdatadir = $(pkgdatadir)
glomdata_DATA = glom_document.dtd

# Install the glade file:
# The glade prefix of glade_DATA seems to cause it to be installed in <appname>/glade/
# Because that's how gladedir is defined.
glade_DATA = glom.glade glom_developer.glade

EXTRA_DIST = glom_document.dtd $(glade_DATA)


