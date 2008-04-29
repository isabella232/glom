AM_CPPFLAGS = -I@top_srcdir@/ $(GLOM_CFLAGS) $(PYTHON_INCLUDES) \
           $(MAEMO_LAUNCHER_CFLAGS) \
           -DGLOM_GLADEDIR=\""$(gladedir)/"\" \
           -DLOCALEDIR=\""$(datadir)/locale"\" \
           -DPREFIX=\""$(prefix)"\" \
           -DSYSCONFDIR=\""$(sysconfdir)"\" \
           -DLIBDIR=\""$(libdir)"\" \
           -DDATADIR=\""$(datadir)"\" \
           -DGLOM_XSLTDIR=\""$(glomxsltdir)/"\" \
           -DGLOM_EXAMPLES_DIR=\""$(glom_examples_dir)"\" \
           -DGLOM_ICON_DIR=\""$(glom_icon_dir)"\"

noinst_PROGRAMS = test_parsing_time test_signal_reemit

test_parsing_time_SOURCES = test_parsing_time.cc
test_parsing_time_LDADD = $(GLOM_LIBS) $(top_builddir)/glom/libglom/libglom.la

test_signal_reemit_SOURCES = test_signal_reemit.cc
test_signal_reemit_LDADD = $(GLOM_LIBS) 

