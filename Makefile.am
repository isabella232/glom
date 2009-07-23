## Copyright (c) 2009  Daniel Elstner <daniel.kitta@gmail.com>
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
## error-prone:
## http://members.pcug.org.au/~millerp/rmch/recu-make-cons-harm.html

AUTOMAKE_OPTIONS = subdir-objects
ACLOCAL_AMFLAGS  = -I macros ${ACLOCAL_FLAGS}
DISTCHECK_CONFIGURE_FLAGS = --enable-warnings=fatal \
--disable-scrollkeeper --disable-update-mime-database

if HAVE_GNOME_DOC_UTILS
doc_subdirs = docs/user-guide
else
doc_subdirs =
endif
SUBDIRS = po $(doc_subdirs)

if HOST_WIN32
win_resfile = glom/glom.res
else
win_resfile =
endif
SUFFIXES = .rc .res

bin_PROGRAMS = glom/glom
lib_LTLIBRARIES = glom/libglom/libglom-1.12.la
pyexec_LTLIBRARIES = glom/python_embed/python_module/glom_1_12.la

noinst_PROGRAMS =						\
	glom/libglom/test_connectionpool			\
	glom/libglom/test_document				\
	glom/libglom/test_sharedptr_layoutitem			\
	glom/utility_widgets/test_flowtable			\
	glom/utility_widgets/egg/toolpalette/testtoolpalette	\
	glom/test_pyembed					\
	regression_tests/test_parsing_time			\
	regression_tests/test_signal_reemit

noinst_LIBRARIES =							\
	glom/utility_widgets/egg/toolpalette/libeggtoolpalette.a	\
	glom/utility_widgets/egg/util/libeggutil.a

libglom_includedir = $(includedir)/glom-$(GLOM_ABI_VERSION)/libglom
libglom_include_HEADERS =			\
	glom/libglom/appstate.h			\
	glom/libglom/init.h			\
	glom/libglom/libglom_config.h		\
	glom/libglom/sharedptr.h		\
	glom/libglom/standard_table_prefs_fields.h

libglom_data_structure_includedir = $(libglom_includedir)/data_structure
libglom_data_structure_include_HEADERS =			\
	glom/libglom/data_structure/field.h			\
	glom/libglom/data_structure/fieldtypes.h		\
	glom/libglom/data_structure/foundset.h			\
	glom/libglom/data_structure/glomconversions.h		\
	glom/libglom/data_structure/groupinfo.h			\
	glom/libglom/data_structure/iso_codes.h			\
	glom/libglom/data_structure/numeric_format.h		\
	glom/libglom/data_structure/parameternamegenerator.h	\
	glom/libglom/data_structure/print_layout.h		\
	glom/libglom/data_structure/privileges.h		\
	glom/libglom/data_structure/relationship.h		\
	glom/libglom/data_structure/report.h			\
	glom/libglom/data_structure/system_prefs.h		\
	glom/libglom/data_structure/tableinfo.h			\
	glom/libglom/data_structure/translatable_item.h

libglom_ds_layout_includedir = $(libglom_data_structure_includedir)/layout
libglom_ds_layout_include_HEADERS =					\
	glom/libglom/data_structure/layout/custom_title.h		\
	glom/libglom/data_structure/layout/fieldformatting.h		\
	glom/libglom/data_structure/layout/layoutgroup.h		\
	glom/libglom/data_structure/layout/layoutitem.h			\
	glom/libglom/data_structure/layout/layoutitem_button.h		\
	glom/libglom/data_structure/layout/layoutitem_calendarportal.h	\
	glom/libglom/data_structure/layout/layoutitem_field.h		\
	glom/libglom/data_structure/layout/layoutitem_image.h		\
	glom/libglom/data_structure/layout/layoutitem_line.h		\
	glom/libglom/data_structure/layout/layoutitem_notebook.h	\
	glom/libglom/data_structure/layout/layoutitem_placeholder.h	\
	glom/libglom/data_structure/layout/layoutitem_portal.h		\
	glom/libglom/data_structure/layout/layoutitem_text.h		\
	glom/libglom/data_structure/layout/usesrelationship.h

libglom_ds_l_report_parts_includedir = $(libglom_ds_layout_includedir)/report_parts
libglom_ds_l_report_parts_include_HEADERS =						\
	glom/libglom/data_structure/layout/report_parts/layoutitem_fieldsummary.h	\
	glom/libglom/data_structure/layout/report_parts/layoutitem_footer.h		\
	glom/libglom/data_structure/layout/report_parts/layoutitem_groupby.h		\
	glom/libglom/data_structure/layout/report_parts/layoutitem_header.h		\
	glom/libglom/data_structure/layout/report_parts/layoutitem_summary.h		\
	glom/libglom/data_structure/layout/report_parts/layoutitem_verticalgroup.h

libglom_document_includedir = $(libglom_includedir)/document
libglom_document_include_HEADERS =		\
	glom/libglom/document/document.h	\
	glom/libglom/document/view.h

libglom_d_bakery_includedir = $(libglom_document_includedir)/bakery
libglom_d_bakery_include_HEADERS =			\
	glom/libglom/document/bakery/document.h		\
	glom/libglom/document/bakery/document_xml.h

libglom_d_b_view_includedir = $(libglom_d_bakery_includedir)/view
libglom_d_b_view_include_HEADERS =				\
	glom/libglom/document/bakery/view/view.h		\
	glom/libglom/document/bakery/view/view_composite.h	\
	glom/libglom/document/bakery/view/viewbase.h

glom_libglom_libglom_1_12_la_SOURCES =					\
	glom/libglom/appstate.cc					\
	glom/libglom/calcinprogress.cc					\
	glom/libglom/calcinprogress.h					\
	glom/libglom/connectionpool.cc					\
	glom/libglom/connectionpool.h					\
	glom/libglom/gst-package.c					\
	glom/libglom/gst-package.h					\
	glom/libglom/init.cc						\
	glom/libglom/spawn_with_feedback.cc				\
	glom/libglom/spawn_with_feedback.h				\
	glom/libglom/utils.cc						\
	glom/libglom/utils.h						\
	glom/libglom/connectionpool_backends/backend.cc			\
	glom/libglom/connectionpool_backends/backend.h			\
	glom/libglom/data_structure/field.cc				\
	glom/libglom/data_structure/fieldtypes.cc			\
	glom/libglom/data_structure/foundset.cc				\
	glom/libglom/data_structure/glomconversions.cc			\
	glom/libglom/data_structure/groupinfo.cc			\
	glom/libglom/data_structure/iso_codes.cc			\
	glom/libglom/data_structure/numeric_format.cc			\
	glom/libglom/data_structure/parameternamegenerator.cc		\
	glom/libglom/data_structure/print_layout.cc			\
	glom/libglom/data_structure/privileges.cc			\
	glom/libglom/data_structure/relationship.cc			\
	glom/libglom/data_structure/report.cc				\
	glom/libglom/data_structure/system_prefs.cc			\
	glom/libglom/data_structure/tableinfo.cc			\
	glom/libglom/data_structure/translatable_item.cc		\
	glom/libglom/data_structure/layout/custom_title.cc		\
	glom/libglom/data_structure/layout/fieldformatting.cc		\
	glom/libglom/data_structure/layout/layoutgroup.cc		\
	glom/libglom/data_structure/layout/layoutitem.cc		\
	glom/libglom/data_structure/layout/layoutitem_button.cc		\
	glom/libglom/data_structure/layout/layoutitem_calendarportal.cc	\
	glom/libglom/data_structure/layout/layoutitem_field.cc		\
	glom/libglom/data_structure/layout/layoutitem_image.cc		\
	glom/libglom/data_structure/layout/layoutitem_line.cc		\
	glom/libglom/data_structure/layout/layoutitem_notebook.cc	\
	glom/libglom/data_structure/layout/layoutitem_placeholder.cc	\
	glom/libglom/data_structure/layout/layoutitem_portal.cc		\
	glom/libglom/data_structure/layout/layoutitem_text.cc		\
	glom/libglom/data_structure/layout/usesrelationship.cc		\
	glom/libglom/data_structure/layout/report_parts/layoutitem_fieldsummary.cc	\
	glom/libglom/data_structure/layout/report_parts/layoutitem_footer.cc		\
	glom/libglom/data_structure/layout/report_parts/layoutitem_groupby.cc		\
	glom/libglom/data_structure/layout/report_parts/layoutitem_header.cc		\
	glom/libglom/data_structure/layout/report_parts/layoutitem_summary.cc		\
	glom/libglom/data_structure/layout/report_parts/layoutitem_verticalgroup.cc	\
	glom/libglom/document/document.cc				\
	glom/libglom/document/bakery/document.cc			\
	glom/libglom/document/bakery/document_xml.cc			\
	glom/libglom/document/bakery/view/view.cc			\
	glom/libglom/document/bakery/view/view_composite.cc		\
	glom/libglom/document/bakery/view/viewbase.cc			\
	glom/libglom/python_embed/py_glom_record.cc			\
	glom/libglom/python_embed/py_glom_record.h			\
	glom/libglom/python_embed/py_glom_related.cc			\
	glom/libglom/python_embed/py_glom_related.h			\
	glom/libglom/python_embed/py_glom_relatedrecord.cc		\
	glom/libglom/python_embed/py_glom_relatedrecord.h		\
	glom/libglom/python_embed/pygdavalue_conversions.c		\
	glom/libglom/python_embed/pygdavalue_conversions.h

if GLOM_ENABLE_POSTGRESQL
glom_libglom_libglom_1_12_la_SOURCES +=				\
	glom/libglom/connectionpool_backends/postgres.cc	\
	glom/libglom/connectionpool_backends/postgres.h		\
	glom/libglom/connectionpool_backends/postgres_central.cc\
	glom/libglom/connectionpool_backends/postgres_central.h
if !GLOM_ENABLE_CLIENT_ONLY
glom_libglom_libglom_1_12_la_SOURCES +=				\
	glom/libglom/connectionpool_backends/postgres_self.cc	\
	glom/libglom/connectionpool_backends/postgres_self.h
endif
endif
if GLOM_ENABLE_SQLITE
glom_libglom_libglom_1_12_la_SOURCES +=				\
	glom/libglom/connectionpool_backends/sqlite.cc		\
	glom/libglom/connectionpool_backends/sqlite.h
endif

glom_libglom_libglom_1_12_la_LIBADD = $(LIBGLOM_LIBS) $(PYTHON_LIBS)

if GLOM_ENABLE_MAEMO
glom_libglom_libglom_1_12_la_LDFLAGS = -no-undefined -static
else
glom_libglom_libglom_1_12_la_LDFLAGS = -no-undefined
endif

glom_python_embed_python_module_glom_1_12_la_SOURCES =		\
	glom/python_embed/python_module/py_glom_module.cc	\
	glom/python_embed/python_module/py_glom_module.h

glom_python_embed_python_module_glom_1_12_la_LIBADD =	\
	glom/libglom/libglom-$(GLOM_ABI_VERSION).la	\
	$(LIBGLOM_LIBS) $(PYTHON_LIBS)

# TODO: Verify that -shrext really does the trick on Windows, or if the
# install-exec-hook rule needs to be resurrected to rename the .dll file.
pymod_ldflags = -module -avoid-version -no-undefined -shrext .pyd
glom_pyexport = -export-symbols-regex '^_*initglom'
glom_python_embed_python_module_glom_1_12_la_LDFLAGS = $(pymod_ldflags) $(glom_pyexport)

eggutildir = glom/utility_widgets/egg/util
eggmarshalers_list = $(top_srcdir)/$(eggutildir)/eggmarshalers.list

libegg_includes = -I$(top_builddir)/$(eggutildir) -I$(top_srcdir)/$(eggutildir)
libegg_cppflags = $(libegg_includes) $(EGG_CFLAGS) -DEGG_COMPILATION

glom_utility_widgets_egg_toolpalette_libeggtoolpalette_a_SOURCES =	\
	glom/utility_widgets/egg/toolpalette/eggenumaction.c		\
	glom/utility_widgets/egg/toolpalette/eggenumaction.h		\
	glom/utility_widgets/egg/toolpalette/eggtoolitemgroup.c		\
	glom/utility_widgets/egg/toolpalette/eggtoolitemgroup.h		\
	glom/utility_widgets/egg/toolpalette/eggtoolpalette.c		\
	glom/utility_widgets/egg/toolpalette/eggtoolpalette.h		\
	glom/utility_widgets/egg/toolpalette/eggtoolpaletteprivate.h

glom_utility_widgets_egg_toolpalette_libeggtoolpalette_a_CPPFLAGS = $(libegg_cppflags)
glom_utility_widgets_egg_toolpalette_libeggtoolpalette_a_LIBADD = \
	glom/utility_widgets/egg/util/libeggutil.a

glom_utility_widgets_egg_util_libeggutil_a_SOURCES =	\
	glom/utility_widgets/egg/util/egg-marshal.c	\
	glom/utility_widgets/egg/util/eggintl.h		\
	glom/utility_widgets/egg/util/egg-macros.h

EXTRA_glom_utility_widgets_egg_util_libeggutil_a_SOURCES =	\
	glom/utility_widgets/egg/util/eggmarshalers.c		\
	glom/utility_widgets/egg/util/eggmarshalers.h

glom_utility_widgets_egg_util_libeggutil_a_CPPFLAGS = $(libegg_cppflags)

glom_glom_SOURCES =							\
	glom/application.cc						\
	glom/application.h						\
	glom/base_db.cc							\
	glom/base_db.h							\
	glom/base_db_table.cc						\
	glom/base_db_table.h						\
	glom/base_db_table_data.cc					\
	glom/base_db_table_data.h					\
	glom/box_db_table.cc						\
	glom/box_db_table.h						\
	glom/box_reports.cc						\
	glom/box_reports.h						\
	glom/box_withbuttons.cc						\
	glom/box_withbuttons.h						\
	glom/combobox_fields.cc						\
	glom/combobox_fields.h						\
	glom/combobox_relationship.cc					\
	glom/combobox_relationship.h					\
	glom/dialog_connection.cc					\
	glom/dialog_connection.h					\
	glom/dialog_existing_or_new.cc					\
	glom/dialog_existing_or_new.h					\
	glom/dialog_glom.cc						\
	glom/dialog_glom.h						\
	glom/dialog_import_csv.cc					\
	glom/dialog_import_csv.h					\
	glom/dialog_import_csv_progress.cc				\
	glom/dialog_import_csv_progress.h				\
	glom/dialog_invalid_data.cc					\
	glom/dialog_invalid_data.h					\
	glom/dialog_progress_creating.cc				\
	glom/dialog_progress_creating.h					\
	glom/filechooser_export.cc					\
	glom/filechooser_export.h					\
	glom/frame_glom.cc						\
	glom/frame_glom.h						\
	glom/glade_utils.cc						\
	glom/glade_utils.h						\
	glom/glom_postgres.cc						\
	glom/glom_postgres.h						\
	glom/glom_privs.cc						\
	glom/glom_privs.h						\
	glom/main.cc							\
	glom/notebook_glom.cc						\
	glom/notebook_glom.h						\
	glom/signal_reemitter.h						\
	glom/utils_ui.cc						\
	glom/utils_ui.h							\
	glom/variablesmap.cc						\
	glom/variablesmap.h						\
	glom/xsl_utils.cc						\
	glom/xsl_utils.h						\
	glom/bakery/app.cc						\
	glom/bakery/app.h						\
	glom/bakery/app_withdoc.cc					\
	glom/bakery/app_withdoc.h					\
	glom/bakery/app_withdoc_gtk.cc					\
	glom/bakery/app_withdoc_gtk.h					\
	glom/bakery/busy_cursor.cc					\
	glom/bakery/busy_cursor.h					\
	glom/bakery/dialog_offersave.cc					\
	glom/bakery/dialog_offersave.h					\
	glom/mode_data/box_data.cc					\
	glom/mode_data/box_data.h					\
	glom/mode_data/box_data_calendar_related.cc			\
	glom/mode_data/box_data_calendar_related.h			\
	glom/mode_data/box_data_details.cc				\
	glom/mode_data/box_data_details.h				\
	glom/mode_data/box_data_list.cc					\
	glom/mode_data/box_data_list.h					\
	glom/mode_data/box_data_list_related.cc				\
	glom/mode_data/box_data_list_related.h				\
	glom/mode_data/box_data_manyrecords.cc				\
	glom/mode_data/box_data_manyrecords.h				\
	glom/mode_data/box_data_portal.cc				\
	glom/mode_data/box_data_portal.h				\
	glom/mode_data/dialog_choose_field.cc				\
	glom/mode_data/dialog_choose_field.h				\
	glom/mode_data/dialog_choose_relationship.cc			\
	glom/mode_data/dialog_choose_relationship.h			\
	glom/mode_data/dialog_layout.cc					\
	glom/mode_data/dialog_layout.h					\
	glom/mode_data/dialog_layout_export.cc				\
	glom/mode_data/dialog_layout_export.h				\
	glom/mode_data/flowtablewithfields.cc				\
	glom/mode_data/flowtablewithfields.h				\
	glom/mode_data/notebook_data.cc					\
	glom/mode_data/notebook_data.h					\
	glom/mode_find/box_data_details_find.cc				\
	glom/mode_find/box_data_details_find.h				\
	glom/mode_find/box_data_list_find.cc				\
	glom/mode_find/box_data_list_find.h				\
	glom/mode_find/notebook_find.cc					\
	glom/mode_find/notebook_find.h					\
	glom/navigation/box_tables.cc					\
	glom/navigation/box_tables.h					\
	glom/python_embed/glom_python.cc				\
	glom/python_embed/glom_python.h					\
	glom/reports/report_builder.cc					\
	glom/reports/report_builder.h					\
	glom/utility_widgets/alignment_justified.cc			\
	glom/utility_widgets/alignment_justified.h			\
	glom/utility_widgets/buttonglom.cc				\
	glom/utility_widgets/buttonglom.h				\
	glom/utility_widgets/checkglom.cc				\
	glom/utility_widgets/checkglom.h				\
	glom/utility_widgets/combo_textglade.cc				\
	glom/utility_widgets/combo_textglade.h				\
	glom/utility_widgets/comboentry_currency.cc			\
	glom/utility_widgets/comboentry_currency.h			\
	glom/utility_widgets/comboentryglom.cc				\
	glom/utility_widgets/comboentryglom.h				\
	glom/utility_widgets/comboglom.cc				\
	glom/utility_widgets/comboglom.h				\
	glom/utility_widgets/comboglomchoicesbase.cc			\
	glom/utility_widgets/comboglomchoicesbase.h			\
	glom/utility_widgets/datawidget.cc				\
	glom/utility_widgets/datawidget.h				\
	glom/utility_widgets/dialog_choose_date.cc			\
	glom/utility_widgets/dialog_choose_date.h			\
	glom/utility_widgets/dialog_choose_id.cc			\
	glom/utility_widgets/dialog_choose_id.h				\
	glom/utility_widgets/dialog_flowtable.cc			\
	glom/utility_widgets/dialog_flowtable.h				\
	glom/utility_widgets/dialog_image_progress.cc			\
	glom/utility_widgets/dialog_image_progress.h			\
	glom/utility_widgets/dialog_layoutitem_properties.cc		\
	glom/utility_widgets/dialog_layoutitem_properties.h		\
	glom/utility_widgets/dialog_properties.cc			\
	glom/utility_widgets/dialog_properties.h			\
	glom/utility_widgets/entryglom.cc				\
	glom/utility_widgets/entryglom.h				\
	glom/utility_widgets/flowtable.cc				\
	glom/utility_widgets/flowtable.h				\
	glom/utility_widgets/imageglom.cc				\
	glom/utility_widgets/imageglom.h				\
	glom/utility_widgets/labelglom.cc				\
	glom/utility_widgets/labelglom.h				\
	glom/utility_widgets/layoutwidgetbase.cc			\
	glom/utility_widgets/layoutwidgetbase.h				\
	glom/utility_widgets/layoutwidgetfield.cc			\
	glom/utility_widgets/layoutwidgetfield.h			\
	glom/utility_widgets/layoutwidgetmenu.cc			\
	glom/utility_widgets/layoutwidgetmenu.h				\
	glom/utility_widgets/layoutwidgetutils.cc			\
	glom/utility_widgets/layoutwidgetutils.h			\
	glom/utility_widgets/notebookglom.cc				\
	glom/utility_widgets/notebookglom.h				\
	glom/utility_widgets/placeholder-glom.cc			\
	glom/utility_widgets/placeholder-glom.h				\
	glom/utility_widgets/placeholder.cc				\
	glom/utility_widgets/placeholder.h				\
	glom/utility_widgets/table_columns.cc				\
	glom/utility_widgets/table_columns.h				\
	glom/utility_widgets/textviewglom.cc				\
	glom/utility_widgets/textviewglom.h				\
	glom/utility_widgets/adddel/adddel.cc				\
	glom/utility_widgets/adddel/adddel.h				\
	glom/utility_widgets/adddel/adddel_withbuttons.cc		\
	glom/utility_widgets/adddel/adddel_withbuttons.h		\
	glom/utility_widgets/adddel/treeviewcolumn_glom.cc		\
	glom/utility_widgets/adddel/treeviewcolumn_glom.h		\
	glom/utility_widgets/canvas/canvas_editable.cc			\
	glom/utility_widgets/canvas/canvas_editable.h			\
	glom/utility_widgets/canvas/canvas_group_grid.cc		\
	glom/utility_widgets/canvas/canvas_group_grid.h			\
	glom/utility_widgets/canvas/canvas_group_movable.cc		\
	glom/utility_widgets/canvas/canvas_group_movable.h		\
	glom/utility_widgets/canvas/canvas_group_resizable.cc		\
	glom/utility_widgets/canvas/canvas_group_resizable.h		\
	glom/utility_widgets/canvas/canvas_image_movable.cc		\
	glom/utility_widgets/canvas/canvas_image_movable.h		\
	glom/utility_widgets/canvas/canvas_item_movable.cc		\
	glom/utility_widgets/canvas/canvas_item_movable.h		\
	glom/utility_widgets/canvas/canvas_line_movable.cc		\
	glom/utility_widgets/canvas/canvas_line_movable.h		\
	glom/utility_widgets/canvas/canvas_rect_movable.cc		\
	glom/utility_widgets/canvas/canvas_rect_movable.h		\
	glom/utility_widgets/canvas/canvas_table_movable.cc		\
	glom/utility_widgets/canvas/canvas_table_movable.h		\
	glom/utility_widgets/canvas/canvas_text_movable.cc		\
	glom/utility_widgets/canvas/canvas_text_movable.h		\
	glom/utility_widgets/cellrendererlist/cellrendererlist.cc	\
	glom/utility_widgets/cellrendererlist/cellrendererlist.h	\
	glom/utility_widgets/db_adddel/cellrenderer_buttonimage.cc	\
	glom/utility_widgets/db_adddel/cellrenderer_buttonimage.h	\
	glom/utility_widgets/db_adddel/cellrenderer_buttontext.cc	\
	glom/utility_widgets/db_adddel/cellrenderer_buttontext.h	\
	glom/utility_widgets/db_adddel/db_adddel.cc			\
	glom/utility_widgets/db_adddel/db_adddel.h			\
	glom/utility_widgets/db_adddel/db_adddel_withbuttons.cc		\
	glom/utility_widgets/db_adddel/db_adddel_withbuttons.h		\
	glom/utility_widgets/db_adddel/db_treeviewcolumn_glom.cc	\
	glom/utility_widgets/db_adddel/db_treeviewcolumn_glom.h		\
	glom/utility_widgets/db_adddel/glom_db_treemodel.cc		\
	glom/utility_widgets/db_adddel/glom_db_treemodel.h

if !GLOM_ENABLE_CLIENT_ONLY
glom_glom_SOURCES +=							\
	glom/dialog_database_preferences.cc				\
	glom/dialog_database_preferences.h				\
	glom/dialog_new_self_hosted_connection.cc			\
	glom/dialog_new_self_hosted_connection.h			\
	glom/printoperation_printlayout.cc				\
	glom/printoperation_printlayout.h				\
	glom/layout_item_dialogs/box_formatting.cc			\
	glom/layout_item_dialogs/box_formatting.h			\
	glom/layout_item_dialogs/combo_summarytype.cc			\
	glom/layout_item_dialogs/combo_summarytype.h			\
	glom/layout_item_dialogs/comboentry_borderwidth.cc		\
	glom/layout_item_dialogs/comboentry_borderwidth.h		\
	glom/layout_item_dialogs/dialog_buttonscript.cc			\
	glom/layout_item_dialogs/dialog_buttonscript.h			\
	glom/layout_item_dialogs/dialog_field_layout.cc			\
	glom/layout_item_dialogs/dialog_field_layout.h			\
	glom/layout_item_dialogs/dialog_field_summary.cc		\
	glom/layout_item_dialogs/dialog_field_summary.h			\
	glom/layout_item_dialogs/dialog_group_by.cc			\
	glom/layout_item_dialogs/dialog_group_by.h			\
	glom/layout_item_dialogs/dialog_groupby_secondaryfields.cc	\
	glom/layout_item_dialogs/dialog_groupby_secondaryfields.h	\
	glom/layout_item_dialogs/dialog_groupby_sortfields.cc		\
	glom/layout_item_dialogs/dialog_groupby_sortfields.h		\
	glom/layout_item_dialogs/dialog_imageobject.cc			\
	glom/layout_item_dialogs/dialog_imageobject.h			\
	glom/layout_item_dialogs/dialog_notebook.cc			\
	glom/layout_item_dialogs/dialog_notebook.h			\
	glom/layout_item_dialogs/dialog_textobject.cc			\
	glom/layout_item_dialogs/dialog_textobject.h			\
	glom/mode_data/dialog_layout_calendar_related.cc		\
	glom/mode_data/dialog_layout_calendar_related.h			\
	glom/mode_data/dialog_layout_details.cc				\
	glom/mode_data/dialog_layout_details.h				\
	glom/mode_data/dialog_layout_list.cc				\
	glom/mode_data/dialog_layout_list.h				\
	glom/mode_data/dialog_layout_list_related.cc			\
	glom/mode_data/dialog_layout_list_related.h			\
	glom/mode_data/treestore_layout.cc				\
	glom/mode_data/treestore_layout.h				\
	glom/mode_design/box_db_table_relationships.cc			\
	glom/mode_design/box_db_table_relationships.h			\
	glom/mode_design/dialog_add_related_table.cc			\
	glom/mode_design/dialog_add_related_table.h			\
	glom/mode_design/dialog_design.cc				\
	glom/mode_design/dialog_design.h				\
	glom/mode_design/dialog_fields.cc				\
	glom/mode_design/dialog_fields.h				\
	glom/mode_design/dialog_relationships.cc			\
	glom/mode_design/dialog_relationships.h				\
	glom/mode_design/fields/box_db_table_definition.cc		\
	glom/mode_design/fields/box_db_table_definition.h		\
	glom/mode_design/fields/combo_fieldtype.cc			\
	glom/mode_design/fields/combo_fieldtype.h			\
	glom/mode_design/fields/dialog_fieldcalculation.cc		\
	glom/mode_design/fields/dialog_fieldcalculation.h		\
	glom/mode_design/fields/dialog_fielddefinition.cc		\
	glom/mode_design/fields/dialog_fielddefinition.h		\
	glom/mode_design/print_layouts/box_print_layouts.cc		\
	glom/mode_design/print_layouts/box_print_layouts.h		\
	glom/mode_design/print_layouts/canvas_layout_item.cc		\
	glom/mode_design/print_layouts/canvas_layout_item.h		\
	glom/mode_design/print_layouts/canvas_print_layout.cc		\
	glom/mode_design/print_layouts/canvas_print_layout.h		\
	glom/mode_design/print_layouts/dialog_text_formatting.cc	\
	glom/mode_design/print_layouts/dialog_text_formatting.h		\
	glom/mode_design/print_layouts/print_layout_toolbar.cc		\
	glom/mode_design/print_layouts/print_layout_toolbar.h		\
	glom/mode_design/print_layouts/print_layout_toolbar_button.cc	\
	glom/mode_design/print_layouts/print_layout_toolbar_button.h	\
	glom/mode_design/print_layouts/window_print_layout_edit.cc	\
	glom/mode_design/print_layouts/window_print_layout_edit.h	\
	glom/mode_design/script_library/dialog_new_script.cc		\
	glom/mode_design/script_library/dialog_new_script.h		\
	glom/mode_design/script_library/dialog_script_library.cc	\
	glom/mode_design/script_library/dialog_script_library.h		\
	glom/mode_design/users/dialog_choose_user.cc			\
	glom/mode_design/users/dialog_choose_user.h			\
	glom/mode_design/users/dialog_groups_list.cc			\
	glom/mode_design/users/dialog_groups_list.h			\
	glom/mode_design/users/dialog_new_group.cc			\
	glom/mode_design/users/dialog_new_group.h			\
	glom/mode_design/users/dialog_user.cc				\
	glom/mode_design/users/dialog_user.h				\
	glom/mode_design/users/dialog_users_list.cc			\
	glom/mode_design/users/dialog_users_list.h			\
	glom/relationships_overview/canvas_group_dbtable.cc		\
	glom/relationships_overview/canvas_group_dbtable.h		\
	glom/relationships_overview/dialog_relationships_overview.cc	\
	glom/relationships_overview/dialog_relationships_overview.h	\
	glom/relationships_overview/printoperation_relationshipsoverview.cc	\
	glom/relationships_overview/printoperation_relationshipsoverview.h	\
	glom/reports/dialog_layout_report.cc				\
	glom/reports/dialog_layout_report.h				\
	glom/reports/treestore_report_layout.cc				\
	glom/reports/treestore_report_layout.h				\
	glom/translation/combobox_locale.cc				\
	glom/translation/combobox_locale.h				\
	glom/translation/dialog_change_language.cc			\
	glom/translation/dialog_change_language.h			\
	glom/translation/dialog_copy_translation.cc			\
	glom/translation/dialog_copy_translation.h			\
	glom/translation/dialog_identify_original.cc			\
	glom/translation/dialog_identify_original.h			\
	glom/translation/window_translations.cc				\
	glom/translation/window_translations.h				\
	glom/utility_widgets/filechooserdialog_saveextras.cc		\
	glom/utility_widgets/filechooserdialog_saveextras.h		\
	glom/utility_widgets/flowtable_dnd.cc				\
	glom/utility_widgets/flowtable_dnd.h				\
	glom/utility_widgets/layouttoolbar.cc				\
	glom/utility_widgets/layouttoolbar.h				\
	glom/utility_widgets/layouttoolbarbutton.cc			\
	glom/utility_widgets/layouttoolbarbutton.h			\
	glom/utility_widgets/notebooklabelglom.cc			\
	glom/utility_widgets/notebooklabelglom.h			\
	glom/utility_widgets/sidebar.cc					\
	glom/utility_widgets/sidebar.h
endif

glom_glom_LDADD = $(win_resfile) \
	glom/utility_widgets/egg/toolpalette/libeggtoolpalette.a \
	glom/utility_widgets/egg/util/libeggutil.a \
	glom/libglom/libglom-$(GLOM_ABI_VERSION).la \
	$(GLOM_LIBS) $(PYTHON_LIBS) $(INTLLIBS) -lgettextpo

if HOST_WIN32
# Suppress console window
glom_glom_LDFLAGS = -mwindows
else
glom_glom_LDFLAGS =
endif

tests_ldadd = glom/libglom/libglom-$(GLOM_ABI_VERSION).la $(LIBGLOM_LIBS)

glom_libglom_test_connectionpool_SOURCES = glom/libglom/test_connectionpool.cc
glom_libglom_test_connectionpool_LDADD = $(tests_ldadd)

glom_libglom_test_document_SOURCES = glom/libglom/test_document.cc
glom_libglom_test_document_LDADD = $(tests_ldadd)

glom_libglom_test_sharedptr_layoutitem_SOURCES = glom/libglom/test_sharedptr_layoutitem.cc
glom_libglom_test_sharedptr_layoutitem_LDADD = $(tests_ldadd)

glom_utility_widgets_test_flowtable_SOURCES =	\
	glom/utility_widgets/flowtable.cc	\
	glom/utility_widgets/flowtable.h	\
	glom/utility_widgets/test_flowtable.cc

glom_utility_widgets_test_flowtable_LDADD = $(GLOM_LIBS)

glom_utility_widgets_egg_toolpalette_testtoolpalette_SOURCES =	\
	glom/utility_widgets/egg/toolpalette/testtoolpalette.c

glom_utility_widgets_egg_toolpalette_testtoolpalette_LDADD =	\
	glom/utility_widgets/egg/toolpalette/libeggtoolpalette.a\
	glom/utility_widgets/egg/util/libeggutil.a		\
	$(EGG_LIBS)

glom_test_pyembed_SOURCES = glom/test_pyembed.cc
glom_test_pyembed_LDADD = $(LIBGLOM_LIBS) $(PYTHON_LIBS)

regression_tests_test_parsing_time_SOURCES = regression_tests/test_parsing_time.cc
regression_tests_test_parsing_time_LDADD = $(tests_ldadd)

regression_tests_test_signal_reemit_SOURCES = regression_tests/test_signal_reemit.cc
regression_tests_test_signal_reemit_LDADD = $(LIBGLOM_LIBS)

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = glom/libglom/glom-1.0.pc

dist_pkgdata_DATA = glom/glom_document.dtd

gladedir = $(pkgdatadir)/glade
dist_glade_DATA = glom/glom.glade glom/glom_developer.glade

xsltdir = $(pkgdatadir)/xslt
dist_xslt_DATA = xslt/print_details_to_html.xsl xslt/print_report_to_html.xsl

# Note: this used to be $(pkgdatadir)/doc/examples
exampledir = $(docdir)/examples
dist_example_DATA =				\
	examples/README				\
	examples/example_film_manager.glom	\
	examples/example_lesson_planner.glom	\
	examples/example_music_collection.glom	\
	examples/example_project_manager.glom	\
	examples/example_smallbusiness.glom

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

# TODO: What are the uninstalled SVGs for?
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

desktopdir = $(datadir)/applications
desktop_DATA = glom.desktop

mimedir = $(datadir)/mime
mimepackagesdir = $(mimedir)/packages
dist_mimepackages_DATA = glom.xml

dist_template = glom/glom.rc glom/utility_widgets/egg/util/eggmarshalers.list
dist_docutils = gnome-doc-utils.make omf.make xmldocs.make
dist_intltool = intltool-extract.in intltool-merge.in intltool-update.in
dist_docfiles = docs/overview.txt docs/developer_documentation.txt \
		docs/postgres_test.c docs/postgres_gda_test.c win32/README

dist_noinst_DATA = $(dist_template) $(noinst_icons) $(dist_docutils) \
		   $(dist_intltool) $(dist_docfiles)
dist_noinst_SCRIPTS = autogen.sh win32/build-installer

CLEANFILES = $(desktop_DATA) $(win_resfile)
DISTCLEANFILES = intltool-extract intltool-merge intltool-update
MAINTAINERCLEANFILES = $(dist_docutils) $(dist_intltool)

# The location of the gettext catalogs as defined by intltool.
glom_localedir = $(prefix)/$(DATADIRNAME)/locale

glom_includes = -I$(top_builddir)/glom -I$(top_srcdir)/glom -I$(top_builddir) -I$(top_srcdir)
glom_defines = \
-DGLOM_DATADIR=\""$(datadir)"\" \
-DGLOM_DOCDIR=\""$(docdir)"\" \
-DGLOM_LOCALEDIR=\""$(glom_localedir)"\" \
-DGLOM_PKGDATADIR=\""$(pkgdatadir)"\"

AM_CPPFLAGS = $(glom_includes) $(GLOM_CFLAGS) $(PYTHON_CPPFLAGS) $(glom_defines)
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

$(eggutildir)/egg-marshal.c: $(eggutildir)/eggmarshalers.c $(eggutildir)/eggmarshalers.h

$(eggutildir)/eggmarshalers.h: $(eggmarshalers_list)
	$(GLIB_GENMARSHAL) --prefix=_egg_marshal --header "$(eggmarshalers_list)" >'$@'

$(eggutildir)/eggmarshalers.c: $(eggmarshalers_list)
	$(GLIB_GENMARSHAL) --prefix=_egg_marshal --body "$(eggmarshalers_list)" >'$@'

if HOST_WIN32
.rc.res:
	$(WINDRES) -i '$<' --input-format=rc -o '$@' -O coff
endif

@INTLTOOL_DESKTOP_RULE@

.PHONY: install-update-icon-cache uninstall-update-icon-cache \
	install-update-mime-database uninstall-update-mime-database

.DELETE_ON_ERROR:
