gladedir = $(datadir)/glom/glade/
glomxsltdir = $(datadir)/glom/xslt/

# We define GLOM_GLADEDIR so that we can get the installed .glade file at runtime.
INCLUDES = -I@top_srcdir@/ $(GLOM_CFLAGS) $(PYTHON_INCLUDES) \
                    -DGLOM_GLADEDIR=\""$(gladedir)/"\" \
                    -DGLOM_XSLTDIR=\""$(glomxsltdir)/"\"

noinst_LIBRARIES = liblayoutitemdialogs.a
liblayoutitemdialogs_a_SOURCES = \
                         dialog_field_layout.cc dialog_field_layout.h \
                         dialog_group_by.cc dialog_group_by.h \
                         dialog_field_summary.cc dialog_field_summary.h \
                         combo_summarytype.h combo_summarytype.cc \
                         dialog_groupby_secondaryfields.h dialog_groupby_secondaryfields.cc
