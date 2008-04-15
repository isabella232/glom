#AM_CPPFLAGS = -I@top_srcdir@/ $(GLOM_CFLAGS)

SUBDIRS = cellrendererlist adddel db_adddel canvas calendar util eggtoolpalette 

gladedir = $(datadir)/glom/glade/
glom_icon_dir = $(datadir)/glom/pixmaps

# We define GLOM_GLADEDIR so that we can get the installed .glade file at runtime.
AM_CPPFLAGS = -I@top_srcdir@/ $(GLOM_CFLAGS) $(PYTHON_INCLUDES) \
           -DGLOM_GLADEDIR=\""$(gladedir)/"\" \
           -DGLOM_ICON_DIR=\""$(glom_icon_dir)"\" \
           -DDATADIR=\""$(datadir)"\"

noinst_LIBRARIES = libutility_widgets.a
libutility_widgets_a_SOURCES = alignment_justified.cc alignment_justified.h \
                               dialog_properties.cc dialog_properties.h \
                               flowtable.cc flowtable.h \
                               datawidget.h datawidget.cc \
                               buttonglom.h buttonglom.cc \
                               notebookglom.h notebookglom.cc \
                               entryglom.h entryglom.cc \
                               comboglom.h comboglom.cc \
                               comboglomchoicesbase.h comboglomchoicesbase.cc \
                               comboentryglom.h comboentryglom.cc \
                               placeholder.cc placeholder.h \
                               placeholder-glom.h  placeholder-glom.cc \
                               table_columns.cc table_columns.h \
                               layoutwidgetbase.cc layoutwidgetbase.h \
                               layoutwidgetfield.cc layoutwidgetfield.h \
                               textviewglom.h textviewglom.cc \
                               combo_textglade.h combo_textglade.cc \
                               comboentry_currency.h comboentry_currency.cc \
                               imageglom.h imageglom.cc \
                               labelglom.h labelglom.cc \
                               dialog_choose_id.cc dialog_choose_id.h \
                               dialog_choose_date.cc dialog_choose_date.h \
                               checkglom.h checkglom.cc \
                               layoutwidgetutils.h layoutwidgetutils.cc \
                               dialog_layoutitem_properties.cc dialog_layoutitem_properties.h \
                               dialog_flowtable.cc  dialog_flowtable.h \
                               layoutwidgetmenu.cc  layoutwidgetmenu.h

if !GLOM_ENABLE_CLIENT_ONLY
libutility_widgets_a_SOURCES += filechooserdialog_saveextras.cc filechooserdialog_saveextras.h \
                                dragbutton.cc dragbutton.h \
                                dragbar.cc dragbar.h \
                                sidebar.h sidebar.cc \
                                flowtable_dnd.h  flowtable_dnd.cc \
                                notebooklabelglom.h notebooklabelglom.cc
                                
endif


libutility_widgets_a_LIBADD = cellrendererlist/libutility_widgets_cellrendererlist.a \
                              adddel/libutility_widgets_adddel.a \
                              db_adddel/libutility_widgets_db_adddel.a \
                              canvas/libutility_widgets_canvas.a

noinst_PROGRAMS = test_flowtable
test_flowtable_SOURCES = test_flowtable.cc flowtable.cc flowtable.h
test_flowtable_LDADD =  $(GLOM_LIBS)











