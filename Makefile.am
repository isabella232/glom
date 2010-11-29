## Copyright (c) 2009, 2010  Openismus GmbH  <http://www.openismus.com/>
##
## This file is part of Glom.
##
## Glom is free software: you can redistribute it and/or modify it
## under the terms of the GNU General Public License as published
## by the Free Software Foundation, either version 2 of the License,
## or (at your option) any later version.
##
## Glom is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
## See the GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program.  If not, see <http://www.gnu.org/licenses/>.

## This project does not use recursive make, but a single toplevel Makefile
## to build the entire tree (excluding po and docs/user-guide as gettext and
## gnome-doc-utils come with their own build systems).  Read Peter Miller's
## excellent paper to learn why recursive make invocations are both slow and
## error-prone:  http://miller.emu.id.au/pmiller/books/rmch/

# Put compiled object files into the same sub-directory as their sources
AUTOMAKE_OPTIONS = subdir-objects

# Use .cc as default extension if no _SOURCES have been defined
AM_DEFAULT_SOURCE_EXT = .cc

# Point to our macro directory and pick up user flags from the environment
ACLOCAL_AMFLAGS  = -I macros ${ACLOCAL_FLAGS}

# Make warnings always fatal at distcheck time, and disable execution
# of tools that change files not installed by the package itself
DISTCHECK_CONFIGURE_FLAGS = --enable-warnings=fatal \
--disable-scrollkeeper --disable-update-mime-database

if ENABLE_DOCUMENTATION
doc_subdirs = docs/libglom_reference docs/pyglom_reference
else
doc_subdirs =
endif
if HAVE_GNOME_DOC_UTILS
gnome_doc_subdirs = docs/user-guide
else
gnome_doc_subdirs =
endif
SUBDIRS = po $(doc_subdirs) $(gnome_doc_subdirs)

if HOST_WIN32
win_resfile = glom/glom.res
else
win_resfile =
endif
SUFFIXES = .rc .res

bin_PROGRAMS = glom/glom
lib_LTLIBRARIES = glom/libglom/libglom-@GLOM_ABI_VERSION@.la
pyexec_LTLIBRARIES = glom/python_embed/python_module/glom_@GLOM_ABI_VERSION_UNDERLINED@.la

include Makefile_libglom.am
include Makefile_glom.am
include Makefile_tests.am

glom_python_embed_python_module_glom_@GLOM_ABI_VERSION_UNDERLINED@_la_SOURCES = \
	glom/python_embed/python_module/py_glom_module.cc	\
	glom/python_embed/python_module/py_glom_module.h

glom_python_embed_python_module_glom_@GLOM_ABI_VERSION_UNDERLINED@_la_LIBADD = \
	glom/libglom/libglom-$(GLOM_ABI_VERSION).la \
	$(PYTHON_LIBS) $(BOOST_PYTHON_LIBS)

if HOST_WIN32
pymod_ldflags = -module -avoid-version -no-undefined -shrext .pyd
else
pymod_ldflags = -module -avoid-version -no-undefined
endif
glom_pyexport = -export-symbols-regex '^_*initglom'
glom_python_embed_python_module_glom_@GLOM_ABI_VERSION_UNDERLINED@_la_LDFLAGS = $(pymod_ldflags) $(glom_pyexport)

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = glom/libglom/glom-@GLOM_ABI_VERSION@.pc

dist_pkgdata_DATA = glom/glom_document.dtd

gladedir = $(pkgdatadir)/glade
dist_glade_DATA = glom/glom.glade glom/glom_developer.glade

xsltdir = $(pkgdatadir)/xslt
dist_xslt_DATA = xslt/print_details_to_html.xsl xslt/print_report_to_html.xsl

exampledir = $(docdir)/examples
dist_example_DATA =				\
	examples/README				\
	examples/example_film_manager.glom	\
	examples/example_lesson_planner.glom	\
	examples/example_music_collection.glom	\
	examples/example_project_manager.glom	\
	examples/example_smallbusiness.glom

# We use += because we set this first in Makefile_tests.am:
dist_noinst_DATA += examples/sqlite/test_sqlite_music/test_sqlite_music.glom

iconthemedir = $(datadir)/icons/hicolor
appicon16dir = $(iconthemedir)/16x16/apps
appicon22dir = $(iconthemedir)/22x22/apps
appicon24dir = $(iconthemedir)/24x24/apps
appicon32dir = $(iconthemedir)/32x32/apps
appicon48dir = $(iconthemedir)/48x48/apps
appiconscdir = $(iconthemedir)/scalable/apps

dist_appicon16_DATA = icons/16x16/glom.png
dist_appicon22_DATA = icons/22x22/glom.png
dist_appicon24_DATA = icons/24x24/glom.png
dist_appicon32_DATA = icons/32x32/glom.png
dist_appicon48_DATA = icons/48x48/glom.png
dist_appiconsc_DATA = icons/scalable/glom.svg

noinst_icons = icons/16x16/glom.svg icons/22x22/glom.svg icons/32x32/glom.svg icons/win32/glom.ico

pixmapdir = $(pkgdatadir)/pixmaps
dist_pixmap_DATA =				\
	icons/16x16/glom-button.png		\
	icons/16x16/glom-field.png		\
	icons/16x16/glom-group.png		\
	icons/16x16/glom-image.png		\
	icons/16x16/glom-line-horizontal.png	\
	icons/16x16/glom-line-vertical.png	\
	icons/16x16/glom-notebook.png		\
	icons/16x16/glom-related-records.png	\
	icons/16x16/glom-text.png

if GLOM_ENABLE_MAEMO
desktopdir = $(OSSO_DESKTOPDIR)
dbusservicedir = $(OSSO_SERVICEDIR)
glom.service: Makefile
	$(AM_V_at)rm -f $@ $@.tmp
	$(AM_V_at)srcdir=''; \
	       test -f ./$@.in || srcdir=$(srcdir)/; \
	       sed -e 's|@bindir[@]|$(bindir)|g' $${srcdir}$@.in >$@.tmp
	$(AM_V_GEN)mv $@.tmp $@

glom.service: $(srcdir)/glom.service.in
dbusservice_DATA = glom.service
else
desktopdir = $(datadir)/applications
endif

if HOST_WIN32
desktop_DATA =
else
desktop_DATA = glom.desktop
endif

mimedir = $(datadir)/mime
mimepackagesdir = $(mimedir)/packages
dist_mimepackages_DATA = glom.xml

dist_template = glom/glom.rc glom.service.in
dist_docutils = gnome-doc-utils.make omf.make xmldocs.make
dist_intltool = intltool-extract.in intltool-merge.in intltool-update.in
dist_docfiles = docs/overview.txt docs/developer_documentation.txt \
		docs/postgres_test.c docs/postgres_gda_test.c win32/README

dist_noinst_DATA += $(dist_template) $(noinst_icons) $(dist_docutils) \
		   $(dist_intltool) $(dist_docfiles)

# We use += here because we already used = in Makefile_tests.am:
dist_noinst_SCRIPTS += autogen.sh win32/build-installer

CLEANFILES = $(dbusservice_DATA) $(desktop_DATA) $(win_resfile)
DISTCLEANFILES = intltool-extract intltool-merge intltool-update
MAINTAINERCLEANFILES = \
	$(dist_docutils) $(dist_intltool) \
	macros/gnome-doc-utils.m4 macros/intltool.m4 \
	macros/libtool.m4 macros/ltoptions.m4 macros/ltsugar.m4 \
	macros/ltversion.m4 macros/lt~obsolete.m4

# The location of the gettext catalogs as defined by intltool.
glom_localedir = $(prefix)/$(DATADIRNAME)/locale

glom_includes = -I$(top_builddir)/glom -I$(top_srcdir)/glom -I$(top_builddir) -I$(top_srcdir)

#Let gimpruler build without changing the paths in its source code:
glom_includes += -I$(top_srcdir)/glom/utility_widgets/gimpruler

# We use += because this is set first in Makefile_tests.am:
glom_defines += \
-DGLOM_DATADIR=\""$(datadir)"\" \
-DGLOM_DOCDIR_EXAMPLES=\""$(docdir)/examples"\" \
-DGLOM_DOCDIR_EXAMPLES_NOTINSTALLED=\""$(abs_top_srcdir)/examples"\" \
-DGLOM_LOCALEDIR=\""$(glom_localedir)"\" \
-DGLOM_PKGDATADIR=\""$(pkgdatadir)"\" \
-DGLOM_PKGDATADIR_NOTINSTALLED=\""$(top_srcdir)/glom"\"


AM_CPPFLAGS = $(glom_includes) $(GLOM_CFLAGS) $(PYTHON_CPPFLAGS) $(BOOST_PYTHON_CFLAGS) $(glom_defines)
AM_CFLAGS   = $(GLOM_WFLAGS)
AM_CXXFLAGS = $(GLOM_WXXFLAGS)

update_mime_database = update-mime-database
update_icon_cache = gtk-update-icon-cache --ignore-theme-index --force

if UPDATE_MIME_DATABASE
install-data-hook: install-update-icon-cache install-update-mime-database
uninstall-hook: uninstall-update-icon-cache uninstall-update-mime-database
else
install-data-hook: install-update-icon-cache
uninstall-hook: uninstall-update-icon-cache
endif

install-update-icon-cache:
	@$(POST_INSTALL)
	test -n "$(DESTDIR)" || $(update_icon_cache) "$(iconthemedir)"

uninstall-update-icon-cache:
	@$(POST_UNINSTALL)
	test -n "$(DESTDIR)" || $(update_icon_cache) "$(iconthemedir)"

install-update-mime-database:
	@$(POST_INSTALL)
	test -n "$(DESTDIR)" || $(update_mime_database) "$(mimedir)"

uninstall-update-mime-database:
	@$(POST_UNINSTALL)
	test -n "$(DESTDIR)" || $(update_mime_database) "$(mimedir)"

if HOST_WIN32
.rc.res:
	$(AM_V_GEN)$(WINDRES) -i '$<' --input-format=rc -o '$@' -O coff
endif

@INTLTOOL_DESKTOP_RULE@

.PHONY: install-update-icon-cache uninstall-update-icon-cache \
	install-update-mime-database uninstall-update-mime-database

# Instruct GNU make to delete the targets of a rule after it failed, in
# order to avoid the complication of handling that situation manually.
.DELETE_ON_ERROR:
