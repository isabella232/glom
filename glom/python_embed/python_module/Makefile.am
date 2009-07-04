AM_CPPFLAGS = -I@top_srcdir@/ -I@top_srcdir@/glom $(GLOM_CFLAGS) $(GLOM_WARNING_FLAGS) $(PYTHON_INCLUDES) \
           -DGLOM_ABI_VERSION=\""$(GLOM_ABI_VERSION)\"" \
           -DGLOM_ABI_VERSION_UNDERLINED=\""$(GLOM_ABI_VERSION_UNDERLINED)\""

SUBDIRS =

#This seems to stop libtool from complaining that glom.la is not named libglom.a
common_ldflags = -module -avoid-version -no-undefined

# Build and install the python module:
# Note that we use underlines because import fails with incorrect syntax 
# when trying to use . or - in the module name. murrayc.
pyexec_LTLIBRARIES = glom_1_12.la
glom_1_12_la_LDFLAGS = $(common_ldflags) -export-symbols-regex initglom

glom_1_12_la_LIBADD = $(top_builddir)/glom/libglom/libglom-1.12.la \
                 $(GLOM_LIBS) $(PYTHON_LDFLAGS)

if !WIN32
glom_1_12_la_LIBADD += -lutil
endif

glom_1_12_la_SOURCES = py_glom_module.cc py_glom_module.h

if WIN32
# Python .pyd modules are simply DLLs, but they have to be called .pyd for
# python to find them, and libtool only creates .dll.
install-exec-hook:
	mv $(pyexecdir)/glom.dll $(pyexecdir)/glom_1_12.pyd
endif
