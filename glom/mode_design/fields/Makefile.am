gladedir = $(datadir)/glom/glade/

# We define GLOM_GLADEDIR so that we can get the installed .glade file at runtime.
INCLUDES = -I@top_srcdir@/ $(GLOM_CFLAGS) $(PYTHON_INCLUDES) \
                    -DGLOM_GLADEDIR=\""$(gladedir)/"\"
           
           
noinst_LIBRARIES = libmode_design_fields.a
libmode_design_fields_a_SOURCES = box_db_table_definition.cc box_db_table_definition.h \
                                                         dialog_fielddefinition.h dialog_fielddefinition.cc \
                                                         dialog_fieldcalculation.h dialog_fieldcalculation.cc \
                                                         combo_fieldtype.h combo_fieldtype.cc

# bin_PROGRAMS = testcombo

# testcombo_SOURCES = combo_test.cc combo_fieldtype.cc combo_fieldtype.h

# testcombo_LDADD =  $(GLOM_LIBS) 
