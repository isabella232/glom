gladedir = $(datadir)/glom/glade/
glomxsltdir = $(datadir)/glom/xslt/

# We define GLOM_GLADEDIR so that we can get the installed .glade file at runtime.
AM_CPPFLAGS = -I@top_srcdir@/ -I@top_srcdir@/glom $(GLOM_CFLAGS) $(GLOM_WARNING_FLAGS) $(PYTHON_INCLUDES) \
                    -DGLOM_GLADEDIR=\""$(gladedir)/"\" \
                    -DGLOM_XSLTDIR=\""$(glomxsltdir)/"\"

noinst_LIBRARIES = libtranslation.a
libtranslation_a_SOURCES = window_translations.cc window_translations.h \
                           combobox_locale.h combobox_locale.cc \
                           dialog_change_language.h dialog_change_language.cc \
                           dialog_identify_original.h dialog_identify_original.cc \
                           dialog_copy_translation.h dialog_copy_translation.cc




