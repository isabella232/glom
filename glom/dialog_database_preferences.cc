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
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */
 
#include "dialog_database_preferences.h"
#include "box_db.h" //For Box_DB::connect_to_server().
#include <glom/libglom/standard_table_prefs_fields.h>
#include <glom/libglom/data_structure/glomconversions.h>
#include <bakery/Utilities/BusyCursor.h>
#include <glibmm/i18n.h>

namespace Glom
{

Dialog_Database_Preferences::Dialog_Database_Preferences(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
: Gtk::Dialog(cobject),
  Base_DB(),
  m_glade_variables_map(refGlade)
{
  m_glade_variables_map.connect_widget("entry_name", m_system_prefs.m_name);
  m_glade_variables_map.connect_widget("entry_org_name", m_system_prefs.m_org_name);
  m_glade_variables_map.connect_widget("entry_org_address_street", m_system_prefs.m_org_address_street);
  m_glade_variables_map.connect_widget("entry_org_address_street2", m_system_prefs.m_org_address_street2);
  m_glade_variables_map.connect_widget("entry_org_address_town", m_system_prefs.m_org_address_town);
  m_glade_variables_map.connect_widget("entry_org_address_county", m_system_prefs.m_org_address_county);
  m_glade_variables_map.connect_widget("entry_org_address_country", m_system_prefs.m_org_address_country);
  m_glade_variables_map.connect_widget("entry_org_address_postcode", m_system_prefs.m_org_address_postcode);

  refGlade->get_widget_derived("imageglom", m_image);
  refGlade->get_widget("button_choose_image", m_button_choose_image);
  m_button_choose_image->signal_clicked().connect(sigc::mem_fun(*this, &Dialog_Database_Preferences::on_button_choose_image));

  refGlade->get_widget("treeview_autoincrements", m_treeview_autoincrements);

  m_model_autoincrements = Gtk::ListStore::create(m_columns);
  m_treeview_autoincrements->set_model(m_model_autoincrements);

  m_treeview_autoincrements->append_column(_("Table"), m_columns.m_col_table);
  m_treeview_autoincrements->append_column(_("Field"), m_columns.m_col_field);
  int view_cols_count = m_treeview_autoincrements->append_column(_("Next Value"), m_columns.m_col_next_value);

  Gtk::CellRendererText* pCellRenderer = dynamic_cast<Gtk::CellRendererText*>(m_treeview_autoincrements->get_column_cell_renderer(view_cols_count-1));
  if(pCellRenderer)
  {
    //Make it editable:
    pCellRenderer->property_editable() = true;
    pCellRenderer->property_xalign() = 1.0f; //Align right.

    //Connect to its signal:
    pCellRenderer->signal_edited().connect(
      sigc::mem_fun(*this, &Dialog_Database_Preferences::on_treeview_cell_edited_next_value) );
  }
}

Dialog_Database_Preferences::~Dialog_Database_Preferences()
{
}

void Dialog_Database_Preferences::on_treeview_cell_edited_next_value(const Glib::ustring& path_string, const Glib::ustring& new_text)
{
  if(path_string.empty())
    return;

  Gtk::TreePath path(path_string);

  //Get the row from the path:
  Gtk::TreeModel::iterator iter = m_model_autoincrements->get_iter(path);
  if(iter != m_model_autoincrements->children().end())
  {
    Gtk::TreeModel::Row row = *iter;

    //Set it in the model:
    long new_value = atol(new_text.c_str()); //TODO: Careful of locale.
    row[m_columns.m_col_next_value] = new_value;


    //Set it in the database system table:
    const Glib::ustring table_name = row[m_columns.m_col_table];
    const Glib::ustring field_name = row[m_columns.m_col_field];

    const Gnome::Gda::Value next_value = GlomConversions::parse_value(new_value);

    const Glib::ustring sql_query = "UPDATE \"" GLOM_STANDARD_TABLE_AUTOINCREMENTS_TABLE_NAME "\" SET "
        "\"" GLOM_STANDARD_TABLE_AUTOINCREMENTS_FIELD_NEXT_VALUE "\" = " + next_value.to_string() +
        " WHERE \"" GLOM_STANDARD_TABLE_AUTOINCREMENTS_FIELD_TABLE_NAME "\" = '" + table_name + "' AND "
               "\"" GLOM_STANDARD_TABLE_AUTOINCREMENTS_FIELD_FIELD_NAME "\" = '" + field_name +"'";

    Glib::RefPtr<Gnome::Gda::DataModel> datamodel = query_execute(sql_query, this);
    if(!datamodel)
    {
      g_warning("Dialog_Database_Preferences::on_treeview_cell_edited_next_value(): UPDATE failed.");
    }

  }
}

void Dialog_Database_Preferences::load_from_document()
{
  m_system_prefs = get_database_preferences();

  //Show the data in the UI:
  m_glade_variables_map.transfer_variables_to_widgets();
  m_image->set_value(m_system_prefs.m_org_logo);


  //Make sure that all auto-increment values are setup:
  Document_Glom* document = get_document();
  const Document_Glom::type_listTableInfo tables = document->get_tables();
  for(Document_Glom::type_listTableInfo::const_iterator iter = tables.begin(); iter != tables.end(); ++iter)
  {
    const Document_Glom::type_vecFields fields = document->get_table_fields((*iter)->get_name());
    for(Document_Glom::type_vecFields::const_iterator iterFields = fields.begin(); iterFields != fields.end(); ++iterFields)
    {
      sharedptr<Field> field = *iterFields;
      if(field->get_primary_key())
        auto_increment_insert_first_if_necessary((*iter)->get_name(), field->get_name());
    }
  }

  //Show the auto-increment values:
  m_model_autoincrements->clear();

  const Glib::ustring sql_query = "SELECT "
    "\"" GLOM_STANDARD_TABLE_AUTOINCREMENTS_TABLE_NAME "\".\"" GLOM_STANDARD_TABLE_AUTOINCREMENTS_FIELD_TABLE_NAME "\", "
    "\"" GLOM_STANDARD_TABLE_AUTOINCREMENTS_TABLE_NAME "\".\"" GLOM_STANDARD_TABLE_AUTOINCREMENTS_FIELD_FIELD_NAME "\", "
    "\"" GLOM_STANDARD_TABLE_AUTOINCREMENTS_TABLE_NAME "\".\"" GLOM_STANDARD_TABLE_AUTOINCREMENTS_FIELD_NEXT_VALUE "\""
    " FROM \"" GLOM_STANDARD_TABLE_AUTOINCREMENTS_TABLE_NAME "\"";

  NumericFormat numeric_format; //ignored

  Glib::RefPtr<Gnome::Gda::DataModel> datamodel = query_execute(sql_query, this);
  guint count = datamodel->get_n_rows();
  for(guint i = 0; i < count; ++i)
  {
    Gtk::TreeModel::iterator iter = m_model_autoincrements->append();
    Gtk::TreeModel::Row row = *iter;
    row[m_columns.m_col_table] = GlomConversions::get_text_for_gda_value(Field::TYPE_TEXT, datamodel->get_value_at(0, i), numeric_format);
    row[m_columns.m_col_field] = GlomConversions::get_text_for_gda_value(Field::TYPE_TEXT, datamodel->get_value_at(1, i), numeric_format);

    //TODO: Careful of locale:
    row[m_columns.m_col_next_value] = atol(datamodel->get_value_at(2, i).to_string().c_str());
  }

  m_model_autoincrements->set_default_sort_func( sigc::mem_fun(*this, &Dialog_Database_Preferences::on_autoincrements_sort) );
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
  Bakery::BusyCursor busy_cursor(this);

  m_glade_variables_map.transfer_widgets_to_variables();
  m_system_prefs.m_org_logo = m_image->get_value();

  set_database_preferences(m_system_prefs);
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

} //namespace Glom
