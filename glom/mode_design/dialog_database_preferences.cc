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
 
#include "dialog_database_preferences.h"
#include <glom/python_embed/glom_python.h>
#include <libglom/standard_table_prefs_fields.h>
#include <libglom/data_structure/glomconversions.h>
#include <libglom/db_utils.h>
#include <glom/bakery/busy_cursor.h>
#include <glom/utils_ui.h>
#include <gtksourceviewmm/languagemanager.h>
#include <glibmm/i18n.h>

#include <iostream>

namespace Glom
{

const char* Dialog_Database_Preferences::glade_id("dialog_database_preferences");
const bool Dialog_Database_Preferences::glade_developer(true);

Dialog_Database_Preferences::Dialog_Database_Preferences(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
: Gtk::Dialog(cobject),
  Base_DB(),
  m_glade_variables_map(builder)
{
  m_glade_variables_map.connect_widget("entry_name", m_system_prefs.m_name);
  m_glade_variables_map.connect_widget("entry_org_name", m_system_prefs.m_org_name);
  m_glade_variables_map.connect_widget("entry_org_address_street", m_system_prefs.m_org_address_street);
  m_glade_variables_map.connect_widget("entry_org_address_street2", m_system_prefs.m_org_address_street2);
  m_glade_variables_map.connect_widget("entry_org_address_town", m_system_prefs.m_org_address_town);
  m_glade_variables_map.connect_widget("entry_org_address_county", m_system_prefs.m_org_address_county);
  m_glade_variables_map.connect_widget("entry_org_address_country", m_system_prefs.m_org_address_country);
  m_glade_variables_map.connect_widget("entry_org_address_postcode", m_system_prefs.m_org_address_postcode);

  builder->get_widget_derived("imageglom", m_image);
  builder->get_widget("button_choose_image", m_button_choose_image);
  m_button_choose_image->signal_clicked().connect(sigc::mem_fun(*this, &Dialog_Database_Preferences::on_button_choose_image));

  builder->get_widget("treeview_autoincrements", m_treeview_autoincrements);

  m_model_autoincrements = Gtk::ListStore::create(m_columns);
  m_treeview_autoincrements->set_model(m_model_autoincrements);

  m_treeview_autoincrements->append_column(_("Table"), m_columns.m_col_table);
  m_treeview_autoincrements->append_column(_("Field"), m_columns.m_col_field);
  const int view_cols_count = m_treeview_autoincrements->append_column(_("Next Value"), m_columns.m_col_next_value);

  auto pCellRenderer = dynamic_cast<Gtk::CellRendererText*>(m_treeview_autoincrements->get_column_cell_renderer(view_cols_count-1));
  if(pCellRenderer)
  {
    //Make it editable:
    pCellRenderer->property_editable() = true;
    pCellRenderer->property_xalign() = 1.0f; //Align right.

    //Connect to its signal:
    pCellRenderer->signal_edited().connect(
      sigc::mem_fun(*this, &Dialog_Database_Preferences::on_treeview_cell_edited_next_value) );
  }

  //Startup script widgets:
  builder->get_widget("textview_calculation",  m_text_view_script);
  builder->get_widget("button_test",  m_button_test_script);

  m_button_test_script->signal_clicked().connect( 
    sigc::mem_fun(*this, &Dialog_Database_Preferences::on_button_test_script) );

  // Set a monospace font
  UiUtils::load_font_into_css_provider(*m_text_view_script, "Monospace");

  //Dialog_Properties::set_modified(false);

  //Tell the SourceView to do syntax highlighting for Python:
  auto languages_manager = 
    Gsv::LanguageManager::get_default();

  auto language = 
    languages_manager->get_language("python"); //This is the GtkSourceView language ID.
  if(language)
  {
     //Create a new buffer and set it, instead of getting the default buffer, in case libglade has tried to set it, using the wrong buffer type:
     auto buffer = Gsv::Buffer::create(language);
     buffer->set_highlight_syntax();
     m_text_view_script->set_buffer(buffer);
  }
}

void Dialog_Database_Preferences::on_treeview_cell_edited_next_value(const Glib::ustring& path_string, const Glib::ustring& new_text)
{
  if(path_string.empty())
    return;

  Gtk::TreeModel::Path path(path_string);

  //Get the row from the path:
  auto iter = m_model_autoincrements->get_iter(path);
  if(iter != m_model_autoincrements->children().end())
  {
    Gtk::TreeModel::Row row = *iter;

    //Set it in the model:
    const auto new_value = std::stol(new_text); //TODO: Careful of locale.
    row[m_columns.m_col_next_value] = new_value;


    //Set it in the database system table:
    const Glib::ustring table_name = row[m_columns.m_col_table];
    const Glib::ustring field_name = row[m_columns.m_col_field];

    const Gnome::Gda::Value next_value = Conversions::parse_value(new_value);
               
    auto builder = Gnome::Gda::SqlBuilder::create(Gnome::Gda::SQL_STATEMENT_UPDATE);
    builder->set_table(GLOM_STANDARD_TABLE_AUTOINCREMENTS_TABLE_NAME);
    builder->add_field_value_as_value(GLOM_STANDARD_TABLE_AUTOINCREMENTS_FIELD_NEXT_VALUE, next_value);
    builder->set_where(
      builder->add_cond(Gnome::Gda::SQL_OPERATOR_TYPE_AND,
        builder->add_cond(Gnome::Gda::SQL_OPERATOR_TYPE_EQ,
          builder->add_field_id(GLOM_STANDARD_TABLE_AUTOINCREMENTS_FIELD_TABLE_NAME, GLOM_STANDARD_TABLE_AUTOINCREMENTS_TABLE_NAME),
          builder->add_expr(table_name)),
        builder->add_cond(Gnome::Gda::SQL_OPERATOR_TYPE_EQ,
          builder->add_field_id(GLOM_STANDARD_TABLE_AUTOINCREMENTS_FIELD_FIELD_NAME, GLOM_STANDARD_TABLE_AUTOINCREMENTS_TABLE_NAME),
          builder->add_expr(field_name))));

    const bool test = DbUtils::query_execute(builder);
    if(!test)
      std::cerr << G_STRFUNC << ": UPDATE failed.\n";
  }
}

void Dialog_Database_Preferences::load_from_document()
{
  const auto document = get_document();
  m_system_prefs = DbUtils::get_database_preferences(document);

  //Show the data in the UI:
  m_glade_variables_map.transfer_variables_to_widgets();
  m_image->set_value(m_system_prefs.m_org_logo);


  //Make sure that all auto-increment values are setup:

  for(const auto& table : document->get_tables())
  {
    const Document::type_vec_fields fields = document->get_table_fields(table->get_name());
    for(const auto& field: fields)
    {
      if(field->get_primary_key())
        DbUtils::auto_increment_insert_first_if_necessary(table->get_name(), field->get_name());
    }
  }

  //Show the auto-increment values:
  m_model_autoincrements->clear();

  auto builder =
    Gnome::Gda::SqlBuilder::create(Gnome::Gda::SQL_STATEMENT_SELECT);
  builder->select_add_field(GLOM_STANDARD_TABLE_AUTOINCREMENTS_FIELD_TABLE_NAME,
    GLOM_STANDARD_TABLE_AUTOINCREMENTS_TABLE_NAME);
  builder->select_add_field(GLOM_STANDARD_TABLE_AUTOINCREMENTS_FIELD_FIELD_NAME,
    GLOM_STANDARD_TABLE_AUTOINCREMENTS_TABLE_NAME);
  builder->select_add_field(GLOM_STANDARD_TABLE_AUTOINCREMENTS_FIELD_NEXT_VALUE,
    GLOM_STANDARD_TABLE_AUTOINCREMENTS_TABLE_NAME);
  builder->select_add_target(GLOM_STANDARD_TABLE_AUTOINCREMENTS_TABLE_NAME);
  
  NumericFormat numeric_format; //ignored

  auto datamodel = DbUtils::query_execute_select(builder);
  if(!datamodel)
  {
    std::cerr << G_STRFUNC << ": Gda::DataModel is NULL.\n";
    return;
  }

  const guint count = datamodel->get_n_rows();
  for(guint i = 0; i < count; ++i)
  {
    auto iter = m_model_autoincrements->append();
    Gtk::TreeModel::Row row = *iter;
    row[m_columns.m_col_table] = Conversions::get_text_for_gda_value(Field::glom_field_type::TEXT, datamodel->get_value_at(0, i), numeric_format);
    row[m_columns.m_col_field] = Conversions::get_text_for_gda_value(Field::glom_field_type::TEXT, datamodel->get_value_at(1, i), numeric_format);

    //TODO: Careful of locale:
    row[m_columns.m_col_next_value] = std::stol(datamodel->get_value_at(2, i).to_string());
  }

  m_model_autoincrements->set_default_sort_func( sigc::mem_fun(*this, &Dialog_Database_Preferences::on_autoincrements_sort) );

  const Glib::ustring script = document->get_startup_script();
  m_text_view_script->get_buffer()->set_text(script);
}

int Dialog_Database_Preferences::on_autoincrements_sort(const Gtk::TreeModel::iterator& a, const Gtk::TreeModel::iterator& b)
{
  const Glib::ustring a_full = (*a)[m_columns.m_col_table] + ", " + (*a)[m_columns.m_col_field];
  const Glib::ustring b_full = (*b)[m_columns.m_col_table] + ", " + (*b)[m_columns.m_col_field];

  if(a_full < b_full)
   return -1;
  else if(a_full > b_full)
   return 1;
  else
   return 0;
}

void Dialog_Database_Preferences::save_to_document()
{
  BusyCursor busy_cursor(this);

  m_glade_variables_map.transfer_widgets_to_variables();
  m_system_prefs.m_org_logo = m_image->get_value();

  auto document = get_document();
  if(!document)
     return;

  //Make sure that set_database_preferences() can work.
  if(get_userlevel() == AppState::userlevels::DEVELOPER)
    DbUtils::add_standard_tables(document);

  DbUtils::set_database_preferences(document, m_system_prefs);

  //The script is not part of "database preferences" in the database data,
  //because it does not seem to be part of simple personalisation.
  const Glib::ustring script = m_text_view_script->get_buffer()->get_text();
  document->set_startup_script(script);
}

void Dialog_Database_Preferences::on_response(int response_id)
{
  if(response_id == Gtk::RESPONSE_OK)
    save_to_document();
}

void Dialog_Database_Preferences::on_button_choose_image()
{
   m_image->do_choose_image();
}


void Dialog_Database_Preferences::on_button_test_script()
{
  const Glib::ustring calculation = m_text_view_script->get_buffer()->get_text();

  auto document = get_document();
  if(!document)
    return;

  //We need the connection when we run the script, so that the script may use it.
  auto sharedconnection = connect_to_server(this /* parent window */);

  Glib::ustring error_message; //TODO: Check this and tell the user.
  PythonUICallbacks callbacks;
  glom_execute_python_function_implementation(calculation,
    type_map_fields(),
    document,
    Glib::ustring() /* table_name */,
    std::shared_ptr<Field>(), Gnome::Gda::Value(), // primary key - only used when setting values in the DB, which we would not encourage in a test.
    sharedconnection->get_gda_connection(),
    callbacks,
    error_message);

  if(!error_message.empty())
  {
    std::cerr << G_STRFUNC << ": Python Error: " << error_message << std::endl;
  }
}


} //namespace Glom
