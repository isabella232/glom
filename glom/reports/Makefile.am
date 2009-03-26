gladedir = $(datadir)/glom/glade/
glomxsltdir = $(datadir)/glom/xslt/

# We define GLOM_GLADEDIR so that we can get the installed .glade file at runtime.
AM_CPPFLAGS = -I@top_srcdir@/  -I@top_srcdir@/glom $(GLOM_CFLAGS) $(GLOM_WARNING_FLAGS) $(PYTHON_INCLUDES) \
                    -DGLOM_GLADEDIR=\""$(gladedir)/"\" \
                    -DGLOM_XSLTDIR=\""$(glomxsltdir)/"\"

noinst_LIBRARIES = libreports.a
libreports_a_SOURCES = report_builder.cc report_builder.h

if !GLOM_ENABLE_CLIENT_ONLY
libreports_a_SOURCES += dialog_layout_report.cc dialog_layout_report.h \
                        treestore_report_layout.cc treestore_report_layout.h
endif
