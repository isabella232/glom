cmake_minimum_required(VERSION 3.2)
project("glom")

set(PACKAGE_VERSION "1.31.3")
set(GETTEXT_PACKAGE "glom")

set(CMAKE_CXX_STANDARD 14)

set(GLOM_ABI_VERSION "1.32")
# TODO: Find an equivalent for autoconf's AS_TR_SH()
set(GLOM_ABI_VERSION_UNDERLINED "1_32")

# TODO: GLOM_LOCALEDIR seems to be empty by default.
include(GNUInstallDirs)
set(GLOM_LOCALEDIR, ${CMAKE_INSTALL_LOCALEDIR})
set(PACKAGE_STRING "glom")

#Enable PostgreSQL features in the UI,
#but not MySQL or SQLite features.
set(GLOM_ENABLE_POSTGRESQL 1)


# Check that we have the glib-compile-resources utiltity executable:
find_program (glib_compile_resources_executable glib-compile-resources)
if (glib_compile_resources_executable)
    # TODO: Find a way to write if(), without else(), so we only mention failure.
    message (STATUS "glib-compile-resources executable found.")
else ()
    message (FATAL_ERROR "glib-compile-resources executable not found. This should be provided by glib.")
endif ()

set(GLADE_DATA_FILES
        data/ui/developer/box_formatting.glade
        data/ui/developer/box_print_layouts.glade
        data/ui/developer/box_reports.glade
        data/ui/developer/dialog_add_related_table.glade
        data/ui/developer/dialog_change_language.glade
        data/ui/developer/dialog_choose_field.glade
        data/ui/developer/dialog_choose_relationship.glade
        data/ui/developer/dialog_choose_user.glade
        data/ui/developer/dialog_database_preferences.glade
        data/ui/developer/dialog_field_summary.glade
        data/ui/developer/dialog_fieldslist.glade
        data/ui/developer/dialog_flowtable.glade
        data/ui/developer/dialog_group_by.glade
        data/ui/developer/dialog_initial_password.glade
        data/ui/developer/dialog_layout_field_properties.glade
        data/ui/developer/dialog_line.glade
        data/ui/developer/dialog_new_group.glade
        data/ui/developer/dialog_new_library_script.glade
        data/ui/developer/dialog_notebook.glade
        data/ui/developer/dialog_script_library.glade
        data/ui/developer/dialog_sort_fields.glade
        data/ui/developer/dialog_translation_copy.glade
        data/ui/developer/dialog_translation_identify_original.glade
        data/ui/developer/dialog_user.glade
        data/ui/developer/window_button_script.glade
        data/ui/developer/window_data_layout.glade
        data/ui/developer/window_data_layout_export.glade
        data/ui/developer/window_default_formatting.glade
        data/ui/developer/window_design.glade
        data/ui/developer/window_field_calculation.glade
        data/ui/developer/window_field_definition_edit.glade
        data/ui/developer/window_groups.glade
        data/ui/developer/window_imageobject.glade
        data/ui/developer/window_print_layout_edit.glade
        data/ui/developer/window_relationships_overview.glade
        data/ui/developer/window_report_layout.glade
        data/ui/developer/window_text_format.glade
        data/ui/developer/window_textobject.glade
        data/ui/developer/window_translations.glade
        data/ui/developer/window_users.glade
        data/ui/operator/box_navigation_tables.glade
        data/ui/operator/dialog_choose_date.glade
        data/ui/operator/dialog_connection.glade
        data/ui/operator/dialog_data_invalid_format.glade
        data/ui/operator/dialog_existing_or_new.glade
        data/ui/operator/dialog_find_id.glade
        data/ui/operator/dialog_image_load_progress.glade
        data/ui/operator/dialog_image_save_progress.glade
        data/ui/operator/dialog_import_csv.glade
        data/ui/operator/dialog_import_csv_progress.glade
        data/ui/operator/dialog_new_record.glade
        data/ui/operator/window_main.glade)

set (resource_dir ${CMAKE_CURRENT_SOURCE_DIR}/data)
set (resource_file ${resource_dir}/glom.gresource.xml)
set (resource_file_generated_source glom-resources.c)

add_custom_command(
        OUTPUT glom-resources.c
        MAIN_DEPENDENCY ${resource_file}
        DEPENDS ${GLADE_DATA_FILES}

        COMMAND ${glib_compile_resources_executable} --internal --target=${resource_file_generated_source} --sourcedir=${CMAKE_CURRENT_SOURCE_DIR} --generate-source ${resource_file}
)


set(SOURCE_FILES
        glom/bakery/appwindow.cc
        glom/bakery/appwindow.h
        glom/bakery/appwindow_withdoc.cc
        glom/bakery/appwindow_withdoc.h
        glom/bakery/busy_cursor.cc
        glom/bakery/busy_cursor.h
        glom/bakery/dialog_offersave.cc
        glom/bakery/dialog_offersave.h
        glom/import_csv/csv_parser.cc
        glom/import_csv/csv_parser.h
        glom/import_csv/dialog_import_csv.cc
        glom/import_csv/dialog_import_csv.h
        glom/import_csv/dialog_import_csv_progress.cc
        glom/import_csv/dialog_import_csv_progress.h
        glom/import_csv/file_encodings.cc
        glom/import_csv/file_encodings.h
        glom/libglom/connectionpool_backends/backend.cc
        glom/libglom/connectionpool_backends/backend.h
        glom/libglom/connectionpool_backends/mysql.cc
        glom/libglom/connectionpool_backends/mysql.h
        glom/libglom/connectionpool_backends/mysql_central.cc
        glom/libglom/connectionpool_backends/mysql_central.h
        glom/libglom/connectionpool_backends/mysql_self.cc
        glom/libglom/connectionpool_backends/mysql_self.h
        glom/libglom/connectionpool_backends/postgres.cc
        glom/libglom/connectionpool_backends/postgres.h
        glom/libglom/connectionpool_backends/postgres_central.cc
        glom/libglom/connectionpool_backends/postgres_central.h
        glom/libglom/connectionpool_backends/postgres_self.cc
        glom/libglom/connectionpool_backends/postgres_self.h
        glom/libglom/connectionpool_backends/sqlite.cc
        glom/libglom/connectionpool_backends/sqlite.h
        glom/libglom/data_structure/layout/report_parts/layoutitem_fieldsummary.cc
        glom/libglom/data_structure/layout/report_parts/layoutitem_fieldsummary.h
        glom/libglom/data_structure/layout/report_parts/layoutitem_footer.cc
        glom/libglom/data_structure/layout/report_parts/layoutitem_footer.h
        glom/libglom/data_structure/layout/report_parts/layoutitem_groupby.cc
        glom/libglom/data_structure/layout/report_parts/layoutitem_groupby.h
        glom/libglom/data_structure/layout/report_parts/layoutitem_header.cc
        glom/libglom/data_structure/layout/report_parts/layoutitem_header.h
        glom/libglom/data_structure/layout/report_parts/layoutitem_summary.cc
        glom/libglom/data_structure/layout/report_parts/layoutitem_summary.h
        glom/libglom/data_structure/layout/report_parts/layoutitem_verticalgroup.cc
        glom/libglom/data_structure/layout/report_parts/layoutitem_verticalgroup.h
        glom/libglom/data_structure/layout/custom_title.cc
        glom/libglom/data_structure/layout/custom_title.h
        glom/libglom/data_structure/layout/formatting.cc
        glom/libglom/data_structure/layout/formatting.h
        glom/libglom/data_structure/layout/layoutgroup.cc
        glom/libglom/data_structure/layout/layoutgroup.h
        glom/libglom/data_structure/layout/layoutitem.cc
        glom/libglom/data_structure/layout/layoutitem.h
        glom/libglom/data_structure/layout/layoutitem_button.cc
        glom/libglom/data_structure/layout/layoutitem_button.h
        glom/libglom/data_structure/layout/layoutitem_calendarportal.cc
        glom/libglom/data_structure/layout/layoutitem_calendarportal.h
        glom/libglom/data_structure/layout/layoutitem_field.cc
        glom/libglom/data_structure/layout/layoutitem_field.h
        glom/libglom/data_structure/layout/layoutitem_image.cc
        glom/libglom/data_structure/layout/layoutitem_image.h
        glom/libglom/data_structure/layout/layoutitem_line.cc
        glom/libglom/data_structure/layout/layoutitem_line.h
        glom/libglom/data_structure/layout/layoutitem_notebook.cc
        glom/libglom/data_structure/layout/layoutitem_notebook.h
        glom/libglom/data_structure/layout/layoutitem_placeholder.cc
        glom/libglom/data_structure/layout/layoutitem_placeholder.h
        glom/libglom/data_structure/layout/layoutitem_portal.cc
        glom/libglom/data_structure/layout/layoutitem_portal.h
        glom/libglom/data_structure/layout/layoutitem_text.cc
        glom/libglom/data_structure/layout/layoutitem_text.h
        glom/libglom/data_structure/layout/layoutitem_withformatting.cc
        glom/libglom/data_structure/layout/layoutitem_withformatting.h
        glom/libglom/data_structure/layout/static_text.cc
        glom/libglom/data_structure/layout/static_text.h
        glom/libglom/data_structure/layout/usesrelationship.cc
        glom/libglom/data_structure/layout/usesrelationship.h
        glom/libglom/data_structure/choicevalue.cc
        glom/libglom/data_structure/choicevalue.h
        glom/libglom/data_structure/database_title.cc
        glom/libglom/data_structure/database_title.h
        glom/libglom/data_structure/field.cc
        glom/libglom/data_structure/field.h
        glom/libglom/data_structure/fieldtypes.cc
        glom/libglom/data_structure/fieldtypes.h
        glom/libglom/data_structure/foundset.cc
        glom/libglom/data_structure/foundset.h
        glom/libglom/data_structure/glomconversions.cc
        glom/libglom/data_structure/glomconversions.h
        glom/libglom/data_structure/groupinfo.cc
        glom/libglom/data_structure/groupinfo.h
        glom/libglom/data_structure/has_title_singular.cc
        glom/libglom/data_structure/has_title_singular.h
        glom/libglom/data_structure/numeric_format.cc
        glom/libglom/data_structure/numeric_format.h
        glom/libglom/data_structure/print_layout.cc
        glom/libglom/data_structure/print_layout.h
        glom/libglom/data_structure/privileges.cc
        glom/libglom/data_structure/privileges.h
        glom/libglom/data_structure/relationship.cc
        glom/libglom/data_structure/relationship.h
        glom/libglom/data_structure/report.cc
        glom/libglom/data_structure/report.h
        glom/libglom/data_structure/system_prefs.cc
        glom/libglom/data_structure/system_prefs.h
        glom/libglom/data_structure/tableinfo.cc
        glom/libglom/data_structure/tableinfo.h
        glom/libglom/data_structure/translatable_item.cc
        glom/libglom/data_structure/translatable_item.h
        glom/libglom/document/bakery/view/view.h
        glom/libglom/document/bakery/view/view_composite.h
        glom/libglom/document/bakery/view/viewbase.cc
        glom/libglom/document/bakery/view/viewbase.h
        glom/libglom/document/bakery/document.cc
        glom/libglom/document/bakery/document.h
        glom/libglom/document/bakery/document_xml.cc
        glom/libglom/document/bakery/document_xml.h
        glom/libglom/document/document.cc
        glom/libglom/document/document.h
        glom/libglom/document/view.h
        glom/libglom/python_embed/py_glom_record.cc
        glom/libglom/python_embed/py_glom_record.h
        glom/libglom/python_embed/py_glom_related.cc
        glom/libglom/python_embed/py_glom_related.h
        glom/libglom/python_embed/py_glom_relatedrecord.cc
        glom/libglom/python_embed/py_glom_relatedrecord.h
        glom/libglom/python_embed/py_glom_ui.cc
        glom/libglom/python_embed/py_glom_ui.h
        glom/libglom/python_embed/py_glom_ui_callbacks.h
        glom/libglom/python_embed/pygdavalue_conversions.cc
        glom/libglom/python_embed/pygdavalue_conversions.h
        glom/libglom/algorithms_utils.h
        glom/libglom/appstate.cc
        glom/libglom/appstate.h
        glom/libglom/calcinprogress.cc
        glom/libglom/calcinprogress.h
        glom/libglom/connectionpool.cc
        glom/libglom/connectionpool.h
        glom/libglom/db_utils.cc
        glom/libglom/db_utils.h
        glom/libglom/db_utils_export.cc
        glom/libglom/db_utils_export.h
        glom/libglom/glom_postgres.cc
        glom/libglom/glom_postgres.h
        glom/libglom/init.cc
        glom/libglom/init.h
        glom/libglom/privs.cc
        glom/libglom/privs.h
        glom/libglom/report_builder.cc
        glom/libglom/report_builder.h
        glom/libglom/sharedptr.h
        glom/libglom/spawn_with_feedback.cc
        glom/libglom/spawn_with_feedback.h
        glom/libglom/standard_table_prefs_fields.h
        glom/libglom/translations_po.cc
        glom/libglom/translations_po.h
        glom/libglom/file_utils.cc
        glom/libglom/file_utils.h
        glom/libglom/string_utils.cc
        glom/libglom/string_utils.h
        glom/libglom/utils.cc
        glom/libglom/utils.h
        glom/libglom/xml_utils.cc
        glom/libglom/xml_utils.h
        glom/libglom/xsl_utils.cc
        glom/libglom/xsl_utils.h
        glom/mode_data/datawidget/cellcreation.cc
        glom/mode_data/datawidget/cellcreation.h
        glom/mode_data/datawidget/cellrenderer_buttonimage.cc
        glom/mode_data/datawidget/cellrenderer_buttonimage.h
        glom/mode_data/datawidget/cellrenderer_buttontext.cc
        glom/mode_data/datawidget/cellrenderer_buttontext.h
        glom/mode_data/datawidget/cellrenderer_dblist.cc
        glom/mode_data/datawidget/cellrenderer_dblist.h
        glom/mode_data/datawidget/checkbutton.cc
        glom/mode_data/datawidget/checkbutton.h
        glom/mode_data/datawidget/combo.cc
        glom/mode_data/datawidget/combo.h
        glom/mode_data/datawidget/combo_as_radio_buttons.cc
        glom/mode_data/datawidget/combo_as_radio_buttons.h
        glom/mode_data/datawidget/combochoices.cc
        glom/mode_data/datawidget/combochoices.h
        glom/mode_data/datawidget/combochoiceswithtreemodel.cc
        glom/mode_data/datawidget/combochoiceswithtreemodel.h
        glom/mode_data/datawidget/datawidget.cc
        glom/mode_data/datawidget/datawidget.h
        glom/mode_data/datawidget/dialog_choose_date.cc
        glom/mode_data/datawidget/dialog_choose_date.h
        glom/mode_data/datawidget/dialog_choose_id.cc
        glom/mode_data/datawidget/dialog_choose_id.h
        glom/mode_data/datawidget/dialog_new_record.cc
        glom/mode_data/datawidget/dialog_new_record.h
        glom/mode_data/datawidget/entry.cc
        glom/mode_data/datawidget/entry.h
        glom/mode_data/datawidget/label.cc
        glom/mode_data/datawidget/label.h
        glom/mode_data/datawidget/textview.cc
        glom/mode_data/datawidget/textview.h
        glom/mode_data/datawidget/treemodel_db.cc
        glom/mode_data/datawidget/treemodel_db.h
        glom/mode_data/datawidget/treemodel_db_withextratext.cc
        glom/mode_data/datawidget/treemodel_db_withextratext.h
        glom/mode_data/db_adddel/db_adddel.cc
        glom/mode_data/db_adddel/db_adddel.h
        glom/mode_data/db_adddel/db_adddel_withbuttons.cc
        glom/mode_data/db_adddel/db_adddel_withbuttons.h
        glom/mode_data/db_adddel/db_treeviewcolumn_glom.cc
        glom/mode_data/db_adddel/db_treeviewcolumn_glom.h
        glom/mode_data/box_data.cc
        glom/mode_data/box_data.h
        glom/mode_data/box_data_calendar_related.cc
        glom/mode_data/box_data_calendar_related.h
        glom/mode_data/box_data_details.cc
        glom/mode_data/box_data_details.h
        glom/mode_data/box_data_list.cc
        glom/mode_data/box_data_list.h
        glom/mode_data/box_data_list_related.cc
        glom/mode_data/box_data_list_related.h
        glom/mode_data/box_data_manyrecords.cc
        glom/mode_data/box_data_manyrecords.h
        glom/mode_data/box_data_portal.cc
        glom/mode_data/box_data_portal.h
        glom/mode_data/buttonglom.cc
        glom/mode_data/buttonglom.h
        glom/mode_data/flowtablewithfields.cc
        glom/mode_data/flowtablewithfields.h
        glom/mode_data/notebook_data.cc
        glom/mode_data/notebook_data.h
        glom/mode_design/fields/box_db_table_definition.cc
        glom/mode_design/fields/box_db_table_definition.h
        glom/mode_design/fields/combo_fieldtype.cc
        glom/mode_design/fields/combo_fieldtype.h
        glom/mode_design/fields/dialog_defaultformatting.cc
        glom/mode_design/fields/dialog_defaultformatting.h
        glom/mode_design/fields/dialog_fieldcalculation.cc
        glom/mode_design/fields/dialog_fieldcalculation.h
        glom/mode_design/fields/dialog_fielddefinition.cc
        glom/mode_design/fields/dialog_fielddefinition.h
        glom/mode_design/layout/layout_item_dialogs/box_formatting.cc
        glom/mode_design/layout/layout_item_dialogs/box_formatting.h
        glom/mode_design/layout/layout_item_dialogs/combo_summarytype.cc
        glom/mode_design/layout/layout_item_dialogs/combo_summarytype.h
        glom/mode_design/layout/layout_item_dialogs/comboentry_borderwidth.cc
        glom/mode_design/layout/layout_item_dialogs/comboentry_borderwidth.h
        glom/mode_design/layout/layout_item_dialogs/dialog_buttonscript.cc
        glom/mode_design/layout/layout_item_dialogs/dialog_buttonscript.h
        glom/mode_design/layout/layout_item_dialogs/dialog_field_layout.cc
        glom/mode_design/layout/layout_item_dialogs/dialog_field_layout.h
        glom/mode_design/layout/layout_item_dialogs/dialog_field_summary.cc
        glom/mode_design/layout/layout_item_dialogs/dialog_field_summary.h
        glom/mode_design/layout/layout_item_dialogs/dialog_fieldslist.cc
        glom/mode_design/layout/layout_item_dialogs/dialog_fieldslist.h
        glom/mode_design/layout/layout_item_dialogs/dialog_formatting.cc
        glom/mode_design/layout/layout_item_dialogs/dialog_formatting.h
        glom/mode_design/layout/layout_item_dialogs/dialog_group_by.cc
        glom/mode_design/layout/layout_item_dialogs/dialog_group_by.h
        glom/mode_design/layout/layout_item_dialogs/dialog_imageobject.cc
        glom/mode_design/layout/layout_item_dialogs/dialog_imageobject.h
        glom/mode_design/layout/layout_item_dialogs/dialog_line.cc
        glom/mode_design/layout/layout_item_dialogs/dialog_line.h
        glom/mode_design/layout/layout_item_dialogs/dialog_sortfields.cc
        glom/mode_design/layout/layout_item_dialogs/dialog_sortfields.h
        glom/mode_design/layout/layout_item_dialogs/dialog_textobject.cc
        glom/mode_design/layout/layout_item_dialogs/dialog_textobject.h
        glom/mode_design/layout/combobox_fields.cc
        glom/mode_design/layout/combobox_fields.h
        glom/mode_design/layout/combobox_relationship.cc
        glom/mode_design/layout/combobox_relationship.h
        glom/mode_design/layout/dialog_choose_field.cc
        glom/mode_design/layout/dialog_choose_field.h
        glom/mode_design/layout/dialog_choose_relationship.cc
        glom/mode_design/layout/dialog_choose_relationship.h
        glom/mode_design/layout/dialog_layout.cc
        glom/mode_design/layout/dialog_layout.h
        glom/mode_design/layout/dialog_layout_calendar_related.cc
        glom/mode_design/layout/dialog_layout_calendar_related.h
        glom/mode_design/layout/dialog_layout_details.cc
        glom/mode_design/layout/dialog_layout_details.h
        glom/mode_design/layout/dialog_layout_export.cc
        glom/mode_design/layout/dialog_layout_export.h
        glom/mode_design/layout/dialog_layout_list.cc
        glom/mode_design/layout/dialog_layout_list.h
        glom/mode_design/layout/dialog_layout_list_related.cc
        glom/mode_design/layout/dialog_layout_list_related.h
        glom/mode_design/layout/treestore_layout.cc
        glom/mode_design/layout/treestore_layout.h
        glom/mode_design/print_layouts/box_print_layouts.cc
        glom/mode_design/print_layouts/box_print_layouts.h
        glom/mode_design/print_layouts/dialog_text_formatting.cc
        glom/mode_design/print_layouts/dialog_text_formatting.h
        glom/mode_design/print_layouts/print_layout_toolbar.cc
        glom/mode_design/print_layouts/print_layout_toolbar.h
        glom/mode_design/print_layouts/print_layout_toolbar_button.cc
        glom/mode_design/print_layouts/print_layout_toolbar_button.h
        glom/mode_design/print_layouts/window_print_layout_edit.cc
        glom/mode_design/print_layouts/window_print_layout_edit.h
        glom/mode_design/relationships_overview/canvas_group_dbtable.cc
        glom/mode_design/relationships_overview/canvas_group_dbtable.h
        glom/mode_design/relationships_overview/printoperation_relationshipsoverview.cc
        glom/mode_design/relationships_overview/printoperation_relationshipsoverview.h
        glom/mode_design/relationships_overview/window_relationships_overview.cc
        glom/mode_design/relationships_overview/window_relationships_overview.h
        glom/mode_design/report_layout/dialog_layout_report.cc
        glom/mode_design/report_layout/dialog_layout_report.h
        glom/mode_design/report_layout/treestore_report_layout.cc
        glom/mode_design/report_layout/treestore_report_layout.h
        glom/mode_design/script_library/dialog_new_script.cc
        glom/mode_design/script_library/dialog_new_script.h
        glom/mode_design/script_library/dialog_script_library.cc
        glom/mode_design/script_library/dialog_script_library.h
        glom/mode_design/translation/combobox_locale.cc
        glom/mode_design/translation/combobox_locale.h
        glom/mode_design/translation/dialog_change_language.cc
        glom/mode_design/translation/dialog_change_language.h
        glom/mode_design/translation/dialog_copy_translation.cc
        glom/mode_design/translation/dialog_copy_translation.h
        glom/mode_design/translation/dialog_identify_original.cc
        glom/mode_design/translation/dialog_identify_original.h
        glom/mode_design/translation/window_translations.cc
        glom/mode_design/translation/window_translations.h
        glom/mode_design/users/dialog_choose_user.cc
        glom/mode_design/users/dialog_choose_user.h
        glom/mode_design/users/dialog_groups_list.cc
        glom/mode_design/users/dialog_groups_list.h
        glom/mode_design/users/dialog_new_group.cc
        glom/mode_design/users/dialog_new_group.h
        glom/mode_design/users/dialog_user.cc
        glom/mode_design/users/dialog_user.h
        glom/mode_design/users/dialog_users_list.cc
        glom/mode_design/users/dialog_users_list.h
        glom/mode_design/box_db_table_relationships.cc
        glom/mode_design/box_db_table_relationships.h
        glom/mode_design/comboentry_currency.cc
        glom/mode_design/comboentry_currency.h
        glom/mode_design/dialog_add_related_table.cc
        glom/mode_design/dialog_add_related_table.h
        glom/mode_design/dialog_database_preferences.cc
        glom/mode_design/dialog_database_preferences.h
        glom/mode_design/dialog_design.cc
        glom/mode_design/dialog_design.h
        glom/mode_design/dialog_fields.cc
        glom/mode_design/dialog_fields.h
        glom/mode_design/dialog_initial_password.cc
        glom/mode_design/dialog_initial_password.h
        glom/mode_design/dialog_relationships.cc
        glom/mode_design/dialog_relationships.h
        glom/mode_design/iso_codes.cc
        glom/mode_design/iso_codes.h
        glom/mode_find/box_data_details_find.cc
        glom/mode_find/box_data_details_find.h
        glom/mode_find/box_data_list_find.cc
        glom/mode_find/box_data_list_find.h
        glom/mode_find/notebook_find.cc
        glom/mode_find/notebook_find.h
        glom/navigation/box_tables.cc
        glom/navigation/box_tables.h
        glom/print_layout/canvas_layout_item.cc
        glom/print_layout/canvas_layout_item.h
        glom/print_layout/canvas_print_layout.cc
        glom/print_layout/canvas_print_layout.h
        glom/print_layout/print_layout_utils.cc
        glom/print_layout/print_layout_utils.h
        glom/print_layout/printoperation_printlayout.cc
        glom/print_layout/printoperation_printlayout.h
        glom/python_embed/glom_python.cc
        glom/python_embed/glom_python.h
        glom/python_embed/python_ui_callbacks.cc
        glom/python_embed/python_ui_callbacks.h
        glom/utility_widgets/adddel/adddel.cc
        glom/utility_widgets/adddel/adddel.h
        glom/utility_widgets/adddel/adddel_withbuttons.cc
        glom/utility_widgets/adddel/adddel_withbuttons.h
        glom/utility_widgets/adddel/treeviewcolumn_glom.cc
        glom/utility_widgets/adddel/treeviewcolumn_glom.h
        glom/utility_widgets/canvas/canvas_editable.cc
        glom/utility_widgets/canvas/canvas_editable.h
        glom/utility_widgets/canvas/canvas_group_grid.cc
        glom/utility_widgets/canvas/canvas_group_grid.h
        glom/utility_widgets/canvas/canvas_group_movable.cc
        glom/utility_widgets/canvas/canvas_group_movable.h
        glom/utility_widgets/canvas/canvas_group_resizable.cc
        glom/utility_widgets/canvas/canvas_group_resizable.h
        glom/utility_widgets/canvas/canvas_image_movable.cc
        glom/utility_widgets/canvas/canvas_image_movable.h
        glom/utility_widgets/canvas/canvas_item_movable.cc
        glom/utility_widgets/canvas/canvas_item_movable.h
        glom/utility_widgets/canvas/canvas_line_movable.cc
        glom/utility_widgets/canvas/canvas_line_movable.h
        glom/utility_widgets/canvas/canvas_rect_movable.cc
        glom/utility_widgets/canvas/canvas_rect_movable.h
        glom/utility_widgets/canvas/canvas_table_movable.cc
        glom/utility_widgets/canvas/canvas_table_movable.h
        glom/utility_widgets/canvas/canvas_text_movable.cc
        glom/utility_widgets/canvas/canvas_text_movable.h
        glom/utility_widgets/eggspreadtable/eggmarshalers.c
        glom/utility_widgets/eggspreadtable/eggmarshalers.h
        glom/utility_widgets/eggspreadtable/eggplaceholder.c
        glom/utility_widgets/eggspreadtable/eggplaceholder.h
        glom/utility_widgets/eggspreadtable/eggspreadtable.c
        glom/utility_widgets/eggspreadtable/eggspreadtable.h
        glom/utility_widgets/eggspreadtable/eggspreadtablednd.c
        glom/utility_widgets/eggspreadtable/eggspreadtablednd.h
        glom/utility_widgets/eggspreadtablemm/private/eggspreadtabledndmm_p.h
        glom/utility_widgets/eggspreadtablemm/private/eggspreadtablemm_p.h
        glom/utility_widgets/eggspreadtablemm/eggspreadtabledndmm.cc
        glom/utility_widgets/eggspreadtablemm/eggspreadtabledndmm.h
        glom/utility_widgets/eggspreadtablemm/eggspreadtablemm.cc
        glom/utility_widgets/eggspreadtablemm/eggspreadtablemm.h
        glom/utility_widgets/gimpruler/libgimpbase/gimpbase-private.c
        glom/utility_widgets/gimpruler/libgimpbase/gimpbase-private.h
        glom/utility_widgets/gimpruler/libgimpbase/gimpbase.h
        glom/utility_widgets/gimpruler/libgimpbase/gimpbaseenums.h
        glom/utility_widgets/gimpruler/libgimpbase/gimpbasetypes.h
        glom/utility_widgets/gimpruler/libgimpbase/gimpparam.h
        glom/utility_widgets/gimpruler/libgimpbase/gimpunit.c
        glom/utility_widgets/gimpruler/libgimpbase/gimpunit.h
        glom/utility_widgets/gimpruler/libgimpmath/gimpmath.h
        glom/utility_widgets/gimpruler/gimpruler.c
        glom/utility_widgets/gimpruler/gimpruler.h
        glom/utility_widgets/cellrendererlist.cc
        glom/utility_widgets/cellrendererlist.h
        glom/utility_widgets/combo_textglade.cc
        glom/utility_widgets/combo_textglade.h
        glom/utility_widgets/dialog_flowtable.cc
        glom/utility_widgets/dialog_flowtable.h
        glom/utility_widgets/dialog_image_load_progress.cc
        glom/utility_widgets/dialog_image_load_progress.h
        glom/utility_widgets/dialog_image_save_progress.cc
        glom/utility_widgets/dialog_image_save_progress.h
        glom/utility_widgets/dialog_properties.cc
        glom/utility_widgets/dialog_properties.h
        glom/utility_widgets/filechooserdialog_saveextras.cc
        glom/utility_widgets/filechooserdialog_saveextras.h
        glom/utility_widgets/flowtable.cc
        glom/utility_widgets/flowtable.h
        glom/utility_widgets/imageglom.cc
        glom/utility_widgets/imageglom.h
        glom/utility_widgets/layouttoolbar.cc
        glom/utility_widgets/layouttoolbar.h
        glom/utility_widgets/layouttoolbarbutton.cc
        glom/utility_widgets/layouttoolbarbutton.h
        glom/utility_widgets/layoutwidgetbase.cc
        glom/utility_widgets/layoutwidgetbase.h
        glom/utility_widgets/layoutwidgetfield.cc
        glom/utility_widgets/layoutwidgetfield.h
        glom/utility_widgets/layoutwidgetmenu.cc
        glom/utility_widgets/layoutwidgetmenu.h
        glom/utility_widgets/layoutwidgetutils.cc
        glom/utility_widgets/layoutwidgetutils.h
        glom/utility_widgets/notebook_noframe.cc
        glom/utility_widgets/notebook_noframe.h
        glom/utility_widgets/notebookglom.cc
        glom/utility_widgets/notebookglom.h
        glom/utility_widgets/notebooklabelglom.cc
        glom/utility_widgets/notebooklabelglom.h
        glom/utility_widgets/placeholder.cc
        glom/utility_widgets/placeholder.h
        glom/application.cc
        glom/application.h
        glom/appwindow.cc
        glom/appwindow.h
        glom/base_db.cc
        glom/base_db.h
        glom/base_db_table.cc
        glom/base_db_table.h
        glom/base_db_table_data.cc
        glom/base_db_table_data.h
        glom/base_db_table_data_readonly.cc
        glom/base_db_table_data_readonly.h
        glom/box_db_table.cc
        glom/box_db_table.h
        glom/box_reports.cc
        glom/box_reports.h
        glom/box_withbuttons.cc
        glom/box_withbuttons.h
        glom/dialog_connection.cc
        glom/dialog_connection.h
        glom/dialog_existing_or_new.cc
        glom/dialog_existing_or_new.h
        glom/dialog_invalid_data.cc
        glom/dialog_invalid_data.h
        glom/filechooser_export.cc
        glom/filechooser_export.h
        glom/frame_glom.cc
        glom/frame_glom.h
        glom/glade_utils.h
        glom/infobar_progress_creating.cc
        glom/infobar_progress_creating.h
        glom/main.cc
        glom/main_local_options.cc
        glom/main_local_options.h
        glom/main_remote_options.cc
        glom/main_remote_options.h
        glom/notebook_glom.cc
        glom/notebook_glom.h
        glom/onlineglom_strings.cc
        glom/show_progress_message.cc
        glom/show_progress_message.h
        glom/signal_reemitter.h
        glom/utils_ui.cc
        glom/utils_ui.h
        glom/variablesmap.cc
        glom/variablesmap.h
        glom/window_boxholder.cc
        glom/window_boxholder.h
        ${resource_file_generated_source})

# TODO: Python module:
#glom/python_embed/python_module/py_glom_module.cc
#glom/python_embed/python_module/py_glom_module.h

# TODO: Utilities:
#glom/glom_create_from_example.cc
#glom/glom_export_po.cc
#glom/glom_export_po_all.cc
#glom/glom_import_po_all.cc

# TODO: Tests
#glom/libglom/example_document_load.cc
#glom/utility_widgets/eggspreadtablemm/test_spreadtablednd.cc
#glom/test_pyembed.cc
#glom/test_pyembed_singleline.cc
#glom/glom_test_connection.cc
#glom/utility_widgets/test_flowtable.cc
#glom/utility_widgets/test_flowtable_dnd.cc
#glom/utility_widgets/canvas/test_canvas_editable.cc
#glom/mode_data/test_flowtablewithfields.cc
#glom/libglom/test_avahi_publisher.cc
#glom/libglom/test_connectionpool.cc
#glom/libglom/test_sync.cc
#tests/export/test_selfhosting_new_then_export.cc
#tests/export/test_selfhosting_new_then_export_with_big_num.cc
#tests/import/test_parsing.cc
#tests/import/test_signals.cc
#tests/import/utils.cc
#tests/import/utils.h
#tests/python/test_load_python_library.cc
#tests/python/test_python_execute_func.cc
#tests/python/test_python_execute_func_bad_syntax.cc
#tests/python/test_python_execute_func_change_result_type.cc
#tests/python/test_python_execute_func_date.cc
#tests/python/test_python_execute_func_with_record.cc
#tests/python/test_python_execute_func_with_record_field_types.cc
#tests/python/test_python_execute_script.cc
#tests/python/test_python_module.cc
#tests/translations_po/test_document_export_po.cc
#tests/translations_po/test_document_import_po.cc
#tests/ui/glade_toplevels_instantiation.cc
#tests/ui/test_glade_derived_instantiation.cc
#tests/ui/test_imageglom_widget.cc
#tests/test_conversions.cc
#tests/test_document_autosave.cc
#tests/test_document_change.cc
#tests/test_document_load.cc
#tests/test_document_load_and_change.cc
#tests/test_document_load_and_save.cc
#tests/test_document_load_image.cc
#tests/test_document_load_translations.cc
#tests/test_fake_connection.cc
#tests/test_field_file_format.cc
#tests/test_iso_codes.cc
#tests/test_layout_item_field.cc
#tests/test_parsing_time.cc
#tests/test_script_check_for_problems.cc
#tests/test_selfhosting_new_empty.cc
#tests/test_selfhosting_new_empty_change_sysprefs.cc
#tests/test_selfhosting_new_empty_then_users.cc
#tests/test_selfhosting_new_from_example.cc
#tests/test_selfhosting_new_from_example_defaultvalues.cc
#tests/test_selfhosting_new_from_example_float.cc
#tests/test_selfhosting_new_from_example_operator.cc
#tests/test_selfhosting_new_from_example_strangepath.cc
#tests/test_selfhosting_new_from_example_table_name_with_space.cc
#tests/test_selfhosting_new_then_alter_table.cc
#tests/test_selfhosting_new_then_backup_restore.cc
#tests/test_selfhosting_new_then_change_columns.cc
#tests/test_selfhosting_new_then_choices.cc
#tests/test_selfhosting_new_then_get_privs.cc
#tests/test_selfhosting_new_then_image.cc
#tests/test_selfhosting_new_then_lookup.cc
#tests/test_selfhosting_new_then_report.cc
#tests/test_selfhosting_new_then_report_summary.cc
#tests/test_selfhosting_non_numeric_primary_keys.cc
#tests/test_selfhosting_sqlinjection.cc
#tests/test_selfhosting_utils.cc
#tests/test_selfhosting_utils.h
#tests/test_signal_reemit.cc
#tests/test_utils.cc
#tests/test_utils.h
#tests/test_utils_images.cc
#tests/test_utils_images.h
#tests/test_xml_utils.cc

#TODO: Docs:
#docs/glom_internal_reference/Doxyfile
#docs/glom_internal_reference/Doxyfile_libglom
#docs/libglom_reference/Doxyfile.in
#docs/pyglom_reference/conf.py.in
#docs/pyglom_reference/index.rst.in
#docs/user-guide/C/figures/glom_data_details.png
#docs/user-guide/C/figures/glom_data_list.png
#docs/user-guide/C/figures/glom_design_details_layout_toolbar.png
#docs/user-guide/C/figures/glom_design_fields.png
#docs/user-guide/C/figures/glom_design_fields_dialog_calculated.png
#docs/user-guide/C/figures/glom_design_layout_details.png
#docs/user-guide/C/figures/glom_design_layout_list.png
#docs/user-guide/C/figures/glom_design_print_layout.png
#docs/user-guide/C/figures/glom_design_reports.png
#docs/user-guide/C/figures/glom_design_reports_details.png
#docs/user-guide/C/figures/glom_design_reports_group_by.png
#docs/user-guide/C/figures/glom_design_reports_vertical_group.png
#docs/user-guide/C/figures/glom_design_translations.png
#docs/user-guide/C/figures/glom_import.png
#docs/user-guide/C/figures/glom_initial_dialog_local_network.png
#docs/user-guide/C/figures/glom_report_result.png
#docs/user-guide/C/figures/glom_tables.png
#docs/user-guide/C/figures/start_create.png
#docs/user-guide/C/figures/start_open.png
#docs/user-guide/C/index.docbook
#docs/user-guide/C/legal.xml
#docs/user-guide/cs/cs.po
#docs/user-guide/de/figures/glom_data_details.png
#docs/user-guide/de/figures/glom_data_list.png
#docs/user-guide/de/figures/glom_design_fields.png
#docs/user-guide/de/figures/glom_design_layout_details.png
#docs/user-guide/de/figures/glom_design_layout_list.png
#docs/user-guide/de/figures/glom_design_reports.png
#docs/user-guide/de/figures/glom_design_reports_details.png
#docs/user-guide/de/figures/glom_design_reports_group_by.png
#docs/user-guide/de/figures/glom_design_reports_vertical_group.png
#docs/user-guide/de/figures/glom_design_translations.png
#docs/user-guide/de/figures/glom_tables.png
#docs/user-guide/de/figures/start.png
#docs/user-guide/de/de.po
#docs/user-guide/el/el.po
#docs/user-guide/en_GB/en_GB.po
#docs/user-guide/es/figures/start.png
#docs/user-guide/es/es.po
#docs/user-guide/es/glom-C.es.po
#docs/user-guide/es/glom.es.xml
#docs/user-guide/es/glom.po
#docs/user-guide/fr/figures/glom_data_details.png
#docs/user-guide/fr/figures/glom_data_list.png
#docs/user-guide/fr/figures/glom_design_fields.png
#docs/user-guide/fr/figures/glom_design_fields_dialog_calculated.png
#docs/user-guide/fr/figures/glom_design_layout_details.png
#docs/user-guide/fr/figures/glom_design_layout_list.png
#docs/user-guide/fr/figures/glom_design_reports.png
#docs/user-guide/fr/figures/glom_design_reports_details.png
#docs/user-guide/fr/figures/glom_design_reports_group_by.png
#docs/user-guide/fr/figures/glom_design_reports_vertical_group.png
#docs/user-guide/fr/figures/glom_design_translations.png
#docs/user-guide/fr/figures/glom_report_result.png
#docs/user-guide/fr/figures/glom_tables.png
#docs/user-guide/fr/figures/start.png
#docs/user-guide/fr/fr.po
#docs/user-guide/oc/oc.po
#docs/user-guide/sl/sl.po
#docs/user-guide/sv/sv.po
#docs/user-guide/ChangeLog

#TODO: Icons
#data/icons/16x16/glom.png
#data/icons/16x16/glom.svg
#data/icons/22x22/glom.png
#data/icons/22x22/glom.svg
#data/icons/24x24/glom.png
#data/icons/32x32/glom.png
#data/icons/32x32/glom.svg
#data/icons/48x48/glom.png
#data/icons/scalable/glom.svg
#data/icons/win32/glom.ico
#data/icons/glom-button.png
#data/icons/glom-field.png
#data/icons/glom-group.png
#data/icons/glom-image.png
#data/icons/glom-line-horizontal.png
#data/icons/glom-line-vertical.png
#data/icons/glom-notebook.png
#data/icons/glom-related-records.png
#data/icons/glom-text.png
#data/icons/glom_icon_large.png

#TODO: Examples
#examples/example_scripts/repository_analyzer_begin_scan.py
#examples/po_files/example_film_manager/bs.po
#examples/po_files/example_film_manager/cs.po
#examples/po_files/example_film_manager/de.po
#examples/po_files/example_film_manager/el.po
#examples/po_files/example_film_manager/en_GB.po
#examples/po_files/example_film_manager/eo.po
#examples/po_files/example_film_manager/es.po
#examples/po_files/example_film_manager/eu.po
#examples/po_files/example_film_manager/example_film_manager.pot
#examples/po_files/example_film_manager/fr.po
#examples/po_files/example_film_manager/gl.po
#examples/po_files/example_film_manager/hu.po
#examples/po_files/example_film_manager/id.po
#examples/po_files/example_film_manager/pl.po
#examples/po_files/example_film_manager/pt.po
#examples/po_files/example_film_manager/pt_BR.po
#examples/po_files/example_film_manager/sl.po
#examples/po_files/example_film_manager/sr.po
#examples/po_files/example_film_manager/sr@latin.po
#examples/po_files/example_film_manager/sv.po
#examples/po_files/example_film_manager/tr.po
#examples/po_files/example_lesson_planner/bs.po
#examples/po_files/example_lesson_planner/cs.po
#examples/po_files/example_lesson_planner/de.po
#examples/po_files/example_lesson_planner/el.po
#examples/po_files/example_lesson_planner/en_GB.po
#examples/po_files/example_lesson_planner/es.po
#examples/po_files/example_lesson_planner/eu.po
#examples/po_files/example_lesson_planner/example_lesson_planner.pot
#examples/po_files/example_lesson_planner/fr.po
#examples/po_files/example_lesson_planner/hu.po
#examples/po_files/example_lesson_planner/id.po
#examples/po_files/example_lesson_planner/lv.po
#examples/po_files/example_lesson_planner/pl.po
#examples/po_files/example_lesson_planner/pt.po
#examples/po_files/example_lesson_planner/pt_BR.po
#examples/po_files/example_lesson_planner/sl.po
#examples/po_files/example_lesson_planner/sr.po
#examples/po_files/example_lesson_planner/sr@latin.po
#examples/po_files/example_lesson_planner/sv.po
#examples/po_files/example_lesson_planner/tr.po
#examples/po_files/example_music_collection/bs.po
#examples/po_files/example_music_collection/cs.po
#examples/po_files/example_music_collection/de.po
#examples/po_files/example_music_collection/el.po
#examples/po_files/example_music_collection/es.po
#examples/po_files/example_music_collection/eu.po
#examples/po_files/example_music_collection/example_music_collection.pot
#examples/po_files/example_music_collection/fr.po
#examples/po_files/example_music_collection/gl.po
#examples/po_files/example_music_collection/hu.po
#examples/po_files/example_music_collection/id.po
#examples/po_files/example_music_collection/lv.po
#examples/po_files/example_music_collection/pl.po
#examples/po_files/example_music_collection/pt.po
#examples/po_files/example_music_collection/pt_BR.po
#examples/po_files/example_music_collection/sl.po
#examples/po_files/example_music_collection/sr.po
#examples/po_files/example_music_collection/sr@latin.po
#examples/po_files/example_music_collection/sv.po
#examples/po_files/example_music_collection/tr.po
#examples/po_files/example_music_collection/zh_CN.po
#examples/po_files/example_project_manager/bs.po
#examples/po_files/example_project_manager/cs.po
#examples/po_files/example_project_manager/de.po
#examples/po_files/example_project_manager/el.po
#examples/po_files/example_project_manager/en_GB.po
#examples/po_files/example_project_manager/es.po
#examples/po_files/example_project_manager/eu.po
#examples/po_files/example_project_manager/example_project_manager.pot
#examples/po_files/example_project_manager/fr.po
#examples/po_files/example_project_manager/gl.po
#examples/po_files/example_project_manager/hu.po
#examples/po_files/example_project_manager/id.po
#examples/po_files/example_project_manager/lv.po
#examples/po_files/example_project_manager/pl.po
#examples/po_files/example_project_manager/pt.po
#examples/po_files/example_project_manager/pt_BR.po
#examples/po_files/example_project_manager/sl.po
#examples/po_files/example_project_manager/sr.po
#examples/po_files/example_project_manager/sr@latin.po
#examples/po_files/example_project_manager/sv.po
#examples/po_files/example_project_manager/tr.po
#examples/po_files/example_smallbusiness/bs.po
#examples/po_files/example_smallbusiness/cs.po
#examples/po_files/example_smallbusiness/de.po
#examples/po_files/example_smallbusiness/el.po
#examples/po_files/example_smallbusiness/en_GB.po
#examples/po_files/example_smallbusiness/es.po
#examples/po_files/example_smallbusiness/eu.po
#examples/po_files/example_smallbusiness/example_smallbusiness.pot
#examples/po_files/example_smallbusiness/fr.po
#examples/po_files/example_smallbusiness/gl.po
#examples/po_files/example_smallbusiness/hu.po
#examples/po_files/example_smallbusiness/id.po
#examples/po_files/example_smallbusiness/lv.po
#examples/po_files/example_smallbusiness/pl.po
#examples/po_files/example_smallbusiness/pt.po
#examples/po_files/example_smallbusiness/pt_BR.po
#examples/po_files/example_smallbusiness/sl.po
#examples/po_files/example_smallbusiness/sr.po
#examples/po_files/example_smallbusiness/sr@latin.po
#examples/po_files/example_smallbusiness/sv.po
#examples/po_files/example_smallbusiness/tr.po
#examples/po_files/README
#examples/sqlite/test_sqlite_music/glom_musiccollection21.db
#examples/sqlite/test_sqlite_music/test_sqlite_music.glom
#examples/tests/example_field_formatting_test.glom
#examples/example_film_manager.glom
#examples/example_lesson_planner.glom
#examples/example_music_collection.glom
#examples/example_project_manager.glom
#examples/example_smallbusiness.glom
#examples/glom-examples.gresource.xml
#examples/README

add_executable(glom ${SOURCE_FILES})

# By default, cmake builds the executable at the top-level,
# but our source directory has the same name,
# so we override RUNTIME_OUTPUT_DIRECTORY.
set_target_properties(glom
        PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)



# Libraries used by libglom:
# Note: cmake doesn't like spaces either side of the > or >=
# Note: cmake doesn't like " quotes around this.
# Note: You'll need to delete CMakeCache.txt if you add something to this list.
set(REQUIRED_LIBGLOM_LIBS
        giomm-2.4>=2.47.4
        libxml++-3.0>=3.0.0
        libxslt>=1.1.10 pygobject-3.0>=2.29.0
        libgdamm-5.0>=4.99.10
        libgda-5.0>=5.2.1
        libgda-postgres-5.0
        libgda-postgres-5.0
        libgda-mysql-5.0
        libarchive>=3.0
        libepc-1.0>=0.4.0)

# Libraries used by Glom:
set(REQUIRED_GLOM_LIBS
        ${REQUIRED_LIBGLOM_LIBS}
        gtkmm-3.0>=3.18.0
        goocanvasmm-2.0>=1.90.11
        goocanvas-2.0>=2.0.1
        evince-view-3.0
        gtksourceviewmm-3.0>=3.18.0
        iso-codes)

include(FindPkgConfig)
pkg_check_modules(DEPS REQUIRED ${REQUIRED_GLOM_LIBS})
include_directories(${DEPS_INCLUDE_DIRS})
target_link_libraries(glom ${DEPS_LIBRARIES})

# Note: We have to call target_link_libraries() after add_executable(),
# which we have to do after listing all the source files.

# Python has no pkg-config file.
# This is awkward with autotools too.
find_package(PythonLibs 3 REQUIRED)
include_directories(${PYTHON_INCLUDE_DIR})
target_link_libraries(glom ${PYTHON_LIBRARY})


# Boost::Python has no pkg-config file.
# This is awkward with autotools too.
# Note: CMake variable names are case-dependent, even though function names are not.
# TODO: On Ubuntu 16.10, at least,
# the PythonLibs check finds python3.4, but the Boost::Python check find the 
# version of Boost::Python built for python2.7.
# Specifying python-py34 instead of python fixes this, but then we are
# hard-coding a version and hoping that it's the same as the one found by
# PythonLibs.
find_package(Boost COMPONENTS python-py34)
include_directories(${Boost_INCLUDE_DIR})
target_link_libraries(glom ${Boost_PYTHON-PY34_LIBRARY})


include(CMakeScripts/GlomFindGettextPO.cmake)
include_directories(${GETTEXTPO_INCLUDE_DIR})
target_link_libraries(glom ${GETTEXTPO_LIBRARY})


# Let the C++ compiler find the #include files for our own source code:
include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR})

# Let the C++ compiler find the #include files for our own source code in libglom:
include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}/glom
        ${CMAKE_CURRENT_BINARY_DIR}/glom)

#Let gimpruler build without changing the paths in its source code:
include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}/glom/utility_widgets/gimpruler)


link_directories(${DEPS_LIBRARY_DIRS})


# Locate the directory containing the PostgreSQL utilities, such as the
# postmaster executable, so we can self-host PostgreSQL databases.
# Note: This is a particularly shocking example of cmake's quoting problem.
# Note: OUTPUT_STRIP_TRAILING_WHITESPACE is not documented other than being
# mentioned as a possible option: https://cmake.org/cmake/help/v3.4/command/execute_process.html
execute_process(COMMAND pg_config --bindir
        OUTPUT_VARIABLE POSTGRES_UTILS_PATH
        OUTPUT_STRIP_TRAILING_WHITESPACE)


# Locate the directory containing the MySQL utilities, such as the
# postmaster executable, so we can self-host MySQL databases.
# TODO: Do this properly.
set(MYSQL_UTILS_PATH "/usr/bin")

#  Note: In our cmake-config.h.in file, we use #define instead of #cmakedefine, to let this be an empty string.
if(WIN32)
    set(EXEEXT ".exe")
else(WIN32)
    set(EXEEXT "")
endif(WIN32)


# libgettext-po changed its API, changing the error handler struct
# from po_error_handler to po_xerror_handler:
include(CheckStructHasMember)
check_struct_has_member(
        "struct po_xerror_handler"
        xerror
        gettext-po.h
        HAVE_GETTEXTPO_XERROR)


# Get the location of the ISO-Codes (currencies, languages) files,
# to define ISO_CODES_PREFIX
include(CMakeScripts/GlomPkgCheckVariable.cmake)
glom_pkg_check_variable(iso-codes prefix)


# Generate config.h from config.h.in:
configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake-config.h.in
        ${CMAKE_CURRENT_BINARY_DIR}/config.h)
configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/glom/libglom/cmake-libglom_config.h.in
        ${CMAKE_CURRENT_BINARY_DIR}/glom/libglom/libglom_config.h)


