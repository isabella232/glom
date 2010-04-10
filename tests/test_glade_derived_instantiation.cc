#include <gtkmm.h>
#include <gtksourceviewmm/init.h>
#include <glom/glade_utils.h>
#include <glom/application.h>
#include <glom/dialog_existing_or_new.h>
#include <glom/mode_design/print_layouts/box_print_layouts.h>
#include <glom/mode_design/relationships_overview/dialog_relationships_overview.h>
#include <glom/mode_design/dialog_relationships.h>
#include <glom/mode_design/report_layout/dialog_layout_report.h>
#include <glom/box_reports.h>
#include <glom/navigation/box_tables.h>
#include <glom/import_csv/dialog_import_csv.h>
#include <glom/import_csv/dialog_import_csv_progress.h>
#include <glom/mode_data/datawidget/dialog_choose_date.h>
#include <glom/mode_data/datawidget/dialog_choose_id.h>
#include <glom/utility_widgets/dialog_flowtable.h>
#include <glom/utility_widgets/dialog_image_progress.h>
#include <glom/mode_design/layout/dialog_choose_field.h>
#include <glom/mode_design/dialog_add_related_table.h>
#include <glom/mode_design/layout/layout_item_dialogs/dialog_buttonscript.h>
#include <glom/mode_design/layout/layout_item_dialogs/dialog_field_layout.h>
#include <glom/mode_design/layout/layout_item_dialogs/dialog_imageobject.h>
#include <glom/mode_design/layout/layout_item_dialogs/dialog_notebook.h>
#include <glom/mode_design/layout/layout_item_dialogs/dialog_textobject.h>
#include <glom/mode_design/layout/layout_item_dialogs/box_formatting.h>
#include <glom/mode_design/layout/layout_item_dialogs/dialog_field_summary.h>
#include <glom/mode_design/layout/layout_item_dialogs/dialog_group_by.h>
#include <glom/mode_design/layout/dialog_layout_calendar_related.h>
#include <glom/mode_design/layout/dialog_layout_details.h>
#include <glom/mode_design/layout/dialog_layout_list.h>
#include <glom/mode_design/layout/dialog_layout_list_related.h>
#include <glom/mode_design/layout/dialog_choose_relationship.h>
#include <glom/mode_design/layout/dialog_layout_export.h>
#include <glom/mode_design/dialog_database_preferences.h>
#include <glom/mode_design/fields/dialog_fielddefinition.h>
#include <glom/mode_design/dialog_fields.h>
#include <glom/mode_design/dialog_initial_password.h>
#include <glom/mode_design/layout/layout_item_dialogs/dialog_groupby_secondaryfields.h>
#include <glom/mode_design/layout/layout_item_dialogs/dialog_groupby_sortfields.h>
#include <glom/mode_design/layout/layout_item_dialogs/dialog_formatting.h>
#include <glom/mode_design/script_library/dialog_new_script.h>
#include <glom/mode_design/script_library/dialog_script_library.h>
#include <glom/mode_design/translation/dialog_identify_original.h>
#include <glom/mode_design/translation/dialog_copy_translation.h>
#include <glom/mode_design/translation/dialog_change_language.h>
#include <glom/mode_design/translation/window_translations.h>
#include <glom/mode_design/users/dialog_new_group.h>
#include <glom/mode_design/users/dialog_groups_list.h>
#include <glom/mode_design/users/dialog_users_list.h>
#include <glom/mode_design/users/dialog_choose_user.h>
#include <glom/mode_design/users/dialog_user.h>
#include <glom/mode_design/print_layouts/dialog_text_formatting.h>
#include <glom/dialog_progress_creating.h>
#include <glom/dialog_invalid_data.h>


template<class T_Widget>
bool instantiate_widget()
{
  //Test that the widget can be instantiated with its own glade ID.
  T_Widget* widget = 0;
  Glom::Utils::get_glade_widget_derived_with_warning(widget);    
  if(!widget)
  {
    std::cerr << "Test: Failed to instantiate widget of type: " << typeid(T_Widget).name() << std::endl;
    exit(EXIT_FAILURE); //Make sure that our test case fails.
    return false;
  }
    
  delete widget;
  return true;
}

int main(int argc, char *argv[])
{
  Gtk::Main kit(argc, argv);
  gtksourceview::init(); //Our .glade files contain gtksourceview widgets too.

  using namespace Glom;

  //Operator-mode dialogs:
  instantiate_widget<Application>();
  instantiate_widget<Dialog_ExistingOrNew>();
  instantiate_widget<Box_Tables>();
  instantiate_widget<Dialog_Import_CSV>();
  instantiate_widget<Dialog_Import_CSV_Progress>();
  instantiate_widget<DataWidgetChildren::Dialog_ChooseID>();
  instantiate_widget<DataWidgetChildren::Dialog_ChooseDate>(); 
  instantiate_widget<Dialog_InvalidData>();   
  instantiate_widget<Dialog_Image_Progress>();
  instantiate_widget<Dialog_ProgressCreating>();

  instantiate_widget<Box_Print_Layouts>();
  instantiate_widget<Dialog_RelationshipsOverview>();
  instantiate_widget<Dialog_Relationships>();
  instantiate_widget<Dialog_Layout_Report>();
  instantiate_widget<Box_Reports>();
  instantiate_widget<Dialog_ChooseField>();
  instantiate_widget<Dialog_FieldLayout>();
  instantiate_widget<Dialog_TextObject>();
  instantiate_widget<Dialog_Layout_Calendar_Related>();
  instantiate_widget<Dialog_Layout_Details>();
  instantiate_widget<Dialog_Layout_List>();
  //TODO: instantiate_widget<Dialog_Layout_List_Related>();
  instantiate_widget<Dialog_ButtonScript>();
  instantiate_widget<Dialog_FlowTable>();
  instantiate_widget<Dialog_ChooseRelationship>();
  instantiate_widget<Dialog_FieldDefinition>();
  instantiate_widget<Box_Formatting>();
  instantiate_widget<Dialog_GroupBy_SecondaryFields>();
  instantiate_widget<Dialog_GroupBy_SortFields>();
  instantiate_widget<Dialog_ImageObject>();
  instantiate_widget<Dialog_Notebook>();
  instantiate_widget<Dialog_FieldSummary>();
  instantiate_widget<Dialog_GroupBy>();
  instantiate_widget<Dialog_IdentifyOriginal>();
  instantiate_widget<Dialog_NewScript>();
  instantiate_widget<Dialog_ScriptLibrary>();
  instantiate_widget<Dialog_CopyTranslation>();
  instantiate_widget<Dialog_ChangeLanguage>();
  instantiate_widget<Window_Translations>();
  instantiate_widget<Dialog_NewGroup>();
  instantiate_widget<Dialog_UsersList>();
  instantiate_widget<Dialog_GroupsList>();
  instantiate_widget<Dialog_ChooseUser>(); 
  instantiate_widget<Dialog_User>();
  instantiate_widget<Dialog_TextFormatting>();
  instantiate_widget<Dialog_Layout_Export>();
  instantiate_widget<Dialog_AddRelatedTable>();
  instantiate_widget<Dialog_Database_Preferences>();
  instantiate_widget<Dialog_Fields>();
  instantiate_widget<Dialog_InitialPassword>();
   
  return EXIT_SUCCESS;
}
