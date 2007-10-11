#INCLUDES = -I@top_srcdir@/ $(GLOM_CFLAGS)

#SUBDIRS = 

gladedir = $(datadir)/glom/glade/

# We define GLOM_GLADEDIR so that we can get the installed .glade file at runtime.
INCLUDES = -I@top_srcdir@/ $(GLOM_CFLAGS) $(PYTHON_INCLUDES) \
           -DGLOM_GLADEDIR=\""$(gladedir)/"\" \
           -DDATADIR=\""$(datadir)"\"

noinst_LIBRARIES = libutility_widgets_canvas.a
libutility_widgets_canvas_a_SOURCES = \
                               canvas_editable.cc canvas_editable.h \
                               canvas_item_movable.cc canvas_item_movable.h \
                               canvas_line_movable.cc canvas_line_movable.h \
                               canvas_rect_movable.cc canvas_rect_movable.h \
                               canvas_group_resizable.cc canvas_group_resizable.h \
                               canvas_group_grid.cc canvas_group_grid.h \
                               canvas_group_movable.cc canvs_group_movable.h \
                               canvas_text_movable.cc canvs_text_movable.h


#libutility_widgets_canvas_a_LIBADD = 


noinst_PROGRAMS =test_canvas_editable

test_canvas_editable_SOURCES = test_canvas_editable.cc $(libutility_widgets_canvas_a_SOURCES)
test_canvas_editable_LDADD =  $(GLOM_LIBS)

