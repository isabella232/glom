INCLUDES = -I@top_srcdir@/ $(GLOM_CFLAGS)

SUBDIRS = fields

noinst_LIBRARIES = libmode_design.a
libmode_design_a_SOURCES = box_db_table_relationships.cc box_db_table_relationships.h \
                                                           dialog_fields.cc dialog_fields.h  \
                                                           dialog_design.cc dialog_design.h \
                                                           dialog_relationships.cc dialog_relationships.h
                                                           
libmode_design_a_LIBADD = fields/libmode_design_fields.a