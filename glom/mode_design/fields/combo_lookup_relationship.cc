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

#include "combo_lookup_relationship.h"

Combo_LookupRelationship::Combo_LookupRelationship(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& /* refGlade */)
: Gtk::ComboBox(cobject)
{
  m_model = Gtk::ListStore::create(m_text_columns);
  set_model(m_model);
  pack_start(m_text_columns.m_col_name);
  pack_start(m_text_columns.m_col_from_field);
}


Combo_LookupRelationship::~Combo_LookupRelationship()
{

}

void Combo_LookupRelationship::append_text(const Glib::ustring& name, const Glib::ustring& from_field)
{
  Gtk::TreeModel::iterator iter = m_model->append();
  if(iter)
  {
    Gtk::TreeModel::Row row = *iter;
    row[m_text_columns.m_col_name] = name;

    //TODO: Make this internationalizable, though I'd rather just have a column title in the GtkComboBox:
    Glib::ustring trigger_text = "( triggered by: " + from_field + " )";
    row[m_text_columns.m_col_from_field] = trigger_text;
  }
}

Glib::ustring Combo_LookupRelationship::get_active_text() const
{
  Glib::ustring result;

  //Get the active row:
  Gtk::TreeModel::iterator active_row = get_active();
  if(active_row)
  {
    Gtk::TreeModel::Row row = *active_row;
    result = row[m_text_columns.m_col_name];
  }

  return result;
}

void Combo_LookupRelationship::clear_text()
{
  m_model->clear();
}

void Combo_LookupRelationship::set_active_text(const Glib::ustring& name)
{
  for(Gtk::TreeModel::iterator iter = m_model->children().begin(); iter != m_model->children().end(); ++iter)
  {
    Glib::ustring this_text = (*iter)[m_text_columns.m_col_name];

    if(this_text == name)
    {
      set_active(iter);
      return; //success
    }
  }

  //Not found, so mark it as blank:
  unset_active();
}




