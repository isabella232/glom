AM_CPPFLAGS = -I@top_srcdir@/ -I@top_srcdir@/glom $(GLOM_CFLAGS) $(GLOM_WARNING_FLAGS) $(PYTHON_INCLUDES)

SUBDIRS =

#This seems to stop libtool from complaining that glom.la is not named libglom.a
common_ldflags = -module -avoid-version -no-undefined

# Build and install the python module:
pyexec_LTLIBRARIES = glom.la
glom_la_LDFLAGS = $(common_ldflags) -export-symbols-regex initglom

glom_la_LIBADD = $(top_builddir)/glom/libglom/libglom-1.0.la \
                 $(GLOM_LIBS) $(PYTHON_LDFLAGS)

if !WIN32
glom_la_LIBADD += -lutil
endif

glom_la_SOURCES = py_glom_module.cc py_glom_module.h

if WIN32
# Python .pyd modules are simply DLLs, but they have to be called .pyd for
# python to find them, and libtool only creates .dll.
install-exec-hook:
	mv $(pyexecdir)/glom.dll $(pyexecdir)/glom.pyd
endif
