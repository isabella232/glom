/* Glom
 *
 * Copyright (C) 2001-2004 Murray Cumming
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 */

#include "config.h" //For GLOM_ENABLE_SQLITE
#include <libglom/libglom_config.h> // For GLOM_ENABLE_CLIENT_ONLY

#include <glom/frame_glom.h>
#include <glom/appwindow.h>
#include <glom/import_csv/dialog_import_csv.h>
#include <glom/import_csv/dialog_import_csv_progress.h>
#include <libglom/appstate.h>

#include <libglom/connectionpool.h>

#ifdef GLOM_ENABLE_POSTGRESQL
#include <libglom/connectionpool_backends/postgres_central.h>
#include <libglom/connectionpool_backends/postgres_self.h>
#endif

#ifndef GLOM_ENABLE_CLIENT_ONLY
#include <glom/mode_design/users/dialog_groups_list.h>
#include <glom/mode_design/dialog_database_preferences.h>
#include <glom/mode_design/report_layout/dialog_layout_report.h>
#include <glom/mode_design/print_layouts/window_print_layout_edit.h>
#include <glom/mode_design/dialog_add_related_table.h>
#include <glom/mode_design/script_library/dialog_script_library.h>
#include <glom/mode_design/dialog_initial_password.h>
#include <glom/mode_design/relationships_overview/window_relationships_overview.h>
#include <glom/glade_utils.h>
#endif // !GLOM_ENABLE_CLIENT_ONLY

#include <glom/utils_ui.h>
#include <glom/glade_utils.h>
#include <libglom/data_structure/glomconversions.h>
#include <libglom/data_structure/layout/report_parts/layoutitem_summary.h>
#include <libglom/data_structure/layout/report_parts/layoutitem_fieldsummary.h>

#include <glom/libglom/report_builder.h>
#ifndef GLOM_ENABLE_CLIENT_ONLY
#include <glom/mode_design/dialog_add_related_table.h>
#include <glom/mode_design/script_library/dialog_script_library.h>
#include <glom/print_layout/printoperation_printlayout.h>
#endif // !GLOM_ENABLE_CLIENT_ONLY

#include <glom/print_layout/print_layout_utils.h>

#include <glom/filechooser_export.h>
#include <libglom/privs.h>
#include <libglom/db_utils.h>
#include <sstream> //For stringstream.
#include <fstream>
#include <glibmm/i18n.h>

namespace Glom
{

Frame_Glom::Frame_Glom(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
: PlaceHolder(cobject, builder),
  m_pLabel_Table_DataMode(nullptr),
  m_pLabel_Table_FindMode(nullptr),
  m_Box_RecordsCount(Gtk::ORIENTATION_HORIZONTAL, Utils::to_utype(UiUtils::DefaultSpacings::SMALL)),
  m_Button_FindAll(_("Find All")),
  m_stack_mode(nullptr),
  m_pBox_Tables(nullptr),
  m_pDialog_Tables(nullptr),
  m_pBox_QuickFind(nullptr),
  m_pEntry_QuickFind(nullptr),
  m_pButton_QuickFind(nullptr),
#ifndef GLOM_ENABLE_CLIENT_ONLY
  m_pDialog_Reports(nullptr),
  m_pDialogLayoutReport(nullptr),
  m_pBox_Reports(nullptr),
  m_pDialog_PrintLayouts(nullptr),
  m_pDialogLayoutPrint(nullptr),
  m_pBox_PrintLayouts(nullptr),
  m_pDialog_Fields(nullptr),
  m_pDialog_Relationships(nullptr),
  m_dialog_addrelatedtable(nullptr),
  m_window_relationships_overview(nullptr),
#endif // !GLOM_ENABLE_CLIENT_ONLY
  m_pDialogConnection(nullptr)
{
  m_pLabel_Table_DataMode = Gtk::manage(new Gtk::Label(_("No Table Selected")));
  m_pLabel_Table_DataMode->show();
  m_Notebook_Data.set_action_widget(m_pLabel_Table_DataMode, Gtk::PACK_START);

  m_pLabel_Table_FindMode = Gtk::manage(new Gtk::Label(_("No Table Selected")));
  m_pLabel_Table_FindMode->show();
  m_Notebook_Find.set_action_widget(m_pLabel_Table_FindMode, Gtk::PACK_START);

  //QuickFind widgets:
  //We don't use Glade for these, so it easier to modify them for the Maemo port.
  m_pBox_QuickFind = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL, Utils::to_utype(UiUtils::DefaultSpacings::SMALL)));
  auto label = Gtk::manage(new Gtk::Label(_("Quick _search:"), true));
  m_pBox_QuickFind->pack_start(*label, Gtk::PACK_SHRINK);

  m_pEntry_QuickFind = Gtk::manage(new Gtk::Entry());

  //Pressing Enter here is like pressing Find:
  m_pEntry_QuickFind->set_activates_default();

  label->set_mnemonic_widget(*m_pEntry_QuickFind);

  m_pBox_QuickFind->pack_start(*m_pEntry_QuickFind, Gtk::PACK_EXPAND_WIDGET);
  m_pButton_QuickFind = Gtk::manage(new Gtk::Button(_("_Find"), true));
  m_pButton_QuickFind->signal_clicked().connect(
    sigc::mem_fun(*this, &Frame_Glom::on_button_quickfind) );
  m_pBox_QuickFind->pack_start(*m_pButton_QuickFind, Gtk::PACK_SHRINK);

  m_pBox_QuickFind->show_all_children();
  m_pBox_QuickFind->hide();

  PlaceHolder* placeholder_quickfind = nullptr;
  builder->get_widget_derived("vbox_quickfind", placeholder_quickfind);
  if(placeholder_quickfind)
    placeholder_quickfind->add(*m_pBox_QuickFind);

  //Add the Records/Found widgets at the right of the notebook tabs:
  m_Box_RecordsCount.pack_start(
    *Gtk::manage(new Gtk::Label(_("Records:"))), Gtk::PACK_SHRINK);
  m_Box_RecordsCount.pack_start(m_Label_RecordsCount, Gtk::PACK_SHRINK);
  m_Box_RecordsCount.pack_start(
    *Gtk::manage(new Gtk::Label(_("Found:"))), Gtk::PACK_SHRINK);
  m_Box_RecordsCount.pack_start(m_Label_FoundCount, Gtk::PACK_SHRINK);
  m_Box_RecordsCount.pack_start(m_Button_FindAll, Gtk::PACK_SHRINK);
  m_Box_RecordsCount.show_all();
  m_Notebook_Data.set_action_widget(&m_Box_RecordsCount, Gtk::PACK_END);
  m_Button_FindAll.signal_clicked().connect(
    sigc::mem_fun(*this, &Frame_Glom::on_button_find_all) );

  builder->get_widget("stack_mode", m_stack_mode);
  if(m_stack_mode)
  {
    m_stack_mode->add(m_Notebook_Data, "data");
    m_stack_mode->add(m_Notebook_Find, "find");
    m_stack_mode->set_visible_child(m_Notebook_Data);
    m_Notebook_Data.set_enable_layout_drag_and_drop(false);
  }

  m_Mode = enumModes::NONE;
  m_Mode_Previous = enumModes::NONE;


  m_Notebook_Find.signal_find_criteria.connect(sigc::mem_fun(*this, &Frame_Glom::on_notebook_find_criteria));
  m_Notebook_Find.show();
  m_Notebook_Data.signal_record_details_requested().connect(sigc::mem_fun(*this, &Frame_Glom::on_notebook_data_record_details_requested));
  m_Notebook_Data.signal_record_selection_changed().connect(sigc::mem_fun(*this,
    &Frame_Glom::on_notebook_data_record_selection_changed));
  m_Notebook_Data.signal_switch_page().connect(sigc::mem_fun(*this, &Frame_Glom::on_notebook_data_switch_page));
  m_Notebook_Data.show();

  //Fill Composite View:
  //This means that set_document and load/save are delegated to these children:
  add_view(&m_Notebook_Data); //Also a composite view.
  add_view(&m_Notebook_Find); //Also a composite view.

  on_userlevel_changed(AppState::userlevels::OPERATOR); //A default to show before a document is created or loaded.
}

Frame_Glom::~Frame_Glom()
{
  if(m_pBox_Tables)
    remove_view(m_pBox_Tables);

  delete m_pDialog_Tables;
  m_pDialog_Tables = nullptr;

  remove_view(&m_Notebook_Data); //Also a composite view.
  remove_view(&m_Notebook_Find); //Also a composite view.


  if(m_pDialogConnection)
  {
    remove_view(m_pDialogConnection);
    delete m_pDialogConnection;
    m_pDialogConnection = nullptr;
  }

#ifndef GLOM_ENABLE_CLIENT_ONLY
  if(m_pBox_Reports)
    remove_view(m_pBox_Reports);

  if(m_pBox_PrintLayouts)
    remove_view(m_pBox_PrintLayouts);

  if(m_pDialog_Relationships)
  {
    remove_view(m_pDialog_Relationships);
    delete m_pDialog_Relationships;
    m_pDialog_Relationships = nullptr;
  }

  if(m_pDialogLayoutReport)
  {
    remove_view(m_pDialogLayoutReport);
    delete m_pDialogLayoutReport;
    m_pDialogLayoutReport = nullptr;
  }

  if(m_pDialogLayoutPrint)
  {
    remove_view(m_pDialogLayoutPrint);
    delete m_pDialogLayoutPrint;
    m_pDialogLayoutPrint = nullptr;
  }

  if(m_pDialog_Fields)
  {
    remove_view(m_pDialog_Fields);
    delete m_pDialog_Fields;
    m_pDialog_Fields = nullptr;
  }

  if(m_dialog_addrelatedtable)
  {
    remove_view(m_dialog_addrelatedtable);
    delete m_dialog_addrelatedtable;
    m_dialog_addrelatedtable = nullptr;
  }

  if(m_window_relationships_overview)
  {
    remove_view(m_window_relationships_overview);
    delete m_window_relationships_overview;
    m_window_relationships_overview = nullptr;
  }
#endif // !GLOM_ENABLE_CLIENT_ONLY
}

void Frame_Glom::set_databases_selected(const Glib::ustring& strName)
{
  //m_pDialog_Databases->hide(); //cause_close();

  get_document()->set_connection_database(strName);

  //show_system_name();

  do_menu_Navigate_Table(true /* open default */);
}

void Frame_Glom::on_box_tables_selected(const Glib::ustring& strName)
{
  if(m_pDialog_Tables)
    m_pDialog_Tables->hide();

  show_table(strName);
}

void Frame_Glom::set_mode_widget(Gtk::Widget& widget)
{
  m_stack_mode->set_visible_child(widget);
}

bool Frame_Glom::set_mode(enumModes mode)
{
  //TODO: This seems to be called twice when changing mode.
  const bool changed = (m_Mode != mode);

  //Choose a default mode, if necessary:
  if(mode == enumModes::NONE)
    mode = enumModes::DATA;

  m_Mode_Previous = m_Mode;
  m_Mode = mode;

  //Hide the Quick Find widgets if we are not in Find mode.
  const bool show_quickfind = (m_Mode == enumModes::FIND);
  if(show_quickfind)
  {
    m_pBox_QuickFind->show();

    //Clear the quick-find entry, ready for a new Find.
    if(changed)
    {
      m_pEntry_QuickFind->set_text(Glib::ustring());

      //Put the cursor in the quick find entry:
      m_pEntry_QuickFind->grab_focus();
    }
  }
  else
  {
    m_pBox_QuickFind->hide();
  }


  //Show the main part of the UI:
  if(m_Mode == enumModes::FIND)
    set_mode_widget(m_Notebook_Find);
  else
    set_mode_widget(m_Notebook_Data);

  return changed;
}

void Frame_Glom::alert_no_table()
{
  //Ask user to choose a table first:
  auto pWindowApp = get_app_window();
  if(pWindowApp)
  {
    //TODO: Obviously this document should have been deleted when the database-creation was cancelled.
    /* Note that "canceled" is the correct US spelling. */
    show_ok_dialog(_("No table"), _("This database has no tables yet."), *pWindowApp, Gtk::MESSAGE_WARNING);
  }
}

void Frame_Glom::show_table_refresh()
{
  show_table(m_table_name);
}

void Frame_Glom::show_table_allow_empty(const Glib::ustring& table_name, const Gnome::Gda::Value& primary_key_value_for_details)
{
  auto pApp = dynamic_cast<AppWindow*>(get_app_window());

  //This can take quite a long time, so we show the busy cursor while it's working:
  BusyCursor busy_cursor(pApp);

  //Choose a default mode, if necessary:
  if(m_Mode == enumModes::NONE)
    set_mode(m_Mode);

  //Show the table:
  m_table_name = table_name;
#ifndef GLOM_ENABLE_CLIENT_ONLY
  //Update the document with any new information in the database if necessary (though the database _should never have changed information)
  update_table_in_document_from_database();
#endif // !GLOM_ENABLE_CLIENT_ONLY

  //Update user-level dependent UI:
  if(pApp)
  {
    on_userlevel_changed(pApp->get_userlevel());
    pApp->update_table_sensitive_ui();
  }

  switch(m_Mode)
  {
    case(enumModes::DATA):
    {
      FoundSet found_set;

      //Start with the last-used found set (sort order and where clause)
      //for this layout:
      //(This would be ignored anyway if a details primary key is specified.)
      auto document = get_document();
      if(document)
        found_set = document->get_criteria_current(m_table_name);

      //Make sure that this is set:
      found_set.m_table_name = m_table_name;

      //If there is no saved sort clause,
      //then sort by the ID, just so we sort by something, so that the order is predictable:
      if(found_set.m_sort_clause.empty())
      {
        std::shared_ptr<Field> field_primary_key = get_field_primary_key_for_table(m_table_name);
        if(field_primary_key)
        {
          std::shared_ptr<LayoutItem_Field> layout_item_sort = std::make_shared<LayoutItem_Field>();
          layout_item_sort->set_full_field_details(field_primary_key);

          found_set.m_sort_clause.clear();

          //Avoid the sort clause if the found set will include too many records,
          //because that would be too slow.
          //The user can explicitly request a sort later, by clicking on a column header.
          //TODO_Performance: This causes an almost-duplicate COUNT query (we do it in the treemodel too), but it's not that slow.
          std::shared_ptr<LayoutItem_Field> layout_item_temp = std::make_shared<LayoutItem_Field>();
          layout_item_temp->set_full_field_details(field_primary_key);
          type_vecLayoutFields layout_fields;
          layout_fields.push_back(layout_item_temp);
          Glib::RefPtr<Gnome::Gda::SqlBuilder> sql_query_without_sort = Utils::build_sql_select_with_where_clause(found_set.m_table_name, layout_fields, found_set.m_where_clause, found_set.m_extra_join, type_sort_clause());

          const Privileges table_privs = Privs::get_current_privs(found_set.m_table_name);
          int count = 0;
          if(table_privs.m_view) //Avoid the query if the user does not have view rights, because it would fail.
            count = DbUtils::count_rows_returned_by(sql_query_without_sort);
            
          if(count < 10000) //Arbitrary large number.
            found_set.m_sort_clause.push_back( type_pair_sort_field(layout_item_sort, true /* ascending */) );
        }
      }

      //Show the wanted records in the notebook, showing details for a particular record if wanted:
      m_Notebook_Data.init_db_details(found_set, primary_key_value_for_details);
      set_mode_widget(m_Notebook_Data);

      //Show how many records were found:
      update_records_count();

      break;
    }
    case(enumModes::FIND):
    {
      m_Notebook_Find.init_db_details(m_table_name, get_active_layout_platform(get_document()));
      set_mode_widget(m_Notebook_Find);
      break;
    }
    default:
    {
      std::cout << "debug: " << G_STRFUNC << ": Unexpected mode" << std::endl;
      break;
    }
  }

  show_table_title();

  //List the reports and print layouts in the menus:
  if(pApp)
  {
    pApp->fill_menu_reports(table_name);
    pApp->fill_menu_print_layouts(table_name);
  }

  //show_all();
}

void Frame_Glom::show_table(const Glib::ustring& table_name, const Gnome::Gda::Value& primary_key_value_for_details)
{
  //Check that there is a table to show:
  if(table_name.empty())
  {
    alert_no_table();
  }
  else
  {
    show_table_allow_empty(table_name, primary_key_value_for_details);
  }
}

void Frame_Glom::show_no_table()
{
  show_table_allow_empty(Glib::ustring());
}

#ifndef GLOM_ENABLE_CLIENT_ONLY
bool Frame_Glom::attempt_change_usermode_to_developer()
{
  auto document = dynamic_cast<Document*>(get_document());
  if(!document)
    return false;

  //Check whether the current user has developer privileges:
  auto connection_pool = ConnectionPool::get_instance();
      std::shared_ptr<SharedConnection> sharedconnection = connection_pool->connect();

  // Default to true; if we don't support users, we always have
  // priviliges to change things in developer mode.
  bool test = true;

  if(sharedconnection && sharedconnection->get_gda_connection()->supports_feature(Gnome::Gda::CONNECTION_FEATURE_USERS))
  {
    test = Privs::get_user_is_in_group(connection_pool->get_user(), GLOM_STANDARD_GROUP_NAME_DEVELOPER);
  }

  if(test)
  {
    std::cout << "DEBUG: User=" << connection_pool->get_user() << " _is_ in the developer group on the server." << std::endl;
    //Avoid double signals:
    //if(document->get_userlevel() != AppState::userlevels::DEVELOPER)
    test = document->set_userlevel(AppState::userlevels::DEVELOPER);
    if(!test)
      std::cout << "  DEBUG: But document->set_userlevel(AppState::userlevels::DEVELOPER) failed." << std::endl;
  }
  else
  {
    std::cout << "DEBUG: User=" << connection_pool->get_user() << " is _not_ in the developer group on the server." << std::endl;
  }

  //If this was not possible then revert the menu:
  if(!test)
  {
    if(document->get_opened_from_browse())
    {
      //TODO: Obviously this could be possible but it would require a network protocol and some work:
      Gtk::MessageDialog dialog(UiUtils::bold_message(_("Developer mode not available.")), true, Gtk::MESSAGE_WARNING);
      dialog.set_secondary_text(_("Developer mode is not available because the file was opened over the network from a running Glom. Only the original file may be edited."));
      dialog.set_transient_for(*get_app_window());
      dialog.run();
    }
    else
    {
      Gtk::MessageDialog dialog(UiUtils::bold_message(_("Developer mode not available")), true, Gtk::MESSAGE_WARNING);
      dialog.set_secondary_text(_("Developer mode is not available. Check that you have sufficient database access rights and that the glom file is not read-only."));
      dialog.set_transient_for(*get_app_window());
      dialog.run();
    }
  }
  else if(document->get_document_format_version() < Document::get_latest_known_document_format_version())
  {
    Gtk::MessageDialog dialog(UiUtils::bold_message(_("Saving in new document format")), true, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_NONE);
    dialog.set_secondary_text(_("The document was created by an earlier version of the application. Making changes to the document will mean that the document cannot be opened by some earlier versions of the application."));
    dialog.set_transient_for(*get_app_window());
    dialog.add_button(_("_Cancel"), Gtk::RESPONSE_CANCEL);
    dialog.add_button(_("Continue"), Gtk::RESPONSE_OK);
    const int response = dialog.run();
    test = (response == Gtk::RESPONSE_OK);
  }

  return test;
}

bool Frame_Glom::attempt_change_usermode_to_operator()
{
  auto document = dynamic_cast<Document*>(get_document());
  if(!document)
    return false;
    
  document->set_userlevel(AppState::userlevels::OPERATOR);

  return true;
}

void Frame_Glom::on_menu_file_export()
{
  //Start with a sequence based on the Details view:
  //The user can changed this by clicking the button in the FileChooser:
  auto document = get_document();
  if(!document)
    return;

  Document::type_list_layout_groups mapGroupSequence = document->get_data_layout_groups_plus_new_fields("details", m_table_name, get_active_layout_platform(document));

  auto pWindowApp = get_app_window();
  g_assert(pWindowApp);

  //Do not try to export the data if the user may not view it:
  Privileges table_privs = Privs::get_current_privs(m_table_name);
  if(!table_privs.m_view)
  {
    show_ok_dialog(_("Export Not Allowed."), _("You do not have permission to view the data in this table, so you may not export the data."), *pWindowApp, Gtk::MESSAGE_ERROR);
    return;
  }

  //Ask the user for the new file location, and to optionally modify the format:
  FileChooser_Export dialog;
  dialog.set_transient_for(*get_app_window());
  dialog.set_do_overwrite_confirmation();
  dialog.set_export_layout(mapGroupSequence, m_table_name, get_document());
  const int response = dialog.run();
  dialog.hide();

  if((response == Gtk::RESPONSE_CANCEL) || (response == Gtk::RESPONSE_DELETE_EVENT))
    return;

  std::string filepath = dialog.get_filename();
  if(filepath.empty())
    return;

  filepath = UiUtils::get_filepath_with_extension(filepath, "csv");

  dialog.get_layout_groups(mapGroupSequence);
  //std::cout << "DEBUG 0: mapGroupSequence.size()=" << mapGroupSequence.size() << std::endl;

  //const int index_primary_key = fieldsSequence.size() - 1;

  const auto found_set = m_Notebook_Data.get_found_set();

  std::fstream the_stream(filepath, std::ios_base::out | std::ios_base::trunc);
  if(!the_stream)
  {
    show_ok_dialog(_("Could Not Create File."), _("Glom could not create the specified file."), *pWindowApp, Gtk::MESSAGE_ERROR);
    return;
  }

  export_data_to_stream(the_stream, found_set, mapGroupSequence);
}

//TODO: Reduce copy/pasting in these export_data_to_*() methods:
void Frame_Glom::export_data_to_vector(Document::type_example_rows& the_vector, const FoundSet& found_set, const Document::type_list_layout_groups& sequence)
{
  type_vecConstLayoutFields fieldsSequence = get_table_fields_to_show_for_sequence(found_set.m_table_name, sequence);

  if(fieldsSequence.empty())
  {
    std::cerr << G_STRFUNC << ": No fields in sequence." << std::endl;
    return;
  }

  Glib::RefPtr<Gnome::Gda::SqlBuilder> query = Utils::build_sql_select_with_where_clause(found_set.m_table_name, fieldsSequence, found_set.m_where_clause, found_set.m_extra_join, found_set.m_sort_clause);

  //TODO: Lock the database (prevent changes) during export.
  Glib::RefPtr<Gnome::Gda::DataModel> result = DbUtils::query_execute_select(query);

  guint rows_count = 0;
  if(result)
    rows_count = result->get_n_rows();

  if(rows_count)
  {
    const guint columns_count = result->get_n_columns();

    for(guint row_index = 0; row_index < rows_count; ++row_index)
    {
        Document::type_row_data row_data;

        for(guint col_index = 0; col_index < columns_count; ++col_index)
        {
          const auto value = result->get_value_at(col_index, row_index);

          std::shared_ptr<const LayoutItem_Field> layout_item = fieldsSequence[col_index];
          //if(layout_item->m_field.get_glom_type() != Field::glom_field_type::IMAGE) //This is too much data.
          //{

            //Output data in canonical SQL format, ignoring the user's locale, and ignoring the layout formatting:
            row_data.push_back(value);  //TODO_Performance: reserve the size.

            //if(layout_item->m_field.get_glom_type() == Field::glom_field_type::IMAGE) //This is too much data.
            //{
             //std::cout << "  field name=" << layout_item->get_name() << ", value=" << layout_item->m_field.sql(value) << std::endl;
            //}
        }

        //std::cout << " row_string=" << row_string << std::endl;
        the_vector.push_back(row_data); //TODO_Performance: Reserve the size.
    }
  }
}

void Frame_Glom::export_data_to_stream(std::ostream& the_stream, const FoundSet& found_set, const Document::type_list_layout_groups& sequence)
{
  type_vecConstLayoutFields fieldsSequence = get_table_fields_to_show_for_sequence(found_set.m_table_name, sequence);

  if(fieldsSequence.empty())
  {
    std::cerr << G_STRFUNC << ": No fields in sequence." << std::endl;
    return;
  }

  Glib::RefPtr<Gnome::Gda::SqlBuilder> query = Utils::build_sql_select_with_where_clause(found_set.m_table_name, fieldsSequence, found_set.m_where_clause, found_set.m_extra_join, found_set.m_sort_clause);

  //TODO: Lock the database (prevent changes) during export.
  Glib::RefPtr<const Gnome::Gda::DataModel> result = DbUtils::query_execute_select(query);

  guint rows_count = 0;
  if(result)
    rows_count = result->get_n_rows();

  if(rows_count)
  {
    const guint columns_count = result->get_n_columns();

    for(guint row_index = 0; row_index < rows_count; ++row_index)
    {
        std::string row_string;

        for(guint col_index = 0; col_index < columns_count; ++col_index)
        {
          const auto value = result->get_value_at(col_index, row_index);

          std::shared_ptr<const LayoutItem_Field> layout_item = fieldsSequence[col_index];
          //if(layout_item->m_field.get_glom_type() != Field::glom_field_type::IMAGE) //This is too much data.
          //{
            if(!row_string.empty())
              row_string += ",";

            //Output data in canonical SQL format, ignoring the user's locale, and ignoring the layout formatting:
            std::shared_ptr<const Field> field = layout_item->get_full_field_details();
            if(!field)
            {
              std::cerr << G_STRFUNC << ": A field was null." << std::endl;
              return;
            }

            const auto field_text = field->to_file_format(value);

            if(layout_item->get_glom_type() == Field::glom_field_type::IMAGE) //This is too much data.
            {
              // Some extra debug checks,
              // though we believe that all these problems are now fixed in File::to_file_format():

              const char* newline_to_find = "\r\n";
              size_t pos = field_text.find_first_of(newline_to_find);
              if(pos != std::string::npos)
              {
                std::cerr << G_STRFUNC << ": export: binary data field text contains an unexpected newline: " << field_text << std::endl;
                continue;
              }

              const char* quote_to_find = "\"";
              pos = field_text.find_first_of(quote_to_find);
              if(pos != std::string::npos)
              {
                std::cerr << G_STRFUNC << ": export: binary data field text contains an unexpected quote: " << field_text << std::endl;
                continue;
              }
            }

            if(layout_item->get_glom_type() == Field::glom_field_type::TEXT)
            {
              //The CSV RFC says text may be quoted and should be if it has newlines:
              //TODO: Escape the text?
              row_string += ("\"" + field_text + "\"");
            }
            else
              row_string += field_text;


            //std::cout << "  field name=" << layout_item->get_name() << ", value=" << layout_item->m_field.sql(value) << std::endl;
          //}
        }

        //std::cout << " row_string=" << row_string << std::endl;
        the_stream << row_string << std::endl;
    }
  }
}

void Frame_Glom::on_menu_file_import()
{
  if(m_table_name.empty())
  {
    UiUtils::show_ok_dialog(_("No Table"), _("There is no table in to which data could be imported."), *get_app_window(), Gtk::MESSAGE_ERROR);
  }
  else
  {
    Gtk::FileChooserDialog file_chooser(*get_app_window(), _("Open CSV Document"), Gtk::FILE_CHOOSER_ACTION_OPEN);
    file_chooser.add_button(_("_Cancel"), Gtk::RESPONSE_CANCEL);
    file_chooser.add_button(_("_Open"), Gtk::RESPONSE_ACCEPT);
    Glib::RefPtr<Gtk::FileFilter> filter_csv = Gtk::FileFilter::create();
    filter_csv->set_name(_("CSV files"));
    filter_csv->add_mime_type("text/csv");
    file_chooser.add_filter(filter_csv);
    Glib::RefPtr<Gtk::FileFilter> filter_any = Gtk::FileFilter::create();
    filter_any->set_name(_("All files"));
    filter_any->add_pattern("*");
    file_chooser.add_filter(filter_any);

    if(file_chooser.run() == Gtk::RESPONSE_ACCEPT)
    {
      file_chooser.hide();

      Dialog_Import_CSV* dialog = nullptr;
      Glom::Utils::get_glade_widget_derived_with_warning(dialog);
      if(!dialog) //Unlikely and it already warns on stderr.
        return;

      add_view(dialog);

      dialog->import(file_chooser.get_uri(), m_table_name);
      while(Glom::UiUtils::dialog_run_with_help(dialog) == Gtk::RESPONSE_ACCEPT)
      {
        dialog->hide();

        Dialog_Import_CSV_Progress* progress_dialog = nullptr;
        Glom::Utils::get_glade_widget_derived_with_warning(progress_dialog);
        int response = Gtk::RESPONSE_OK;
        if(!progress_dialog)
        {
          std::cerr << G_STRFUNC << ": progress_dialog was null." << std::endl;
        }
        else
        {
          add_view(progress_dialog);

          progress_dialog->init_db_details(dialog->get_target_table_name());
          progress_dialog->import(*dialog);
          response = progress_dialog->run(); //TODO: Check response?

          remove_view(progress_dialog);
          delete progress_dialog;
          progress_dialog = nullptr;

          // Force update from database so the newly added entries are shown
          show_table_refresh();

          // Re-show chooser dialog when an error occured or when the user
          // cancelled.
          if(response == Gtk::RESPONSE_OK)
            break;
        }
      }

      remove_view(dialog);
      delete dialog;
    }
  }
}

bool Frame_Glom::attempt_toggle_shared(bool shared)
{
  //Prevent this change if not in developer mode,
  //though the menu item should be disabled then anyway.
  auto document = dynamic_cast<Document*>(get_document());
  if(!document || document->get_userlevel() != AppState::userlevels::DEVELOPER)
    return false;

  if(shared == document->get_network_shared())
  {
    //Do nothing, because things are already as requested.
    //This is probably just an extra signal emitted when we set the toggle in the UI.
    //So we avoid the endless loop:
    return false;
  }

  bool change = true;

  //Ask user for confirmation:
  //TODO: Warn that this will be saved as the default if doing this in developer mode?
  if(shared)
  {
    Gtk::MessageDialog dialog(UiUtils::bold_message(_("Share on the network")), true, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_NONE);
    dialog.set_secondary_text(_("This will allow other users on the network to use this database."));
    dialog.set_transient_for(*get_app_window());
    dialog.add_button(_("_Cancel"), Gtk::RESPONSE_CANCEL);
    dialog.add_button(_("_Share"), Gtk::RESPONSE_OK);

    const auto response = dialog.run();
    dialog.hide();
    if(response == Gtk::RESPONSE_OK)
    {
      shared = true;

      //Ask for a user/password if none is set:
      const auto real_user_exists = Privs::get_developer_user_exists_with_password(document->get_hosting_mode());
      if(!real_user_exists)
      {
        //Ask for an initial user:
        Glib::ustring user, password;
        const auto initial_password_provided = connection_request_initial_password(user, password);
        bool added = false;
        if(initial_password_provided)
          added = DbUtils::add_user(document, user, password, GLOM_STANDARD_GROUP_NAME_DEVELOPER);

        if(initial_password_provided && added)
        {
          //Use the new user/password from now on:
          auto connectionpool = ConnectionPool::get_instance();
          connectionpool->set_user(user);
          connectionpool->set_password(password);
        }
        else
        {
          shared = false;
          change = false;
        }
      }
      else
      {
        //Ask for the password of a developer user, to
        //a) Check that the user knows it, so he won't lose access.
        //b) Reconnect as that user so we can remove the default user.
        //TODO: Check that this user is a developer.
        bool database_not_found = false; //Ignored;
        const auto dev_password_known = connection_request_password_and_attempt(database_not_found, "" ,"", true /* alternative text */);
        if(!dev_password_known)
        {
          shared = false;
          change = false;
        }
      }

      if(change) //If nothing has gone wrong so far.
      {
        //Remove the default no-password user, because that would be a security hole:
        //We do this after adding/using the non-default user, because we can't
        //remove a currently-used user.
        const auto hosting_mode = document->get_hosting_mode();
        const auto default_user_exists = Privs::get_default_developer_user_exists(hosting_mode);
        if(default_user_exists)
        {
          //Force a reconnection with the new password:
          //auto connectionpool = ConnectionPool::get_instance();

          //Remove it, after stopping it from being the database owner:
          bool disabled = true;
          Glib::ustring default_password;
          const auto default_user = Privs::get_default_developer_user_name(default_password, hosting_mode);

          auto connectionpool = ConnectionPool::get_instance();
          const auto reowned = set_database_owner_user(connectionpool->get_user());
          bool removed = false;
          if(reowned)
            removed = DbUtils::remove_user(default_user);

          if(!removed)
          {
            //This is a workaround.
            //Try to revoke it instead.
            //TODO: Discover how to make remove_user() succeed.
            disabled = disable_user(default_user);
            if(disabled)
            {
              //This message should be reassuring if the user sees a previous error
              //about the default user not being removed.
              std::cout << G_STRFUNC << ": The default user could not be removed, but it has been disabled." << std::endl;
            }
          }

          if(!reowned || !(removed || disabled))
          {
            std::cerr << G_STRFUNC << ": Failed to reown and remove/revoke default user." << std::endl;
            shared = false;
            change = false;
          }
        }
      }
    }
    else
    {
      shared = false;
      change = false;
    }
  }
  else //not shared:
  {
    //TODO: Warn about connected users if possible.
    Gtk::MessageDialog dialog(UiUtils::bold_message(_("Stop sharing on the network")), true, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_NONE);
    dialog.set_secondary_text(_("This will prevent other users on the network from using this database."));
    dialog.set_transient_for(*get_app_window());
    dialog.add_button(_("_Cancel"), Gtk::RESPONSE_CANCEL);
    dialog.add_button(_("_Stop Sharing"), Gtk::RESPONSE_OK);

    const auto response = dialog.run();
    dialog.hide();
    if(response == Gtk::RESPONSE_OK)
    {
      shared = false;

      //Make sure the default no-password user exists:
      const auto hosting_mode = document->get_hosting_mode();
      const auto default_user_exists = Privs::get_default_developer_user_exists(hosting_mode);
      if(!default_user_exists)
      {
        //Add it:
        Glib::ustring default_password;
        const auto default_user = Privs::get_default_developer_user_name(default_password, hosting_mode);

        const auto added = DbUtils::add_user(document, default_user, default_password, GLOM_STANDARD_GROUP_NAME_DEVELOPER);
        if(!added)
        {
           shared = true;
           change = false;
        }
      }
    }
    else
    {
      shared = true;
      change = false;
    }
  }

  if(document)
    document->set_network_shared(shared);


  //Stop the self-hosted database server,
  //change its configuration,
  //and start it again:
  if(change)
  {
    auto connectionpool = ConnectionPool::get_instance();
    std::shared_ptr<SharedConnection> sharedconnection = connectionpool->connect();
    if(sharedconnection)
    {
      sharedconnection->close();
      sharedconnection.reset();
    }

    ShowProgressMessage cleanup_message(_("Stopping Database Server"));
    connectionpool->cleanup (sigc::mem_fun(*this, &Frame_Glom::on_connection_cleanup_progress) );

    ShowProgressMessage startup_message(_("Starting Database Server"));
    connectionpool->set_network_shared(sigc::mem_fun(*this, &Frame_Glom::on_connection_startup_progress), shared);
    const ConnectionPool::StartupErrors started =
      connectionpool->startup( sigc::mem_fun(*this, &Frame_Glom::on_connection_startup_progress) );
    if(started != ConnectionPool::Backend::StartupErrors::NONE)
    {
      //TODO: Output more exact details of the error message.
      //TODO: Recover somehow?
      std::cerr << G_STRFUNC << ": startup() failed." << std::endl;
      return false;
    }

    connectionpool->set_ready_to_connect();
  }

  //Update the UI:
  auto pApp = dynamic_cast<AppWindow*>(get_app_window());
  if(pApp)
  {
    pApp->update_network_shared_ui();
  }

  return true;
}
#endif // !GLOM_ENABLE_CLIENT_ONLY

void Frame_Glom::on_menu_file_print()
{
 auto notebook_current = dynamic_cast<Notebook_Glom*>(m_stack_mode->get_visible_child());
 if(notebook_current)
   notebook_current->do_menu_file_print();
}

#ifndef GLOM_ENABLE_CLIENT_ONLY
void Frame_Glom::on_menu_file_print_edit_layouts()
{
  on_menu_developer_print_layouts();
}

void Frame_Glom::set_enable_layout_drag_and_drop(bool enable)
{
  m_Notebook_Data.set_enable_layout_drag_and_drop(enable);
}

#endif // !GLOM_ENABLE_CLIENT_ONLY

void Frame_Glom::set_mode_find()
{
  //Switch to find mode.
  //This can take quite a long time, flicking between 1 or 2 intermediate screens.
  //It shouldn't, but until we fix that, let's show the busy cursor while it's working:
  BusyCursor busy_cursor(get_app_window());

  const bool previously_in_data_mode = (m_Mode == enumModes::DATA);

  const auto list_or_details = m_Notebook_Data.get_current_view();

  //A workaround hack to make sure that the list view will be active when the results are shown.
  //Because the list doesn't refresh properly (to give the first result) when the Details view was active first.
  //murrayc.
  if(previously_in_data_mode && (list_or_details == Notebook_Data::dataview::DETAILS))
    m_Notebook_Data.set_current_view(Notebook_Data::dataview::LIST);

  if(!set_mode(enumModes::FIND))
    return;

  show_table(m_table_name);

  if(previously_in_data_mode)
  {
    //Show the same layout in Find mode as was just being viewed in Data mode:
    m_Notebook_Find.set_current_view(list_or_details);
  }
}

void Frame_Glom::set_mode_data()
{
  //Switch to data mode
  if(!set_mode(enumModes::DATA))
    return;

  //This would lose the current found set, if any:
  //show_table(m_table_name);
}


void Frame_Glom::on_menu_add_record()
{
  BusyCursor busy_cursor(get_app_window());

  //Note: This should only be called in Data mode.
  m_Notebook_Data.do_menu_file_add_record();
}


#ifndef GLOM_ENABLE_CLIENT_ONLY
void Frame_Glom::on_menu_Reports_EditReports()
{
  on_menu_developer_reports();
}

void Frame_Glom::on_menu_Tables_EditTables()
{
  do_menu_Navigate_Table();
}

/* Commented out because it is useful but confusing to new users:
void Frame_Glom::on_menu_Tables_AddRelatedTable()
{
  //Delete and recreate the dialog,
  //so we start with a blank one:
  if(m_dialog_addrelatedtable)
  {
    remove_view(m_dialog_addrelatedtable);
    delete m_dialog_addrelatedtable;
    m_dialog_addrelatedtable = nullptr;
  }

  Utils::get_glade_widget_derived_with_warning(m_dialog_addrelatedtable);
  if(!m_dialog_addrelatedtable)
    return;

  add_view(m_dialog_addrelatedtable); //Give it access to the document.
  m_dialog_addrelatedtable->set_fields(m_table_name);

  m_dialog_addrelatedtable->signal_request_edit_fields().connect( sigc::mem_fun(*this, &Frame_Glom::on_dialog_add_related_table_request_edit_fields) );

  m_dialog_addrelatedtable->signal_response().connect( sigc::mem_fun(*this, &Frame_Glom::on_dialog_add_related_table_response) );

  auto parent = get_app_window();

  if(parent)
    m_dialog_addrelatedtable->set_transient_for(*parent);

  m_dialog_addrelatedtable->set_modal(); //We don't want people to edit the main window while we are changing structure.
  m_dialog_addrelatedtable->show();
}
*/

#endif

#ifndef GLOM_ENABLE_CLIENT_ONLY

void Frame_Glom::on_dialog_add_related_table_response(int response)
{
  if(!m_dialog_addrelatedtable)
    return;

  m_dialog_addrelatedtable->hide();

  if(response == Gtk::RESPONSE_OK)
  {
    bool stop_trying = false;

    Glib::ustring table_name, relationship_name, from_key_name;
    m_dialog_addrelatedtable->get_input(table_name, relationship_name, from_key_name);

    auto parent = get_app_window();

    //It would be nice to put this in the dialog's on_response() instead,
    //but I don't think we can stop the response from being returned. murrayc
    if(DbUtils::get_table_exists_in_database(table_name))
    {
      Frame_Glom::show_ok_dialog(_("Table Exists Already"), _("A table with this name already exists in the database. Please choose a different table name."), *parent, Gtk::MESSAGE_ERROR);
    }
    else if(get_relationship_exists(m_table_name, relationship_name))
    {
      Frame_Glom::show_ok_dialog(_("Relationship Exists Already"), _("A relationship with this name already exists for this table. Please choose a different relationship name."), *parent, Gtk::MESSAGE_ERROR);
    }
    else if(table_name.empty() || relationship_name.empty())
    {
      Frame_Glom::show_ok_dialog(_("More information needed"), _("You must specify a field, a table name, and a relationship name."), *parent, Gtk::MESSAGE_ERROR);
    }
    else
    {
      stop_trying = true;
    }

    if(!stop_trying)
    {
      //Offer the dialog again:
      //This signal handler should be called again when a button is clicked.
      m_dialog_addrelatedtable->show();
    }
    else
    {
      //Create the new table:
      const auto result = DbUtils::create_table_with_default_fields(get_document(), table_name);
      if(!result)
      {
        std::cerr << G_STRFUNC << ": create_table_with_default_fields() failed." << std::endl;
        return;
      }

      //Create the new relationship:
      std::shared_ptr<Relationship> relationship = std::make_shared<Relationship>();

      relationship->set_name(relationship_name);
      relationship->set_title(Utils::title_from_string(relationship_name), AppWindow::get_current_locale());
      relationship->set_from_table(m_table_name);
      relationship->set_from_field(from_key_name);
      relationship->set_to_table(table_name);

      std::shared_ptr<Field> related_primary_key = get_field_primary_key_for_table(table_name); //This field was created by create_table_with_default_fields().
      if(!related_primary_key)
      {
        std::cerr << G_STRFUNC << ": get_field_primary_key_for_table() failed." << std::endl;
        return;
      }

      relationship->set_to_field(related_primary_key->get_name());

      relationship->set_allow_edit(true);
      relationship->set_auto_create(true);

      auto document = get_document();
      if(!document)
        return;

      document->set_relationship(m_table_name, relationship);

      on_dialog_tables_hide(); //Update the menu.

      if(parent)
        show_ok_dialog(_("Related Table Created"), _("The new related table has been created."), *parent, Gtk::MESSAGE_INFO);
    }
  }
}

void Frame_Glom::on_dialog_add_related_table_request_edit_fields()
{
  if(m_dialog_addrelatedtable)
    do_menu_developer_fields(*m_dialog_addrelatedtable);
}
#endif // !GLOM_ENABLE_CLIENT_ONLY

void Frame_Glom::do_menu_Navigate_Table(bool open_default)
{

  if(get_document()->get_connection_database().empty())
  {
    alert_no_table();
    return;
  }

  Glib::ustring default_table_name;
  if(open_default)
    default_table_name = get_document()->get_default_table();

  //Create the dialog, if it has not already been created:
  if(!m_pBox_Tables)
  {
    const Glib::RefPtr<Gtk::Builder> builderToKeepWidgetAlive =
      Utils::get_glade_child_widget_derived_with_warning(m_pBox_Tables);
    m_pDialog_Tables = new Window_BoxHolder(m_pBox_Tables, _("Edit Tables"));
    m_pDialog_Tables->signal_hide().connect(sigc::mem_fun(*this, &Frame_Glom::on_dialog_tables_hide));

    auto pWindow = get_app_window();
    if(pWindow)
      m_pDialog_Tables->set_transient_for(*pWindow);

    m_pDialog_Tables->set_default_size(300, 400);
    m_pBox_Tables->show_all();
    add_view(m_pBox_Tables);

    //Connect signals:
    m_pBox_Tables->signal_selected.connect(sigc::mem_fun(*this, &Frame_Glom::on_box_tables_selected));
  }

  {
    BusyCursor busy_cursor(get_app_window());
    m_pBox_Tables->init_db_details();
  }

  //Let the user choose a table:
  //m_pDialog_Tables->set_policy(false, true, false); //TODO_port
  //m_pDialog_Tables->load_from_document(); //Refresh
  if(!default_table_name.empty())
  {
    //Show the default table, and let the user navigate to another table manually if he wants:
    show_table(default_table_name);
  }
  else
  {
    m_pDialog_Tables->show();
  }
}

const Gtk::Window* Frame_Glom::get_app_window() const
{
  auto nonconst = const_cast<Frame_Glom*>(this);
  return nonconst->get_app_window();
}

Gtk::Window* Frame_Glom::get_app_window()
{
  auto pWidget = get_parent();
  while(pWidget)
  {
    //Is this widget a Gtk::Window?:
    auto pWindow = dynamic_cast<Gtk::Window*>(pWidget);
    if(pWindow)
    {
      //Yes, return it.
      return pWindow;
    }
    else
    {
      //Try the parent's parent:
      pWidget = pWidget->get_parent();
    }
  }

  return nullptr; //not found.

}

void Frame_Glom::show_ok_dialog(const Glib::ustring& title, const Glib::ustring& message, Gtk::Window& parent, Gtk::MessageType message_type)
{
  UiUtils::show_ok_dialog(title, message, parent, message_type);
}

void Frame_Glom::on_button_quickfind()
{
  //This will get the criteria for the quick find:
  on_notebook_find_criteria(Gnome::Gda::SqlExpr());
}

void Frame_Glom::on_notebook_find_criteria(const Gnome::Gda::SqlExpr& where_clause)
{
  auto app = dynamic_cast<AppWindow*>(get_app_window());
  if(!app)
  {
    std::cerr << G_STRFUNC << ": get_app_window() failed." << std::endl;
    return;
  }

  //Identify the where_clause:
  Gnome::Gda::SqlExpr where_clause_to_use = where_clause;

  //Prefer the quick find text if any was entered:
  const auto quickfind_criteria = m_pEntry_QuickFind->get_text();
  if(!quickfind_criteria.empty())
  {
    where_clause_to_use = 
      Utils::get_find_where_clause_quick(get_document(), m_table_name, Gnome::Gda::Value(quickfind_criteria));
  }

  //Warn if there was no find criteria:
  if(where_clause_to_use.empty())
  {
    const Glib::ustring message = _("You have not entered any find criteria. Try entering information in the fields.");

    Gtk::MessageDialog dialog(UiUtils::bold_message(_("No Find Criteria")), true, Gtk::MESSAGE_WARNING );
    dialog.set_secondary_text(message);
    dialog.set_transient_for(*app);
    dialog.run();
    return;
  }

  //std::cout << "debug: " << G_STRFUNC << ": " << where_clause << std::endl;
  

  //Try to find some records with the where_clause:
  bool records_found = false;

  { //Extra scope, to control the lifetime of the busy cursor.
    BusyCursor busy_cursor(app);

    //std::cout << "Frame_Glom::on_notebook_find_criteria: where_clause=" << where_clause << std::endl;
    FoundSet found_set;
    found_set.m_table_name = m_table_name;
    found_set.m_where_clause = where_clause_to_use;
    const auto inited = m_Notebook_Data.init_db_details(found_set);

    m_Notebook_Data.select_page_for_find_results();

    //Show how many records were found:
    records_found = (update_records_count() > 0);

    if(!inited)
      records_found = 0;
  }

  std::cout << G_STRFUNC << ": records_found=" << records_found << std::endl;

  if(!records_found)
  {
    const auto find_again = UiUtils::show_warning_no_records_found(*app);

    if(!find_again)
    {
      //Go back to data mode, showing all records:
      on_button_find_all();
      app->set_mode_data();
    }
  }
  else
  {
    //Actually show the found data,
    //and show that we are in data mode:
    app->set_mode_data();
  }
}

void Frame_Glom::on_userlevel_changed(AppState::userlevels /* userlevel */)
{
  //show user level in the window title:
  show_table_title();
}

void Frame_Glom::show_table_title()
{
  auto document = dynamic_cast<Document*>(get_document());
  if(!document)
    return;

  //Show the table title:
  Glib::ustring table_label = document->get_table_title(m_table_name, AppWindow::get_current_locale());
  if(!table_label.empty())
  {
    if(document->get_userlevel() == AppState::userlevels::DEVELOPER)
      table_label += " (" + m_table_name + ")"; //Show the table name as well, if in developer mode.
  }
  else //Use the table name if there is no table title.
    table_label = m_table_name;

  //Show the table title in bold text, because it's important to the user.
  const auto title = UiUtils::bold_message(table_label);
  m_pLabel_Table_DataMode->set_markup(title);
  m_pLabel_Table_FindMode->set_markup(title);
}

#ifndef GLOM_ENABLE_CLIENT_ONLY
void Frame_Glom::update_table_in_document_from_database()
{
  //Add any new/changed information from the database to the document
  //The database should never change without the knowledge of the document anyway, so this should be unnecessary.

  //TODO_performance: There are a lot of temporary Field and Column instances here, with a lot of string copying.

  //For instance, changed field details, or new fields, or removed fields.
  typedef Box_DB_Table::type_vec_fields type_vec_fields;

  //Get the fields information from the database:
  DbUtils::type_vec_fields fieldsDatabase = DbUtils::get_fields_for_table_from_database(m_table_name);
  if(fieldsDatabase.empty())
  {
    std::cerr << G_STRFUNC << ": Could not get the list of fields for table=" << m_table_name <<
      " from the database. This might be due to insufficient database user rights." << 
      " Falling back to the field details in the document." << std::endl;
  }

  auto pDoc = dynamic_cast<Document*>(get_document());
  if(pDoc)
  {
    bool document_must_be_updated = false;

    //Get the fields information from the document.
    //and add to, or update Document's list of fields:
    type_vec_fields fieldsDocument = pDoc->get_table_fields(m_table_name);

    for(const auto& field_database : fieldsDatabase)
    {
      if(field_database)
      {
        //Is the field already in the document?
        auto iterFindDoc = find_if_same_name(fieldsDocument, field_database->get_name());
        if(iterFindDoc == fieldsDocument.end()) //If it was not found:
        {
          //Add it
          fieldsDocument.push_back(field_database);
          document_must_be_updated = true;
        }
        else //if it was found.
        {
          //Compare the information:
          Glib::RefPtr<Gnome::Gda::Column> field_info_db = field_database->get_field_info();
          std::shared_ptr<Field> field_document =  *iterFindDoc;
          if(field_document)
          {
            if(!field_document->field_info_from_database_is_equal( field_info_db )) //ignores auto_increment because libgda does not report it from the database properly.
            {
              //The database has different information. We assume that the information in the database is newer.

              //Update the field information:

              // Ignore things that libgda does not report from the database properly:
              // We do this also in Field::field_info_from_database_is_equal() and
              // Base_DB::get_fields_for_table(),
              // so make sure that we ignore the same things everywhere always
              // TODO: Avoid that duplication?
              field_info_db->set_auto_increment( field_document->get_auto_increment() );
              field_info_db->set_default_value( field_document->get_default_value() );

              (*iterFindDoc)->set_field_info( field_info_db );

              document_must_be_updated = true;
            }
          }
        }
      }
    }

    //Remove fields that are no longer in the database:
    //TODO_performance: This is incredibly inefficient - but it's difficut to erase() items while iterating over them.
    if(!fieldsDatabase.empty()) //Do not do this if getting the fields from the database failed completely, probably due to permissions.
    {
      type_vec_fields fieldsActual;
      for(const auto& field : fieldsDocument)
      {
        //Check whether it's in the database:
        auto iterFindDatabase = find_if_same_name(fieldsDatabase, field->get_name());
        if(iterFindDatabase != fieldsDatabase.end()) //If it was found
        {
          fieldsActual.push_back(field);
        }
        else
        {
          document_must_be_updated = true; //Something changed.
        }
      }

      if(document_must_be_updated)
        pDoc->set_table_fields(m_table_name, fieldsActual);
    }
  }
}
#endif // !GLOM_ENABLE_CLIENT_ONLY

void Frame_Glom::set_document(Document* pDocument)
{
  View_Composite_Glom::set_document(pDocument);

  auto document = get_document();
  if(document)
  {
    //Connect to a signal that is only on the derived document class:
    document->signal_userlevel_changed().connect( sigc::mem_fun(*this, &Frame_Glom::on_userlevel_changed) );

    //Show the appropriate UI for the user level that is specified by this new document:
    on_userlevel_changed(document->get_userlevel());
  }
}

void Frame_Glom::load_from_document()
{
  auto document = dynamic_cast<Document*>(get_document());
  if(document)
  {
    //Call base class:
    View_Composite_Glom::load_from_document();
  }
}

#ifndef GLOM_ENABLE_CLIENT_ONLY
void Frame_Glom::on_menu_developer_database_preferences()
{
  Dialog_Database_Preferences* dialog = nullptr;
  Utils::get_glade_widget_derived_with_warning(dialog);
  if(!dialog) //Unlikely and it already warns on stderr.
    return;
        
  dialog->set_transient_for(*(get_app_window()));
  add_view(dialog);
  dialog->load_from_document();

  Glom::UiUtils::dialog_run_with_help(dialog);

  remove_view(dialog);
  delete dialog;

  //show_system_name(); //In case it has changed.
}

void Frame_Glom::on_menu_developer_fields()
{
  auto parent = get_app_window();
  if(parent)
    do_menu_developer_fields(*parent);

}

void Frame_Glom::do_menu_developer_fields(Gtk::Window& parent, const Glib::ustring table_name)
{
  if(!m_pDialog_Fields)
  {
    Utils::get_glade_widget_derived_with_warning(m_pDialog_Fields);
    m_pDialog_Fields->signal_hide().connect( sigc::mem_fun(*this, &Frame_Glom::on_developer_dialog_hide));
    add_view(m_pDialog_Fields);
  }

  // Some database backends (SQLite) require the table to change to no longer
  // be in use when changing the records, so we stop the database usage
  // here. We reshow everything in on_developer_dialog_hide() anyway.
  auto document = dynamic_cast<Document*>(get_document());
  if(document && document->get_hosting_mode() == Document::HostingMode::SQLITE)
    show_no_table();

  // Remember the old table name so that we re-show the previous table as
  // soon as the dialog has been closed.
  m_table_name = table_name;

  m_pDialog_Fields->set_transient_for(parent);
  m_pDialog_Fields->init_db_details(table_name);
  m_pDialog_Fields->show();
}

void Frame_Glom::do_menu_developer_fields(Gtk::Window& parent)
{
  //Check that there is a table to show:
  if(m_table_name.empty())
    return;

  do_menu_developer_fields(parent, m_table_name);
}


void Frame_Glom::on_menu_developer_relationships_overview()
{
  if(!m_window_relationships_overview)
  {
    Utils::get_glade_widget_derived_with_warning(m_window_relationships_overview);
    add_view(m_window_relationships_overview);

    if(m_window_relationships_overview)
    {
      m_window_relationships_overview->signal_hide().connect( sigc::mem_fun(*this, &Frame_Glom::on_developer_dialog_hide));
      add_window_to_app(m_window_relationships_overview);
    }
  }

  if(m_window_relationships_overview)
  {
    m_window_relationships_overview->set_transient_for(*(get_app_window()));
    m_window_relationships_overview->load_from_document();

    m_window_relationships_overview->show();
  }
}

void Frame_Glom::do_menu_developer_relationships(Gtk::Window& parent, const Glib::ustring table_name)
{
  //Create the widget if necessary:
  if(!m_pDialog_Relationships)
  {
    Utils::get_glade_widget_derived_with_warning(m_pDialog_Relationships);
    if(!m_pDialog_Relationships)
    {
      std::cerr << G_STRFUNC << ": m_pDialog_Relationships is null." << std::endl;
      return;
    }
    
    m_pDialog_Relationships->set_title("Relationships");
    m_pDialog_Relationships->signal_hide().connect( sigc::mem_fun(*this, &Frame_Glom::on_developer_dialog_hide));
    add_view(m_pDialog_Relationships); //Also a composite view.
  }

  m_pDialog_Relationships->set_transient_for(parent);
  m_pDialog_Relationships->init_db_details(table_name);
  m_pDialog_Relationships->show();
}

void Frame_Glom::on_menu_developer_relationships()
{
  //Check that there is a table to show:
  if(m_table_name.empty())
    return;

  auto app = get_app_window();
  if(app)
    do_menu_developer_relationships(*app, m_table_name);
}

void Frame_Glom::on_menu_developer_users()
{
  Dialog_GroupsList* dialog = nullptr;
  Utils::get_glade_widget_derived_with_warning(dialog);
  if(!dialog) //Unlikely and it already warns on stderr.
    return;
    
  dialog->set_transient_for(*get_app_window());

  add_view(dialog); //Give it access to the document.
  dialog->load_from_document(); //Update the UI now that it has the document.

  Glom::UiUtils::dialog_run_with_help(dialog);
  remove_view(dialog);
  delete dialog;

  //Update the Details and List layouts, in case the permissions have changed:
  //TODO: Also update them somehow if another user has changed them,
  //or respond to the failed SQL nicely.
  show_table(m_table_name);
}

void Frame_Glom::on_menu_developer_layout()
{
  //Check that there is a table to show:
  if(m_table_name.empty())
    return;

  auto notebook_current = dynamic_cast<Notebook_Glom*>(m_stack_mode->get_visible_child());
  if(notebook_current)
    notebook_current->do_menu_developer_layout();
}

void Frame_Glom::on_menu_developer_reports()
{
  //Check that there is a table to show:
  if(m_table_name.empty())
    return;

  //Create the widget if necessary:
  if(!m_pBox_Reports)
  {
    const Glib::RefPtr<Gtk::Builder> builderToKeepWidgetAlive =
      Utils::get_glade_child_widget_derived_with_warning(m_pBox_Reports);
    if(!m_pBox_Reports)
    {
      std::cerr << G_STRFUNC << ": m_pBox_Reports is null." << std::endl;
      return;
    }

    m_pDialog_Reports = new Window_BoxHolder(m_pBox_Reports);
    m_pDialog_Reports->set_transient_for(*(get_app_window()));
    m_pDialog_Reports->set_title(_("Reports"));

    Utils::get_glade_widget_derived_with_warning(m_pDialogLayoutReport);
    if(!m_pDialogLayoutReport)
    {
      std::cerr << G_STRFUNC << ": m_pDialogLayoutReport is null." << std::endl;
      return;
    }

    add_view(m_pDialogLayoutReport);
    m_pDialogLayoutReport->set_transient_for(*(get_app_window()));
    m_pDialogLayoutReport->signal_hide().connect( sigc::mem_fun(*this, &Frame_Glom::on_dialog_layout_report_hide) );

    m_pDialog_Reports->set_default_size(300, 400);
    m_pBox_Reports->show_all();

    m_pBox_Reports->signal_selected.connect(sigc::mem_fun(*this, &Frame_Glom::on_box_reports_selected));
    add_view(m_pBox_Reports);
  }

  m_pBox_Reports->init_db_details(m_table_name);
  m_pDialog_Reports->show();
}

void Frame_Glom::on_menu_developer_print_layouts()
{
  //Check that there is a table to show:
  if(m_table_name.empty())
    return;

  //Create the widget if necessary:
  if(!m_pBox_PrintLayouts)
  {
    const Glib::RefPtr<Gtk::Builder> builderToKeepWidgetAlive =
      Utils::get_glade_child_widget_derived_with_warning(m_pBox_PrintLayouts);
    if(!m_pBox_PrintLayouts)
    {
      std::cerr << G_STRFUNC << ": m_pBox_PrintLayouts is null." << std::endl;
      return;
    }

    m_pDialog_PrintLayouts = new Window_BoxHolder(m_pBox_PrintLayouts);

    m_pDialog_PrintLayouts->set_transient_for(*get_app_window());
    m_pDialog_PrintLayouts->set_title(_("Print Layouts"));
    m_pDialog_PrintLayouts->set_default_size(300, 400);
    m_pBox_PrintLayouts->show_all();
    add_view(m_pBox_PrintLayouts);

    m_pBox_PrintLayouts->signal_selected.connect(sigc::mem_fun(*this, &Frame_Glom::on_box_print_layouts_selected));
  }

  m_pBox_PrintLayouts->init_db_details(m_table_name);
  m_pDialog_PrintLayouts->show();
}

void Frame_Glom::on_menu_developer_script_library()
{
  Dialog_ScriptLibrary* dialog = nullptr;
  Utils::get_glade_widget_derived_with_warning(dialog);
  if(!dialog) //Unlikely and it already warns on stderr.
    return;
    
  dialog->set_transient_for(*(get_app_window()));
  add_view(dialog); //Give it access to the document.
  dialog->load_from_document();
  Glom::UiUtils::dialog_run_with_help(dialog); //TODO: Create the help section.
  dialog->save_to_document();
  remove_view(dialog);
  delete dialog;
}

void Frame_Glom::on_box_reports_selected(const Glib::ustring& report_name)
{
  m_pDialog_Reports->hide();

  std::shared_ptr<Report> report = get_document()->get_report(m_table_name, report_name);
  if(report)
  {
    m_pDialogLayoutReport->set_transient_for(*get_app_window());
    m_pDialogLayoutReport->set_report(m_table_name, report);
    m_pDialogLayoutReport->show();
  }
}

void Frame_Glom::add_window_to_app(Gtk::ApplicationWindow* window)
{
  if(!window)
  {
    std::cerr << G_STRFUNC << ": window is null." << std::endl;
    return;
  }

  auto app_window = get_app_window();
  if(!app_window)
  {
    std::cerr << G_STRFUNC << ": app_window is null" << std::endl;
    return;
  }

  //This probably lets the GtkApplication know about the window's actions, which might be useful.
  Glib::RefPtr<Gtk::Application> app = app_window->get_application();
  if(app)
    app->add_window(*window);
  else
  {
    std::cerr << G_STRFUNC << ": app is null." << std::endl;
  }
}

void Frame_Glom::on_box_print_layouts_selected(const Glib::ustring& print_layout_name)
{
  auto app_window = get_app_window();
  if(!app_window)
  {
    std::cerr << G_STRFUNC << ": app_window is null" << std::endl;
    return;
  }

  //Create the dialog if necessary:
  if(!m_pDialogLayoutPrint)
  {
    Utils::get_glade_widget_derived_with_warning(m_pDialogLayoutPrint);
    if(!m_pDialogLayoutPrint)
    {
      std::cerr << G_STRFUNC << ": m_pDialogLayoutPrint is null" << std::endl;
      return;
    }

    add_view(m_pDialogLayoutPrint);
    m_pDialogLayoutPrint->signal_hide().connect( sigc::mem_fun(*this, &Frame_Glom::on_dialog_layout_print_hide) );

    add_window_to_app(m_pDialogLayoutPrint);
  }

  m_pDialog_PrintLayouts->hide();

  std::shared_ptr<PrintLayout> print_layout = get_document()->get_print_layout(m_table_name, print_layout_name);
  if(print_layout)
  {
    m_pDialogLayoutPrint->set_transient_for(*app_window);

    m_pDialogLayoutPrint->set_print_layout(m_table_name, print_layout);

    m_pDialogLayoutPrint->show();
  }
}

void Frame_Glom::on_developer_dialog_hide()
{
  //The dababase structure might have changed, so refresh the data view:
  show_table(m_table_name);

  //TODO: This is a bit of a hack. It's not always useful to do this:
  if(m_dialog_addrelatedtable)
    m_dialog_addrelatedtable->set_fields(m_table_name);

  //Update the display. TODO: Shouldn't this happen automatically via the view?
  if(m_window_relationships_overview)
    m_window_relationships_overview->load_from_document();
}
#endif // !GLOM_ENABLE_CLIENT_ONLY

#ifndef GLOM_ENABLE_CLIENT_ONLY
void Frame_Glom::on_connection_initialize_progress()
{
  AppWindow *app = dynamic_cast<AppWindow*>(AppWindow::get_appwindow());
  if(app)
    app->pulse_progress_message();
}
#endif //GLOM_ENABLE_CLIENT_ONLY

void Frame_Glom::on_connection_startup_progress()
{
  AppWindow *app = dynamic_cast<AppWindow*>(AppWindow::get_appwindow());
  if(app)
    app->pulse_progress_message();
}

void Frame_Glom::on_connection_cleanup_progress()
{
  AppWindow *app = dynamic_cast<AppWindow*>(AppWindow::get_appwindow());
  if(app)
    app->pulse_progress_message();
}

bool Frame_Glom::handle_connection_initialize_errors(ConnectionPool::InitErrors error)
{
  Glib::ustring title;
  Glib::ustring message;

  if(error == ConnectionPool::Backend::InitErrors::NONE)
    return true;
  else if(error == ConnectionPool::Backend::InitErrors::DIRECTORY_ALREADY_EXISTS)
  {
    title = _("Directory Already Exists");
    message = _("There is an existing directory with the same name as the directory that should be created for the new database files. You should specify a different filename to use a new directory instead.");
  }
  else if(error == ConnectionPool::Backend::InitErrors::COULD_NOT_CREATE_DIRECTORY)
  {
    title = _("Could Not Create Directory");
    message = _("There was an error when attempting to create the directory for the new database files.");
  }
  else if(error == ConnectionPool::Backend::InitErrors::COULD_NOT_START_SERVER)
  {
    title = _("Could Not Start Database Server");
    message = _("There was an error when attempting to start the database server.");
  }

  UiUtils::show_ok_dialog(title, message, *get_app_window(), Gtk::MESSAGE_ERROR);

  return false;
}

#ifndef GLOM_ENABLE_CLIENT_ONLY
bool Frame_Glom::connection_request_initial_password(Glib::ustring& user, Glib::ustring& password)
{
  //Intialze output parameters:
  user = Glib::ustring();
  password = Glib::ustring();

  auto document = dynamic_cast<Document*>(get_document());
  if(!document)
    return false;

  //This is only useful for self-hosted postgres:
  if(document->get_hosting_mode() != Document::HostingMode::POSTGRES_SELF)
    return false;

  //Ask for a new username and password to specify when creating a new self-hosted database.
  Dialog_InitialPassword* dialog = nullptr;
  Utils::get_glade_widget_derived_with_warning(dialog);
  if(!dialog)
    return false;

  add_view(dialog);


  int response = Gtk::RESPONSE_OK;
  bool keep_trying = true;
  while(keep_trying)
  {
    response = UiUtils::dialog_run_with_help(dialog);

    //Check the password is acceptable:
    if(response == Gtk::RESPONSE_OK)
    {
      const auto password_ok = dialog->check_password();
      if(password_ok)
      {
        user = dialog->get_user();
        password = dialog->get_password();

        keep_trying = false; //Everything is OK.
      }
    }
    else
    {
      keep_trying = false; //The user cancelled.
    }

    dialog->hide();
  }

  remove_view(dialog);
  delete dialog;
  dialog = nullptr;

  return (response == Gtk::RESPONSE_OK);
}

bool Frame_Glom::connection_request_password_and_choose_new_database_name()
{
  auto document = dynamic_cast<Document*>(get_document());
  if(!document)
    return false;

  auto connection_pool = ConnectionPool::get_instance();
  connection_pool->setup_from_document(document);

  const auto hosting_mode = document->get_hosting_mode();
  switch(hosting_mode)
  {
    case Document::HostingMode::POSTGRES_SELF:
    case Document::HostingMode::MYSQL_SELF:
    {
      Glib::ustring user, password;

      if(document->get_network_shared()) //Usually not the case when creating new documents.
      {
        const auto test = connection_request_initial_password(user, password);
        if(!test)
          return false;
      }
      else
      {
        //Use the default user because we are not network shared:
        user = Privs::get_default_developer_user_name(password, hosting_mode);
      }

      // Create the requested self-hosting database:

      //Set the connection details in the ConnectionPool singleton.
      //The ConnectionPool will now use these every time it tries to connect.
      connection_pool->set_user(user);
      connection_pool->set_password(password);

      ShowProgressMessage progress_message(_("Initializing Database Data"));
      const auto initialized = handle_connection_initialize_errors( connection_pool->initialize(
         sigc::mem_fun(*this, &Frame_Glom::on_connection_initialize_progress) ) );

      if(!initialized)
        return false;

      //std::cout << "DEBUG: after connection_pool->initialize(). The database cluster should now exist." << std::endl;

      break;
    }
    case Document::HostingMode::POSTGRES_CENTRAL:
    case Document::HostingMode::MYSQL_CENTRAL:
    {
      if(!m_pDialogConnection)
      {
        Utils::get_glade_widget_derived_with_warning(m_pDialogConnection);
        if(!m_pDialogConnection)
        {
          std::cerr << G_STRFUNC << ": m_pBox_Reports is null." << std::endl;
          return false;
        }

        add_view(m_pDialogConnection); //Also a composite view.
      }

      //Ask for connection details:
      m_pDialogConnection->load_from_document(); //Get good defaults.
      m_pDialogConnection->set_transient_for(*get_app_window());

      const auto response = Glom::UiUtils::dialog_run_with_help(m_pDialogConnection);
      m_pDialogConnection->hide();

      if(response == Gtk::RESPONSE_OK)
      {
        // We are not self-hosting, but we also call initialize() for
        // consistency (the backend will ignore it anyway).
        ConnectionPool::SlotProgress slot_ignored;
        if(!handle_connection_initialize_errors( connection_pool->initialize(slot_ignored)) )
          return false;

        Glib::ustring username, password;
        m_pDialogConnection->get_username_and_password(username, password);
        connection_pool->set_user(username);
        connection_pool->set_password(password);

        // Remember the user name in the document, to be able to open the
        // document again later:
        document->set_connection_user(username);
      }
      else
      {
        // The user cancelled
        return false;
      }
      break;
    }
#ifdef GLOM_ENABLE_SQLITE
    case Document::HostingMode::SQLITE:
    {
      // SQLite:
      ConnectionPool::SlotProgress slot_ignored;
      if(!handle_connection_initialize_errors( connection_pool->initialize(slot_ignored)) )
        return false;

      //m_pDialogConnection->load_from_document(); //Get good defaults.
      // No authentication required
      
      break;
    }
#endif //GLOM_ENABLE_SQLITE
    default:
      g_assert_not_reached();
      break;
  }

  // Do startup, such as starting the self-hosting database server
  ShowProgressMessage progress_message(_("Starting Database Server"));
  const ConnectionPool::StartupErrors started =
    connection_pool->startup( sigc::mem_fun(*this, &Frame_Glom::on_connection_startup_progress) );
  if(started != ConnectionPool::Backend::StartupErrors::NONE)
  {
    std::cerr << G_STRFUNC << ": startup() failed." << std::endl;
    //TODO: Output more exact details of the error message.
    return false;
  }

  const auto database_name = document->get_connection_database();
  const auto database_name_possible = DbUtils::get_unused_database_name(database_name);
  if (database_name_possible.empty())
  {
    //This can only happen if we couldn't connect to the server at all.
    //Warn the user, and let him try again:
    UiUtils::show_ok_dialog(_("Connection Failed"), _("Glom could not connect to the database server. Maybe you entered an incorrect user name or password, or maybe the postgres database server is not running."), *(get_app_window()), Gtk::MESSAGE_ERROR); //TODO: Add help button.
    return false;
  }
  else
  {
    std::cout << "debug: " << G_STRFUNC << ": unused database name successfully found: " << database_name_possible << std::endl;

    //std::cout << "debug: unused database name found: " << database_name_possible << std::endl;
    document->set_connection_database(database_name_possible);

    // Remember host and port if the document is not self hosted
    #ifdef GLOM_ENABLE_POSTGRESQL
    if(document->get_hosting_mode() == Document::HostingMode::POSTGRES_CENTRAL)
    {
      auto backend = connection_pool->get_backend();
      auto central = dynamic_cast<ConnectionPoolBackends::PostgresCentralHosted*>(backend);
      g_assert(central);

      document->set_connection_server(central->get_host());
      document->set_connection_port(central->get_port());
      document->set_connection_try_other_ports(false);
    }

    // Remember port if the document is self-hosted, so that remote
    // connections to the database (usinc browse network) know what port to use.
    // TODO: There is already similar code in
    // connect_to_server_with_connection_settings, which is just not
    // executed because it failed with no database present. We should
    // somehow avoid this code duplication.
    else if(document->get_hosting_mode() == Document::HostingMode::POSTGRES_SELF)
    {
      auto backend = connection_pool->get_backend();
      auto self = dynamic_cast<ConnectionPoolBackends::PostgresSelfHosted*>(backend);
      g_assert(self);

      document->set_connection_port(self->get_port());
      document->set_connection_try_other_ports(false);
    }

    #endif //GLOM_ENABLE_POSTGRESQL

    return true;
  }

  cleanup_connection();

  return false;
}
#endif //GLOM_ENABLE_CLIENT_ONLY

void Frame_Glom::cleanup_connection()
{
  auto connection_pool = ConnectionPool::get_instance();
  ShowProgressMessage progress_message(_("Stopping Database Server"));
  connection_pool->cleanup( sigc::mem_fun(*this, &Frame_Glom::on_connection_cleanup_progress) );
}

bool Frame_Glom::handle_request_password_connection_error(bool asked_for_password, const ExceptionConnection& ex, bool& database_not_found)
{
  std::cerr << G_STRFUNC << ": caught exception." << std::endl;

  //Initialize input parameter:
  database_not_found = false;

  if(asked_for_password && ex.get_failure_type() == ExceptionConnection::failure_type::NO_SERVER)
  {
    //Warn the user, and let him try again:
    UiUtils::show_ok_dialog(_("Connection Failed"), _("Glom could not connect to the database server. Maybe you entered an incorrect user name or password, or maybe the postgres database server is not running."), *(get_app_window()), Gtk::MESSAGE_ERROR); //TODO: Add help button.
    return true;
  }
  else if(ex.get_failure_type() == ExceptionConnection::failure_type::NO_DATABASE)
  {
    cleanup_connection();

    //The connection to the server might be OK, but the specified database does not exist:
    //Or the connection failed when trying without a password.
    database_not_found = true; //Tell the caller about this error.
    return false;
  }
  else
  {
    std::cerr << G_STRFUNC << ": Unexpected exception: " << ex.what() << std::endl;
    cleanup_connection();
    return false;
  }
}

bool Frame_Glom::connection_request_password_and_attempt(bool& database_not_found, const Glib::ustring known_username, const Glib::ustring& known_password, bool confirm_known_user)
{
  //Initialize output parameter:
  database_not_found = false;

  auto document = dynamic_cast<Document*>(get_document());
  if(!document)
    return false;


  //Start a self-hosted server if necessary:
  auto connection_pool = ConnectionPool::get_instance();
  ShowProgressMessage progress_message(_("Starting Database Server"));
  connection_pool->setup_from_document(document);
  const ConnectionPool::StartupErrors started =
    connection_pool->startup( sigc::mem_fun(*this, &Frame_Glom::on_connection_startup_progress) );
  if(started != ConnectionPool::Backend::StartupErrors::NONE)
  {
    std::cerr << G_STRFUNC << ": startup() failed." << std::endl;
    return false;
    //TODO: Output more exact details of the error message.
  }

  auto app = dynamic_cast<AppWindow*>(get_app_window());
  if(!app)
  {
    std::cerr << G_STRFUNC << ": app is null." << std::endl;
    return false;
  }

  app->clear_progress_message();

  //Only ask for the password if we are shared on the network, or we are using a centrally hosted server.
  //Otherwise, no password question is necessary, due to how our self-hosted database server is configured.
  if(document->get_network_shared()
    || document->get_hosting_mode() == Document::HostingMode::POSTGRES_CENTRAL)
  {
    //We recreate the dialog each time to make sure it is clean of any changes:
    delete m_pDialogConnection;
    m_pDialogConnection = nullptr;

    Utils::get_glade_widget_derived_with_warning(m_pDialogConnection);
    if(!m_pDialogConnection)
    {
      std::cerr << G_STRFUNC << ": m_pDialogConnection is null." << std::endl;
      return false;
    }

    add_view(m_pDialogConnection); //Also a composite view.

    m_pDialogConnection->load_from_document(); //Get good defaults.
    m_pDialogConnection->set_transient_for(*get_app_window());

    //Show alternative text if necessary:
    if(confirm_known_user)
      m_pDialogConnection->set_confirm_existing_user_note();

    if(!known_username.empty())
      m_pDialogConnection->set_username(known_username);

    if(!known_password.empty())
      m_pDialogConnection->set_password(known_password);
  }
  else
  {
    //Later, if m_pDialogConnection is null then we assume we should use the known user/password:
    delete m_pDialogConnection;
    m_pDialogConnection = nullptr;
  }


  //Ask for connection details:
  while(true) //Loop until a return
  {
    //Only show the dialog if we don't know the correct username/password yet:
    int response = Gtk::RESPONSE_OK;

    if(m_pDialogConnection)
    {
      response = Glom::UiUtils::dialog_run_with_help(m_pDialogConnection);
      m_pDialogConnection->hide();
    }

    //Try to use the entered username/password:
    if(response == Gtk::RESPONSE_OK)
    {
      std::shared_ptr<SharedConnection> sharedconnection;

      //Ask for the user/password if necessary:
      //TODO: Remove any previous database setting?
      if(m_pDialogConnection)
      {
        try
        {
          sharedconnection = m_pDialogConnection->connect_to_server_with_connection_settings();
          // TODO: Save username in document?
          return true; //Succeeded, because no exception was thrown.
        }
        catch(const ExceptionConnection& ex)
        {
          if(!handle_request_password_connection_error(true, ex, database_not_found))
            return false;
        }
      }
      else
      {
        //Use the known password:
        auto connectionpool = ConnectionPool::get_instance();
        connectionpool->set_user(known_username);
        connectionpool->set_password(known_password);

        try
        {
          Base_DB::connect_to_server(get_app_window());
          return true; //Succeeded, because no exception was thrown.
        }
        catch(const ExceptionConnection& ex)
        {
          if(!handle_request_password_connection_error(false, ex, database_not_found))
            return false;
        }
      }

      //Try again.
    }
    else
    {
      cleanup_connection();
      return false; //The user cancelled.
    }
  }
}

#ifndef GLOM_ENABLE_CLIENT_ONLY
bool Frame_Glom::create_database(const Glib::ustring& database_name, const Glib::ustring& title)
{
  bool result = false;

  auto pWindowApp = get_app_window();
  g_assert(pWindowApp);

  {
    BusyCursor busycursor(*pWindowApp);

    std::function<void()> onProgress; //TODO: Show visual feedback.
    result = DbUtils::create_database(get_document(), database_name, title, onProgress);
  }

  if(!result)
  {
    //Tell the user:
    Gtk::Dialog* dialog = nullptr;
    Utils::get_glade_widget_with_warning("glom_developer.glade", "dialog_error_create_database", dialog);
    if(!dialog)
    {
      std::cerr << G_STRFUNC << ": dialog is null." << std::endl;
      return false;
    }

    dialog->set_transient_for(*pWindowApp);
    Glom::UiUtils::dialog_run_with_help(dialog, "dialog_error_create_database");
    delete dialog;

    return false;
  }

  return result;
}
#endif // !GLOM_ENABLE_CLIENT_ONLY

void Frame_Glom::on_menu_report_selected(const Glib::ustring& report_name)
{
  const auto table_privs = Privs::get_current_privs(m_table_name);

  //Don't try to print tables that the user can't view.
  if(!table_privs.m_view)
  {
    //TODO: Warn the user.
    return;
  }

  auto document = get_document();
  std::shared_ptr<Report> report = document->get_report(m_table_name, report_name);
  if(!report)
    return;

  FoundSet found_set = m_Notebook_Data.get_found_set();

  //TODO: Find a way to get a full locale name from the simplified locale name from AppWindow::get_current_locale():
  ReportBuilder report_builder(std::locale("") /* the user's current locale */);
  report_builder.set_document(document);
  const std::string filepath = 
    report_builder.report_build_and_save(found_set, report); //TODO: Use found set's where_clause.
  UiUtils::show_report_in_browser(filepath, get_app_window());
}

#ifndef GLOM_ENABLE_CLIENT_ONLY
 
void Frame_Glom::on_menu_print_layout_selected(const Glib::ustring& print_layout_name)
{
  do_print_layout(print_layout_name, false /* not preview */, get_app_window());
}

void Frame_Glom::do_print_layout(const Glib::ustring& print_layout_name, bool preview, Gtk::Window* transient_for)
{
  const auto document = get_document();
  std::shared_ptr<const PrintLayout> print_layout = document->get_print_layout(m_table_name, print_layout_name);
    
  const auto table_privs = Privs::get_current_privs(m_table_name);

  //Don't try to print tables that the user can't view.
  if(!table_privs.m_view)
  {
    //TODO: Warn the user.
    return;
  }
  
  //TODO: When expanding items, avoid the page gaps that the print layout's design
  //has added.  
  const auto found_set = m_Notebook_Data.get_found_set_selected();
  //Note that found_set.m_where_clause could be empty if there are no records yet,
  //and that is acceptable if this is for a print preview while designing the print layout. 
  
  PrintLayoutUtils::do_print_layout(print_layout, found_set, 
    preview, document, false /* do not avoid print margins */, transient_for);
}
 
#endif // !GLOM_ENABLE_CLIENT_ONLY

#ifndef GLOM_ENABLE_CLIENT_ONLY
void Frame_Glom::on_dialog_layout_report_hide()
{
  auto document = get_document();

  if(document && true) //m_pDialogLayoutReport->get_modified())
  {
    const auto original_name = m_pDialogLayoutReport->get_original_report_name();
    std::shared_ptr<Report> report = m_pDialogLayoutReport->get_report();
    if(report && (original_name != report->get_name()))
      document->remove_report(m_table_name, original_name);

    document->set_report(m_table_name, report);
  }

  //Update the reports menu:
  auto pApp = dynamic_cast<AppWindow*>(get_app_window());
  if(pApp)
    pApp->fill_menu_reports(m_table_name);
}

void Frame_Glom::on_dialog_layout_print_hide()
{
  auto document = get_document();

  if(document && true) //m_pDialogLayoutReport->get_modified())
  {
    const auto original_name = m_pDialogLayoutPrint->get_original_name();
    std::shared_ptr<PrintLayout> print_layout = m_pDialogLayoutPrint->get_print_layout();
    if(print_layout && (original_name != print_layout->get_name()))
      document->remove_report(m_table_name, original_name);

    document->set_print_layout(m_table_name, print_layout);
  }

  //Update the print layouts menu:
  auto pApp = dynamic_cast<AppWindow*>(get_app_window());
  if(pApp)
    pApp->fill_menu_print_layouts(m_table_name);
}
#endif // !GLOM_ENABLE_CLIENT_ONLY

void Frame_Glom::on_dialog_tables_hide()
{
  //If tables could have been added or removed, update the tables menu:
  auto document = dynamic_cast<Document*>(get_document());
  if(document)
  {
    // This is never true in client only mode, so we can as well save the
    // code size.
#ifndef GLOM_ENABLE_CLIENT_ONLY
    if(document->get_userlevel() == AppState::userlevels::DEVELOPER)
    {
      auto pApp = dynamic_cast<AppWindow*>(get_app_window());
      if(pApp)
        pApp->fill_menu_tables();

      //Select a different table if the current one no longer exists:
      if(!document->get_table_is_known(m_table_name))
      {
        //Open the default table, or the first table if there is no default:
        Glib::ustring table_name = document->get_default_table();
        if(table_name.empty())
          table_name = document->get_first_table();

        show_table(table_name);
      }
    }
#endif // !GLOM_ENABLE_CLIENT_ONLY
  }
}

void Frame_Glom::on_notebook_data_switch_page(Gtk::Widget* /* page */)
{
  //Refill this menu, because it depends on whether list or details are visible:
  auto pApp = dynamic_cast<AppWindow*>(get_app_window());
  if(pApp)
    pApp->fill_menu_print_layouts(m_table_name);
}

void Frame_Glom::on_notebook_data_record_details_requested(const Glib::ustring& table_name, Gnome::Gda::Value primary_key_value)
{
  //Specifying a primary key value causes the details tab to be shown:
  show_table(table_name, primary_key_value);
}

void Frame_Glom::on_notebook_data_record_selection_changed()
{
  bool something_selected = false;
  const auto found_set = m_Notebook_Data.get_found_set_selected();
  if(!found_set.m_where_clause.empty())
    something_selected = true;
  
  auto pApp = dynamic_cast<AppWindow*>(get_app_window());
  if(pApp)
    pApp->enable_menu_print_layouts_details(something_selected);  
}

namespace {

static Glib::ustring ulong_as_string(gulong value)
{
  Glib::ustring result;
  std::stringstream the_stream;
  //the_stream.imbue( current_locale );
  the_stream << value;
  the_stream >> result;
  return result;
}

} //anonymous namespace

gulong Frame_Glom::update_records_count()
{
  //Get the number of records available and the number found,
  //and allow the user to find all if necessary.

  gulong count_all = 0;
  gulong count_found = 0;
  m_Notebook_Data.get_record_counts(count_all, count_found);
  //std::cout << G_STRFUNC << ": count_all=" << count_all << ", count_found=" << count_found << std::endl;

  std::string str_count_all, str_count_found;
  str_count_all = ulong_as_string(count_all);

  if(count_found == count_all)
  {
    if(count_found != 0) //Show 0 instead of "all" when all of no records are found, to avoid confusion.
      str_count_found = _("All");
    else
      str_count_found = str_count_all;

    m_Button_FindAll.hide();
  }
  else
  {
    str_count_found = ulong_as_string(count_found);

    m_Button_FindAll.show();
  }

  m_Label_RecordsCount.set_text(str_count_all);
  m_Label_FoundCount.set_text(str_count_found);

  return count_found;
}

void Frame_Glom::on_button_find_all()
{
  //Change the found set to all records:
  show_table(m_table_name);
}

Glib::ustring Frame_Glom::get_shown_table_name() const
{
  return m_table_name;
}

} //namespace Glom
