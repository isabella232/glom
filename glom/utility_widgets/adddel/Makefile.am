SUBDIRS = eggcolumnchooser

INCLUDES = -I@top_srcdir@/ $(GLOM_CFLAGS)

noinst_LIBRARIES = libutility_widgets_adddel.a
libutility_widgets_adddel_a_SOURCES = adddel.cc adddel.h adddel_withbuttons.cc adddel_withbuttons.h \
                                      cellrendererlist.h cellrendererlist.cc cellrendererpopup.h \
                                      cellrendererpopup.cc popupentry.h popupentry.cc \
                                      treeviewcolumn_glom.cc treeviewcolumn_glom.h
  
libutility_widgets_adddel_a_LIBADD = eggcolumnchooser/libutility_widgets_adddel_eggcolumnchooserdialog.a
