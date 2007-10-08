#INCLUDES = -I@top_srcdir@/ $(GLOM_CFLAGS)

SUBDIRS = adddel db_adddel

gladedir = $(datadir)/glom/glade/

# We define GLOM_GLADEDIR so that we can get the installed .glade file at runtime.
INCLUDES = -I@top_srcdir@/ $(GLOM_CFLAGS) $(PYTHON_INCLUDES) \
           -DGLOM_GLADEDIR=\""$(gladedir)/"\" \
           -DDATADIR=\""$(datadir)"\"

noinst_LIBRARIES = libutility_widgets.a
libutility_widgets_a_SOURCES = alignment_justified.cc alignment_justified.h \
                               dialog_properties.cc dialog_properties.h \
                               flowtable.cc flowtable.h \
                               flowtablewithfields.cc flowtablewithfields.h \
                               datawidget.h datawidget.cc \
                               buttonglom.h buttonglom.cc \
                               notebookglom.h notebookglom.cc \
                               entryglom.h entryglom.cc \
                               comboglom.h comboglom.cc \
                               comboglomchoicesbase.h comboglomchoicesbase.cc \
                               comboentryglom.h comboentryglom.cc \
                               placeholder.cc placeholder.h \
                               table_columns.cc table_columns.h \
                               layoutwidgetbase.cc layoutwidgetbase.h \
                               layoutwidgetfield.cc layoutwidgetfield.h \
                               textviewglom.h textviewglom.cc \
                               combo_textglade.h combo_textglade.cc \
                               cellrendererlist.h cellrendererlist.cc \
                               comboentry_currency.h comboentry_currency.cc \
                               imageglom.h imageglom.cc \
                               labelglom.h labelglom.cc \
                               dialog_choose_id.cc dialog_choose_id.h \
                               dialog_choose_date.cc dialog_choose_date.h \
                               filechooserdialog.cc filechooserdialog.h \
                               canvas_editable.cc canvas_editable.h \
                               canvas_line_movable.cc canvas_line_movable.h \
                               canvas_rect_movable.cc canvas_rect_movable.h \
                               canvas_group_resizable.cc canvas_group_resizable.h

libutility_widgets_a_LIBADD = adddel/libutility_widgets_adddel.a \
                              db_adddel/libutility_widgets_db_adddel.a


noinst_PROGRAMS = test_flowtable test_canvas_editable

test_flowtable_SOURCES = test_flowtable.cc flowtable.cc flowtable.h
test_flowtable_LDADD =  $(GLOM_LIBS)

test_canvas_editable_SOURCES = test_canvas_editable.cc canvas_editable.cc canvas_editable.h canvas_group_resizable.cc canvas_group_resizable.h canvas_rect_movable.cc canvas_rect_movable.h canvas_line_movable.cc canvas_line_movable.h
test_canvas_editable_LDADD =  $(GLOM_LIBS)

