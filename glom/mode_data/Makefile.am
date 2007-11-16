gladedir = $(datadir)/glom/glade/
glomxsltdir = $(datadir)/glom/xslt/

# We define GLOM_GLADEDIR so that we can get the installed .glade file at runtime.
INCLUDES = -I@top_srcdir@/ $(GLOM_CFLAGS) $(PYTHON_INCLUDES) \
                    -DGLOM_GLADEDIR=\""$(gladedir)/"\" \
                    -DGLOM_XSLTDIR=\""$(glomxsltdir)/"\"

noinst_LIBRARIES = libmode_data.a
libmode_data_a_SOURCES = box_data.cc box_data.h \
                         box_data_details.cc box_data_details.h \
                         box_data_list_related.cc box_data_list_related.h \
                         box_data_list.cc box_data_list.h \
                         notebook_data.cc notebook_data.h \
                         dialog_layout.cc dialog_layout.h \
                         dialog_choose_field.cc dialog_choose_field.h \
                         dialog_choose_relationship.cc dialog_choose_relationship.h \
                         dialog_layout_export.cc dialog_layout_export.h

if !GLOM_ENABLE_CLIENT_ONLY
libmode_data_a_SOURCES += dialog_layout_list.cc dialog_layout_list.h \
                          dialog_layout_details.cc dialog_layout_details.h \
                          dialog_layout_list_related.cc dialog_layout_list_related.h \
                          treestore_layout.cc treestore_layout.h
endif
