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
## yelp-tools (TODO: Still?) come with their own build systems).  Read Peter Miller's
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
DISTCHECK_CONFIGURE_FLAGS = --enable-warnings=fatal --disable-update-mime-database

if ENABLE_DOCUMENTATION
doc_subdirs = docs/libglom_reference docs/pyglom_reference
else
doc_subdirs =
endif

if ENABLE_DOCUMENTATION
yelp_doc_subdirs = docs/user-guide
else
yelp_doc_subdirs = docs/user-guide
endif

SUBDIRS = po $(doc_subdirs) $(yelp_doc_subdirs)

if HOST_WIN32
win_resfile = glom/glom.res
else
win_resfile =
endif
SUFFIXES = .rc .res

glom_includes = -I$(top_builddir)/glom -I$(top_srcdir)/glom -I$(top_builddir) -I$(top_srcdir)

include Makefile_libglom.am

glom_defines = \
-DGLOM_LOCALEDIR=\""$(glom_localedir)"\"

# We use these instead of individual variables,
# to avoid repetition, becase we really want to use 
# the same flags for everything:
AM_CFLAGS   = $(GLOM_WFLAGS)
AM_CXXFLAGS = $(GLOM_WXXFLAGS)

include Makefile_glom.am

include Makefile_tests.am

#We must do this so we can use += later:
MAINTAINERCLEANFILES =

# gcov
if GCOV_ENABLED
gcov: gcov-clean all check
	$(AM_V_GEN)(mkdir -p tests/coverage; \
	coverage_dir='tests/coverage'; \
	$(LCOV) --directory . --base-directory . --capture --output-file $$coverage_dir/coverage.info; \
	$(GENHTML) --legend --output-directory $$coverage_dir $$coverage_dir/coverage.info; \
	$(LCOV) --directory . --zerocounters; \
	echo "Code coverage report: file://$(abs_top_builddir)/$$coverage_dir/index.html")

clean: gcov-clean
gcov-clean:
	$(LCOV) --directory . --zerocounters

gcov_phony_files = gcov gcov-clean gcov-report.txt
else
gcov:
	echo "Need to reconfigure with --enable-gcov"
endif



dist_pkgdata_DATA = glom/glom_document.dtd


EXTRA_DIST += \
	data/ui/operator/box_navigation_tables.glade \
	data/ui/operator/dialog_choose_date.glade \
	data/ui/operator/dialog_connection.glade \
	data/ui/operator/dialog_data_invalid_format.glade \
	data/ui/operator/dialog_existing_or_new.glade \
	data/ui/operator/dialog_find_id.glade \
	data/ui/operator/dialog_image_load_progress.glade \
	data/ui/operator/dialog_image_save_progress.glade \
	data/ui/operator/dialog_import_csv.glade \
	data/ui/operator/dialog_import_csv_progress.glade \
	data/ui/operator/dialog_new_record.glade \
	data/ui/operator/window_main.glade
		
EXTRA_DIST += \
	data/ui/developer/dialog_add_related_table.glade \
	data/ui/developer/dialog_change_language.glade \
	data/ui/developer/dialog_choose_field.glade \
	data/ui/developer/dialog_choose_relationship.glade \
	data/ui/developer/dialog_choose_user.glade \
	data/ui/developer/dialog_database_preferences.glade \
	data/ui/developer/dialog_fieldslist.glade \
	data/ui/developer/dialog_field_summary.glade \
	data/ui/developer/dialog_flowtable.glade \
	data/ui/developer/dialog_group_by.glade \
	data/ui/developer/dialog_initial_password.glade \
	data/ui/developer/dialog_line.glade \
	data/ui/developer/dialog_layout_field_properties.glade \
	data/ui/developer/dialog_new_group.glade \
	data/ui/developer/dialog_new_library_script.glade \
	data/ui/developer/dialog_notebook.glade \
	data/ui/developer/dialog_script_library.glade \
	data/ui/developer/dialog_sort_fields.glade \
	data/ui/developer/dialog_translation_copy.glade \
	data/ui/developer/dialog_translation_identify_original.glade \
	data/ui/developer/dialog_user.glade \
	data/ui/developer/window_button_script.glade \
	data/ui/developer/window_data_layout_export.glade \
	data/ui/developer/window_data_layout.glade \
	data/ui/developer/window_default_formatting.glade \
	data/ui/developer/window_design.glade \
	data/ui/developer/window_field_calculation.glade \
	data/ui/developer/window_field_definition_edit.glade \
	data/ui/developer/box_formatting.glade \
	data/ui/developer/window_groups.glade \
	data/ui/developer/window_imageobject.glade \
	data/ui/developer/window_print_layout_edit.glade \
	data/ui/developer/window_relationships_overview.glade \
	data/ui/developer/box_print_layouts.glade \
	data/ui/developer/window_report_layout.glade \
	data/ui/developer/box_reports.glade \
	data/ui/developer/window_text_format.glade \
	data/ui/developer/window_textobject.glade \
	data/ui/developer/window_translations.glade \
	data/ui/developer/window_users.glade

example_files = \
	examples/example_film_manager.glom \
	examples/example_lesson_planner.glom \
	examples/example_music_collection.glom \
	examples/example_project_manager.glom \
	examples/example_smallbusiness.glom
dist_noinst_DATA += \
	$(example_files) \
	examples/README

# Get the base filenames only, without the extension: 
example_files_names ?= $(patsubst %.glom,%,$(notdir $(example_files)))

# Create .po files for all locales in all example files:
examples_export_po: $(top_builddir)/glom/glom_export_po_all $(top_builddir)/glom/glom_export_po
	for example_file in $(example_files_names); do \
	  $(top_builddir)/glom/glom_export_po_all $(top_srcdir)/examples/$$example_file.glom --output-path=$(top_builddir)/examples/po_files/$$example_file/; \
	  $(top_builddir)/glom/glom_export_po $(top_srcdir)/examples/$$example_file.glom --output-path=$(top_builddir)/examples/po_files/$$example_file/$$example_file.pot --template; \
	done
	

# Import all .po files into all example files:
examples_import_po: $(top_builddir)/glom/glom_import_po_all
	for example_file in $(example_files_names); do \
	  $(top_builddir)/glom/glom_import_po_all $(top_srcdir)/examples/$$example_file.glom --input-path=$(top_builddir)/examples/po_files/$$example_file/; \
	done

# We use += because we set this first in Makefile_tests.am:
dist_noinst_DATA += \
	examples/sqlite/test_sqlite_music/test_sqlite_music.glom \
	examples/sqlite/test_sqlite_music/glom_musiccollection21.db

iconthemedir = $(datadir)/icons/hicolor
appicon16dir = $(iconthemedir)/16x16/apps
appicon22dir = $(iconthemedir)/22x22/apps
appicon24dir = $(iconthemedir)/24x24/apps
appicon32dir = $(iconthemedir)/32x32/apps
appicon48dir = $(iconthemedir)/48x48/apps
appiconscdir = $(iconthemedir)/scalable/apps

dist_appicon16_DATA = data/icons/16x16/glom.png
dist_appicon22_DATA = data/icons/22x22/glom.png
dist_appicon24_DATA = data/icons/24x24/glom.png
dist_appicon32_DATA = data/icons/32x32/glom.png
dist_appicon48_DATA = data/icons/48x48/glom.png
dist_appiconsc_DATA = data/icons/scalable/glom.svg

noinst_icons = \
	data/icons/16x16/glom.svg \
	data/icons/22x22/glom.svg \
	data/icons/32x32/glom.svg \
	data/icons/win32/glom.ico

# These icons are used for toolpalette buttons, and an about dialog:
icons_for_resources = \
	data/icons/glom-button.png \
	data/icons/glom-field.png \
	data/icons/glom-group.png \
	data/icons/glom-image.png \
	data/icons/glom-line-horizontal.png \
	data/icons/glom-line-vertical.png \
	data/icons/glom-notebook.png \
	data/icons/glom-related-records.png \
	data/icons/glom-text.png \
	data/icons/48x48/glom.png

EXTRA_DIST += $(icons_for_resources)


desktopdir = $(datadir)/applications

if HOST_WIN32
desktop_DATA =
else
desktop_DATA = glom.desktop
endif


# The AppData file:
appdata_in_files = glom.appdata.xml.in
appdata_files = $(appdata_in_files:.xml.in=.xml)
appdatadir = $(datadir)/appdata
dist_appdata_DATA = $(appdata_files)

EXTRA_DIST += $(appdata_in_files)

@INTLTOOL_XML_RULE@


#Validate the AppData File:
#TODO: Run this during make check when appdata-tools is widespread.
validate-appdata: $(appdata_files)
	appdata-validate --strict $(appdata_files)


mimedir = $(datadir)/mime
mimepackagesdir = $(mimedir)/packages
dist_mimepackages_DATA = glom.xml

dist_template = glom/glom.rc glom.service.in
dist_docutils =
dist_intltool = intltool-extract.in intltool-merge.in intltool-update.in
dist_docfiles = docs/overview.txt docs/developer_documentation.txt \
		docs/postgres_test.c docs/postgres_gda_test.c win32/README

dist_noinst_DATA += $(dist_template) $(noinst_icons) $(dist_docutils) \
		   $(dist_intltool) $(dist_docfiles)

# We use += here because we already used = in Makefile_tests.am:
dist_noinst_SCRIPTS += autogen.sh win32/build-installer

# messages.mo is apparently generated because we have .po files under examples/po_files, in git,
# TODO: Is there some way to tell the build to ignore them?
CLEANFILES = $(dbusservice_DATA) $(desktop_DATA) $(appdata_DATA) $(win_resfile) $(top_builddir)/messages.mo
DISTCLEANFILES = intltool-extract intltool-merge intltool-update
MAINTAINERCLEANFILES += \
	$(dist_docutils) $(dist_intltool) \
	macros/intltool.m4 \
	macros/libtool.m4 macros/ltoptions.m4 macros/ltsugar.m4 \
	macros/ltversion.m4 macros/lt~obsolete.m4

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
	install-update-mime-database uninstall-update-mime-database \
        $(gcov_phony_files)

# Optional: auto-generate the ChangeLog file from the git log on make dist
# dist-changelog.am is added by mm-common-prepare in autogen.sh.
include $(top_srcdir)/dist-changelog.am


# Instruct GNU make to delete the targets of a rule after it failed, in
# order to avoid the complication of handling that situation manually.
.DELETE_ON_ERROR:
