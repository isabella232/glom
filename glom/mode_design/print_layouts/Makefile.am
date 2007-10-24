SUBDIRS = 

gladedir = $(datadir)/glom/glade/

# We define GLOM_GLADEDIR so that we can get the installed .glade file at runtime.
# We define PREFIX, SYSCONFDIR, LIBDIR, and DATADIR for the GNOME_PROGRAM_STANDARD_PROPERTIES macro.
INCLUDES = -I@top_srcdir@/ $(GLOM_CFLAGS) $(PYTHON_INCLUDES) \
           -DGLOM_GLADEDIR=\""$(gladedir)/"\"

noinst_LIBRARIES = libmode_design_print_layouts.a
libmode_design_print_layouts_a_SOURCES = box_print_layouts.h box_print_layouts.cc \
                                         window_print_layout_edit.h window_print_layout_edit.cc \
                                         canvas_layout_item.h canvas_layout_item.cc \
                                         canvas_print_layout.h canvas_print_layout.cc \
                                         dialog_text_formatting.h dialog_text_formatting.cc
