gladedir = $(datadir)/glom/glade/

# We define GLOM_GLADEDIR so that we can get the installed .glade file at runtime.
INCLUDES = -I@top_srcdir@/ $(GLOM_CFLAGS) \
                    -DGLOM_GLADEDIR=\""$(gladedir)/"\"


noinst_LIBRARIES = libmode_data.a
libmode_data_a_SOURCES = box_data.cc box_data.h \
                         box_data_Details.cc box_data_Details.h \
                         Box_Data_List_Related.cc Box_Data_List_Related.h \
                         box_data_list.cc box_data_list.h \
                         notebook_data.cc notebook_data.h \
                         dialog_layout.cc dialog_layout.h
                         

