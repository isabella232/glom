INCLUDES = -I@top_srcdir@/ $(GLOM_CFLAGS)

SUBDIRS = adddel db_adddel

noinst_LIBRARIES = libutility_widgets.a
libutility_widgets_a_SOURCES = alignment_justified.cc alignment_justified.h \
                               dialog_properties.cc dialog_properties.h \
                               flowtable.cc flowtable.h \
                               flowtablewithfields.cc flowtablewithfields.h \
                               entry_numerical.cc entry_numerical.h \
                               datawidget.h datawidget.cc \
                               entryglom.h entryglom.cc \
                               placeholder.cc placeholder.h \
                               table_columns.cc table_columns.h \
                               layoutwidgetbase.cc layoutwidgetbase.h

libutility_widgets_a_LIBADD = adddel/libutility_widgets_adddel.a db_adddel/libutility_widgets_db_adddel.a


noinst_PROGRAMS = test

test_SOURCES = test_flowtable.cc flowtable.cc flowtable.h

test_LDADD =  $(GLOM_LIBS)

