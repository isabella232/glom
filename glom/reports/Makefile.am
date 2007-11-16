gladedir = $(datadir)/glom/glade/
glomxsltdir = $(datadir)/glom/xslt/

# We define GLOM_GLADEDIR so that we can get the installed .glade file at runtime.
INCLUDES = -I@top_srcdir@/ $(GLOM_CFLAGS) $(PYTHON_INCLUDES) \
                    -DGLOM_GLADEDIR=\""$(gladedir)/"\" \
                    -DGLOM_XSLTDIR=\""$(glomxsltdir)/"\"

noinst_LIBRARIES = libreports.a
libreports_a_SOURCES = report_builder.cc report_builder.h

if !GLOM_ENABLE_CLIENT_ONLY
libreports_a_SOURCES += dialog_layout_report.cc dialog_layout_report.h \
                        treestore_report_layout.cc treestore_report_layout.h
endif
