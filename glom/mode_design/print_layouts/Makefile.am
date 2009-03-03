SUBDIRS = 

gladedir = $(datadir)/glom/glade/
glom_icon_dir = $(datadir)/glom/pixmaps

# We define GLOM_GLADEDIR so that we can get the installed .glade file at runtime.
# We define PREFIX, SYSCONFDIR, LIBDIR, and DATADIR for the GNOME_PROGRAM_STANDARD_PROPERTIES macro.
AM_CPPFLAGS = -I@top_srcdir@/ -I@top_srcdir@/glom $(GLOM_CFLAGS) $(PYTHON_INCLUDES) \
           -DGLOM_GLADEDIR=\""$(gladedir)/"\" \
           -DGLOM_ICON_DIR=\""$(glom_icon_dir)"\"

noinst_LIBRARIES = libmode_design_print_layouts.a
libmode_design_print_layouts_a_SOURCES = box_print_layouts.h box_print_layouts.cc \
                                         window_print_layout_edit.h window_print_layout_edit.cc \
                                         canvas_layout_item.h canvas_layout_item.cc \
                                         canvas_print_layout.h canvas_print_layout.cc \
                                         dialog_text_formatting.h dialog_text_formatting.cc \
                                         print_layout_toolbar.h print_layout_toolbar.cc \
                                         print_layout_toolbar_button.h print_layout_toolbar_button.cc

