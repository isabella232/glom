/* Glom
 *
 * Copyright (C) 2009 Murray Cumming
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

#include <glom/navigation/maemo/pickerbutton_table.h>
#include <glibmm/i18n.h>

namespace Glom
{

PickerButton_Table::PickerButton_Table()
{
  m_list_store = Gtk::ListStore::create(m_model_columns);

  //If this uses the 0th model column, then that should be documented in hildon.
  Glib::RefPtr<Hildon::TouchSelectorColumn> title_column =
    m_touchselector.append_text_column(m_list_store);
  title_column->set_property("text-column", 0); // TODO: Add a TextSelectorColumn::set_text_column() method?
  
  set_selector(m_touchselector);
  set_title(_("Table"));
}

PickerButton_Table::~PickerButton_Table()
{
}

void PickerButton_Table::load_from_document()
{
  fill_from_database();
}


bool PickerButton_Table::fill_from_database()
{
  Document* document = get_document();
  if(!document)
    return false;

  //Fill the model:
  m_list_store->clear();
  Document::type_listTableInfo listTablesDocument = document->get_tables();

  const type_vec_strings vecTables = get_table_names_from_database();
  for(type_vec_strings::const_iterator iter = vecTables.begin(); iter != vecTables.end(); iter++)
  {
    const Glib::ustring strName = *iter;
    sharedptr<TableInfo> table_info;

    Document::type_listTableInfo::const_iterator iterFind = 
      std::find_if(listTablesDocument.begin(), listTablesDocument.end(), predicate_FieldHasName<TableInfo>(strName));
    if(iterFind == listTablesDocument.end())
      continue;
    
    //Check whether it should be hidden:
    table_info = *iterFind;
    if(!table_info || table_info->m_hidden)
      continue;
    
    //Add to the model:
    Gtk::TreeModel::iterator model_iter = m_list_store->append();
    Gtk::TreeModel::Row row = *model_iter;
    row[m_model_columns.m_name] = table_info->get_name();
    row[m_model_columns.m_title] = table_info->get_title_or_name();
  }

  return true;
}


void PickerButton_Table::set_table_name(const Glib::ustring& table_name)
{
  for(Gtk::TreeModel::iterator iter = m_list_store->children().begin(); iter != m_list_store->children().end(); ++iter)
  {
    const Glib::ustring& this_text = (*iter)[m_model_columns.m_name];

    if(this_text == table_name)
    {
      //TODO: set_selected(iter);
      return; //success
    }
  }

  //Not found, so mark it as blank:
  std::cerr << "PickerButton_Table::set_table_name(): table_name not found in list: " << table_name << std::endl;
  //TODO: unset_active();
}

Glib::ustring PickerButton_Table::get_table_name() const
{
  //TODO: Check if this should be const:
  Hildon::TouchSelector* unconst_selector = const_cast<Hildon::TouchSelector*>(&m_touchselector);
  Gtk::TreeModel::iterator iter = unconst_selector->get_selected(0);
  if(!iter)
    return Glib::ustring();

  Gtk::TreeModel::Row row = *iter;
  const Glib::ustring name = row[m_model_columns.m_name];
  return name;
}

} //namespace Glom

